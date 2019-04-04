//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
// main.cpp

#include "DirectoryRead.hpp"
#include "BlockMapWriter.hpp"
#include "Encoding.hpp"
#include "DeflateObject.hpp"
#include "ContentTypeWriter.hpp"
#include "ZipObject.hpp"

#include <zlib.h>

#include <map>
#include <string>
#include <iostream>

typedef struct Block
{
    std::vector<std::uint8_t> block;
    std::string base64Encoded;
} BlockAndHash;

typedef struct PayloadFile // rename this
{
    APPX_COMPRESSION_OPTION compressionOption;
    std::uint32_t crc = 0;
    std::uint64_t uncompressedSize;
    std::string relativeName;
    std::vector<std::unique_ptr<Block>> fileBlocks;
} PayloadFile;


// So main.cpp is kind of like the package writer...
class PackageBuilder final
{
public:
    PackageBuilder(std::string root, std::string packageName) : m_root(root)
    {
        m_xmlFactory = CreateXmlFactory();

        m_contentTypeWriter = std::make_unique<ContentTypeWriter>(m_xmlFactory);
        m_zipObject = std::make_unique<ZipObjectWriter>(packageName);

        // payload files are added by last modified time. using multimap to get
        // "free" sorting and to have repeated keys in the unlikely case files
        // have the same last modified time
        std::multimap<std::uint64_t, std::string> files;
        std::string empty; // this is weird... please fix it
        GetAllFilesInDirectory(root, empty ,files);

        // Creating the blocks
        for (auto& file : files)
        {
            ProcessFiles(file.second);
        }

        ThrowIf(m_payloadFiles.empty(), "there should be some payload files"); // TODO: verify this is true
        ThrowIf(!m_manifestFile, "AppxManifest.xml not found")

        // Creating block map file
        m_blockMapWriter = std::make_unique<BlockMapWriter>(m_xmlFactory);

        for (auto& payloadFile: m_payloadFiles)
        {
            AddToBlockMapAndZip(*payloadFile);
        }

        // TODO: validate appx manifest. Just create an AppxManifestObject from msix and if doesn't fail we should be good.
        AddToBlockMapAndZip(*m_manifestFile); // the manifest is the last one to be added 

        // add appx blockmap to zip
        auto blockMapBuffer = m_blockMapWriter->GetStream();
        AddToZip(APPXBLOCKMAP_XML, blockMapBuffer);

        // add blockmap to content types
        m_contentTypeWriter->AddOverride("application/vnd.ms-appx.blockmap+xml", "/AppxBlockMap.xml");

        // add content types to zip
        auto contentTypeBuffer = m_contentTypeWriter->GetStream();
        AddToZip(CONTENT_TYPES_XML, contentTypeBuffer);

        m_zipObject->Close();
    }

private:
    void AddToZip(std::string name, std::vector<std::uint8_t>& buffer)
    {
        auto lfh = m_zipObject->WriteLfh(name, true);
        std::uint32_t crc = crc32(0, buffer.data(), static_cast<uInt>(buffer.size()));
        DeflateObject deflateObj;
        deflateObj.SetInput(buffer.data(), buffer.size());
        auto compressedBuffer =  deflateObj.Deflate();
        m_zipObject->WriteBuffer(compressedBuffer);
        m_zipObject->WriteCdh(*lfh, crc, compressedBuffer.size(), buffer.size());
    }

    void AddToBlockMapAndZip(PayloadFile& payloadFile)
    {
        bool isCompressed = (payloadFile.compressionOption != APPX_COMPRESSION_OPTION_NONE );
        auto opcFileName = EncodeFileName(payloadFile.relativeName);
        auto lfh = m_zipObject->WriteLfh(opcFileName, isCompressed);

        // Add file to blockmap
        auto fileElement = m_blockMapWriter->AddFile(payloadFile.relativeName, payloadFile.uncompressedSize, static_cast<std::uint32_t>(lfh->Size()));

        std::uint32_t sizeOfBlocks = 0; // this is compressed data size (may be not uncompressed)
        for (auto& block : payloadFile.fileBlocks)
        {
            // for production procesing all the SHA256s and base64s should be done in parallel and before.
            sizeOfBlocks += static_cast<std::uint32_t>(block->block.size());

            m_zipObject->WriteBuffer(block->block);

            auto size = (isCompressed) ? block->block.size() : 0;
            m_blockMapWriter->AddBlockToElement(*fileElement, block->base64Encoded, size);
        }
        m_zipObject->WriteCdh(*lfh, payloadFile.crc, sizeOfBlocks, payloadFile.uncompressedSize);
    }

    // read file, process blocks
    void ProcessFiles(std::string& fileName)
    {
        bool isManifest = false;
        if (!m_manifestFile && (fileName == APPXMANIFEST_XML)) { isManifest = true; ; }

        auto payloadFile = std::make_unique<PayloadFile>();
        payloadFile->relativeName = fileName;
        #ifdef WIN32
        std::string realPath = m_root + "\\" + fileName;
        #else
        std::string realPath = m_root + "/" + fileName;
        #endif
        auto file = std::make_unique<File>(realPath, File::Mode::READ);

        std::string contentType;
        std::string ext;
        if (isManifest)
        {
            payloadFile->compressionOption = APPX_COMPRESSION_OPTION_NORMAL;
            contentType = "application/vnd.ms-appx.manifest+xml";
            ext = "xml";
        }
        else
        {
            ext = fileName.substr(fileName.find_last_of(".") + 1);
            auto findExt = s_extToContentType.find(ext);
            if (findExt == s_extToContentType.end())
            {
                // Default seetings
                payloadFile->compressionOption = APPX_COMPRESSION_OPTION_NORMAL;
                contentType = "application/octet-stream";
            }
            else
            {
                payloadFile->compressionOption = findExt->second.second;
                contentType = findExt->second.first;
            }
        }
        m_contentTypeWriter->AddDefault(contentType, ext, isManifest);

        // reading file
        payloadFile->uncompressedSize = file->GetSize();

        std::unique_ptr<Block> blockData;
        auto bytesToRead = file->GetSize();
        std::uint32_t crc = 0;
        
        while (bytesToRead > 0)
        {
            // Calculate the size of the next block to add
            static const std::uint32_t defaultBlockSize = 65536;
            std::uint32_t blockSize = (bytesToRead > defaultBlockSize) ? defaultBlockSize : static_cast<std::uint32_t>(bytesToRead);
            bytesToRead -= blockSize;

            std::vector<std::uint8_t> buffer;
            buffer.resize(blockSize);
            std::size_t bytesRead;
            file->Read(buffer.data(), buffer.size(), &bytesRead);
            ThrowIf(bytesRead != buffer.size(), "error reading file");
            crc = crc32(crc, buffer.data(), static_cast<uInt>(buffer.size()));

            // write block to blockmap
            std::vector<std::uint8_t> hash;
            bool result = SHA256::ComputeHash(buffer.data(), static_cast<std::uint32_t>(buffer.size()), hash);
            ThrowIf(!result, "SHA256 error ");
            auto base64Encoded = Base64::ComputeBase64(hash.data(), static_cast<uint32_t>(hash.size()));

            if (payloadFile->compressionOption == APPX_COMPRESSION_OPTION_NORMAL)
            {
                DeflateObject deflateObj;
                // maybe make this into one call?
                deflateObj.SetInput(buffer.data(), buffer.size());
                auto compressedBuffer =  deflateObj.Deflate();
                buffer.swap(compressedBuffer);
            }

            blockData.reset(new (std::nothrow) Block());
            blockData->block = std::move(buffer);
            blockData->base64Encoded = base64Encoded;
            payloadFile->fileBlocks.push_back(std::move(blockData));
        }
        payloadFile->crc = crc;
        if (isManifest)
        {
            m_manifestFile = std::move(payloadFile);
        }
        else
        {
            m_payloadFiles.push_back(std::move(payloadFile));
        }
    }

protected:
std::shared_ptr<ProtoXmlFactory> m_xmlFactory;
std::unique_ptr<ContentTypeWriter> m_contentTypeWriter;
std::unique_ptr<BlockMapWriter> m_blockMapWriter;
std::unique_ptr<ZipObjectWriter> m_zipObject;
std::vector<std::unique_ptr<PayloadFile>> m_payloadFiles;
std::unique_ptr<PayloadFile> m_manifestFile;
std::string m_root;
};

int main(int argc, char* argv[])
{
    if (argc != 5) { return -1; } // usage is prototype.exe -d <path> -p <package>

    std::string directoryToPack;
    std::string package;
    for (int i = 1; i < 5; i++)
    {
        std::string arg(argv[i]);
        if (arg == "-d")
        {
            directoryToPack = std::string(argv[++i]);
        }
        else if (arg == "-p")
        {
            package = std::string(argv[++i]);
        }
        else
        {
            std::cout << "invalid command" << std::endl;
            return -1;
        }
    }

    try
    {
        auto builder = PackageBuilder(directoryToPack, package);
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}
