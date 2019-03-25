//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

// zip object
// this would end up in zip folder, unless we define this as zipobject writer
// to avoid including writing functionality to msix.dll
#include "ObjectBase.hpp"
#include "ZipObject.hpp"
#include "Helpers.hpp"

#include <limits>
#include <algorithm>

// from ZIP file format specification detailed in AppNote.txt
enum class Signatures : std::uint32_t
{
    LocalFileHeader         = 0x04034b50,
    DataDescriptor          = 0x08074b50,
    CentralFileHeader       = 0x02014b50,
    Zip64EndOfCD            = 0x06064b50,
    Zip64EndOfCDLocator     = 0x07064b50,
    EndOfCentralDirectory   = 0x06054b50,
};

// from AppNote.txt, section 4.5.2:
enum class HeaderIDs : std::uint16_t
{
    Zip64ExtendedInfo = 0x0001, // Zip64 extended information extra field
};

enum class ZipVersions : std::uint16_t
{
    // Zip32DefaultVersion = 20, - we always do Zip64
    Zip64FormatExtension = 0x2d, // 45
};

enum class GeneralPurposeBitFlags : std::uint16_t
{
    GeneralPurposeBit = 0x0008,     // the field's crc-32 compressed and uncompressed sizes = 0 in the local header
                                    // the correct values are put in the data descriptor immediately following the
                                    // compressed data.
};

enum class CompressionType : std::uint16_t
{
    Store = 0,
    Deflate = 8,
};

// Windows implementation takes the current time of the system.
// We need to either replicate this and find a way to do UNIX time to MS DOS times
// or just put placeholders. We don't really do anything with the these anyway.
enum class LastMod : std::uint16_t
{
    FileDate = 0x5347,
    FileTime = 0x4552,
};

// 0 - tag for the "extra" block type               2 bytes(0x0001)
// 1 - size of this "extra" block                   2 bytes
// 2 - Original uncompressed file size              8 bytes
// No point in validating these as it is actually 
// possible to have a 0-byte file... Who knew.
// 3 - Compressed file size                         8 bytes
// No point in validating these as it is actually 
// possible to have a 0-byte file... Who knew.    
// 4 - Offset of local header record                8 bytes
// 5 - number of the disk on which the file starts  4 bytes -- ITS A FAAKEE!
Zip64ExtendedInformation::Zip64ExtendedInformation(std::uint64_t compressedSize, std::uint64_t uncompressedSize, std::uint64_t relativeOffset)
{
    Field<0>().value = static_cast<std::uint16_t>(HeaderIDs::Zip64ExtendedInfo);
    // Should be 0x18, but do it this way if we end up field 5
    Field<1>().value = static_cast<std::uint16_t>(Size() - Field<0>().Size() - Field<1>().Size());
    Field<2>().value = uncompressedSize;
    Field<3>().value = compressedSize;
    Field<4>().value = relativeOffset;
}

// 0 - central file header signature   4 bytes(0x02014b50)
// 1 - version made by                 2 bytes
// 2 - version needed to extract       2 bytes
// 3 - general purpose bit flag        2 bytes
// 4 - compression method              2 bytes
// 5 - last mod file time              2 bytes
// 6 - last mod file date              2 bytes
// 7 - crc - 32                        4 bytes
// 8 - compressed size                 4 bytes
// 9 - uncompressed size               4 bytes
//10 - file name length                2 bytes
//11 - extra field length              2 bytes
//12 - file comment length             2 bytes
//13 - disk number start               2 bytes
//14 - internal file attributes        2 bytes
//15 - external file attributes        4 bytes
//16 - relative offset of local header 4 bytes
//17 - file name                       (variable size)
//18 - extra field                     (variable size)
//19 - file comment                  (variable size) NOT USED
CentralDirectoryFileHeader::CentralDirectoryFileHeader(std::string& name, std::uint32_t crc, std::uint64_t compressedSize,
    std::uint64_t uncompressedSize, std::uint64_t relativeOffset,  std::uint16_t compressionMethod)
{
    Field<0>().value = static_cast<std::uint32_t>(Signatures::CentralFileHeader);
    Field<1>().value = static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension);
    Field<2>().value = static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension);
    Field<3>().value = static_cast<std::uint16_t>(GeneralPurposeBitFlags::GeneralPurposeBit);
    Field<4>().value = compressionMethod;
    Field<5>().value = static_cast<std::uint16_t>(LastMod::FileTime);
    Field<6>().value = static_cast<std::uint16_t>(LastMod::FileDate);
    Field<7>().value = crc;
    Field<8>().value = std::numeric_limits<std::uint32_t>::max(); // always use zip64
    Field<9>().value = std::numeric_limits<std::uint32_t>::max(); // always use zip64
    Field<10>().value = static_cast<std::uint16_t>(name.size());
    // Field<11>().value not yet
    Field<12>().value = 0x0;
    Field<13>().value = 0x0;
    Field<14>().value = 0x0;
    Field<15>().value = 0x0;
    Field<16>().value = std::numeric_limits<std::uint32_t>::max(); // always use zip64
    Field<17>().value.resize(Field<10>().value, 0);
    std::copy(name.begin(), name.end(), Field<17>().value.begin());
    // Field 11 and 18
    auto extendendInfo = std::make_unique<Zip64ExtendedInformation>(compressedSize, uncompressedSize, relativeOffset);
    Field<11>().value = static_cast<std::uint16_t>(extendendInfo->Size());
    Field<18>().value = extendendInfo->GetBytes();
    // Field<19> NOT USED
}

// 0 - data descriptor header signature  4 bytes(0x08074b50)
// 1 - crc -32                           4 bytes
// 2 - compressed size                   8 bytes(zip64)
// 3 - uncompressed size                 8 bytes(zip64)
DataDescriptor::DataDescriptor(std::uint32_t crc, std::uint64_t compressSize, std::uint64_t uncompressSize)
{
    Field<0>().value = static_cast<std::uint32_t>(Signatures::DataDescriptor);
    Field<1>().value = crc;
    Field<2>().value = compressSize;
    Field<3>().value = uncompressSize;
}

// 0 - local file header signature     4 bytes(0x04034b50)
// 1 - version needed to extract       2 bytes
// 2 - general purpose bit flag        2 bytes
// 3 - compression method              2 bytes
// 4 - last mod file time              2 bytes
// 5 - last mod file date              2 bytes
// 6 - crc - 32                        4 bytes
// 7 - compressed size                 4 bytes
// 8 - uncompressed size               4 bytes
// 9 - file name length                2 bytes
// 10- extra field length              2 bytes
// 11- file name                       (variable size)
// 12- extra field                     (variable size) NOT USED
LocalFileHeader::LocalFileHeader(std::string& name, bool isCompressed, std::uint64_t offset) : m_offset(offset)
{
    Field<0>().value = static_cast<std::uint32_t>(Signatures::LocalFileHeader);
    Field<1>().value = static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension); // always zip64
    Field<2>().value = static_cast<std::uint16_t>(GeneralPurposeBitFlags::GeneralPurposeBit);
    Field<3>().value = (isCompressed) ? 0x8 : 0x0;
    Field<4>().value = static_cast<std::uint16_t>(LastMod::FileTime);
    Field<5>().value = static_cast<std::uint16_t>(LastMod::FileDate);
    Field<6>().value = 0x0;
    Field<7>().value = 0x0;
    Field<8>().value = 0x0;
    Field<9>().value = static_cast<std::uint16_t>(name.size());
    Field<10>().value = 0x0;
    Field<11>().value.resize(Field<9>().value, 0);
    std::copy(name.begin(), name.end(), Field<11>().value.begin());
    // Field<12> NOT USED
}

// 0 - zip64 end of central dir signature                            4 bytes(0x06064b50)
// 1 - size of zip64 end of central directory record                 8 bytes
// 2 - version made by                                               2 bytes
// 3 - version needed to extract                                     2 bytes
// 4 - number of this disk                                           4 bytes
// 5 - number of the disk with the start of the central directory    4 bytes
// 6 - total number of entries in the central directory on this disk 8 bytes
// 7 - total number of entries in the central directory              8 bytes
// 8 - size of the central directory                                 8 bytes
// 9 - offset of start of central directory with respect to the
//     starting disk number                                          8 bytes
// 10 - zip64 extensible data sector                                  (variable size) NOT USED
Zip64EndOfCentralDirectoryRecord::Zip64EndOfCentralDirectoryRecord(std::uint64_t numCentralDirs, std::uint64_t sizeCentralDir, std::uint64_t offsetStartCentralDirectory)
{
    Field<0>().value = static_cast<std::uint32_t>(Signatures::Zip64EndOfCD);
    Field<1>().value = static_cast<std::uint64_t>(Size() - Field<0>().Size() - Field<1>().Size()); // We not use Field<10> so there's no variable data
    Field<2>().value = static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension); // always zip64
    Field<3>().value = static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension); // always zip64
    Field<4>().value = 0x0;
    Field<5>().value = 0x0;
    Field<6>().value = numCentralDirs;
    Field<7>().value = numCentralDirs;
    Field<8>().value = sizeCentralDir;
    Field<9>().value = offsetStartCentralDirectory;
    // Field<10> NOT USED
}

// 0 - zip64 end of central dir locator signature        4 bytes(0x07064b50)
// 1 - number of the disk with the start of the zip64
//     end of central directory                          4 bytes
// 2 - relative offset of the zip64 end of central
//     directory record                                  8 bytes
// 3 - total number of disks                             4 bytes
Zip64EndOfCentralDirectoryLocator::Zip64EndOfCentralDirectoryLocator(std::uint64_t zip64EndCdrOffset)
{
    Field<0>().value = static_cast<std::uint32_t>(Signatures::Zip64EndOfCDLocator);
    Field<1>().value = 0x0;
    Field<2>().value = zip64EndCdrOffset;
    Field<3>().value = 0x1; // always 1 disk
}

// 0 - end of central dir signature              4 bytes  (0x06054b50)
// 1 - number of this disk                       2 bytes
// 2 - number of the disk with the start of the
//     central directory                         2 bytes
// 3 - total number of entries in the central
//     directory on this disk                    2 bytes
// 4 - total number of entries in the central
//     directory                                 2 bytes
// 5 - size of the central directory             4 bytes
// 6 - offset of start of central directory with
//     respect to the starting disk number       4 bytes
// 7 - .ZIP file comment length                  2 bytes
// 8 - .ZIP file comment                         (variable size) NOT USED
EndCentralDirectoryRecord::EndCentralDirectoryRecord()
{
    Field<0>().value = static_cast<std::uint32_t>(Signatures::EndOfCentralDirectory);
    Field<1>().value = std::numeric_limits<std::uint16_t>::max(); // always use zip64
    Field<2>().value = std::numeric_limits<std::uint16_t>::max(); // always use zip64
    Field<3>().value = std::numeric_limits<std::uint16_t>::max(); // always use zip64
    Field<4>().value = std::numeric_limits<std::uint16_t>::max(); // always use zip64
    Field<5>().value = std::numeric_limits<std::uint32_t>::max(); // always use zip64
    Field<6>().value = std::numeric_limits<std::uint32_t>::max(); // always use zip64
    Field<7>().value = 0x0;
    //Field<8>() NOT USED
}

// Implement ZipObjectWriter
ZipObjectWriter::ZipObjectWriter(std::string& name) : m_packageName(name)
{
    m_package = std::make_unique<File>(name, File::Mode::APPEND_UPDATE);
    // it is technically valid to have an empty zip, not an empty package tho. But we validate that waaay before.
    m_state = State::ReadyForLfhOrClose;
}

std::unique_ptr<LocalFileHeader> ZipObjectWriter::WriteLfh(std::string& name, bool isCompressed)
{
    ThrowIf(m_state != ZipObjectWriter::State::ReadyForLfhOrClose, "expecting CDH");
    auto offset = m_package->GetOffset();
    auto lfh = std::make_unique<LocalFileHeader>(name, isCompressed, offset);
    auto lfhBytes = lfh->GetBytes();
    std::size_t bytesWritten = 0;
    m_package->Write(static_cast<void*>(lfhBytes.data()), lfhBytes.size(), &bytesWritten);
    m_state = ZipObjectWriter::State::ReadyForBuffer;
    return lfh;
}

void ZipObjectWriter::WriteBuffer(std::vector<std::uint8_t>& buffer)
{
    ThrowIf((m_state != ZipObjectWriter::State::ReadyForBuffer) && (m_state != ZipObjectWriter::State::ReadyForBufferOrCdh), "expecting buffer");
    std::size_t bytesWritten = 0;
    m_package->Write(static_cast<void*>(buffer.data()), buffer.size(), &bytesWritten);
    m_state = ZipObjectWriter::State::ReadyForBufferOrCdh;
}

void ZipObjectWriter::WriteCdh(LocalFileHeader& lfh, std::uint32_t crc, std::uint64_t compressedSize, std::uint64_t uncompressedSize)
{
    ThrowIf(m_state != ZipObjectWriter::State::ReadyForBufferOrCdh, "expecting LFH or Close");
    auto lfhOffset = lfh.GetOffset();
    auto whereweshouldbe = lfhOffset + lfh.Size() + compressedSize;
    // we should expect that we are actually saying the truth
    ThrowIf(whereweshouldbe != m_package->GetOffset(), "Invalid data");
    // create and write data descriptor
    auto dataDescriptor = std::make_unique<DataDescriptor>(crc, compressedSize, uncompressedSize);
    auto dataDescriptorBytes = dataDescriptor->GetBytes();
    std::size_t bytesWritten = 0;
    m_package->Write(static_cast<void*>(dataDescriptorBytes.data()), dataDescriptorBytes.size(), &bytesWritten);
    // create cdh
    auto name = lfh.GetName();
    auto cdh = std::make_unique<CentralDirectoryFileHeader>(name, crc, compressedSize, uncompressedSize, lfhOffset, lfh.GetCompressionMethod());
    m_cdhs.push_back(std::move(cdh));
    m_state = ZipObjectWriter::State::ReadyForLfhOrClose;
}

void ZipObjectWriter::Close()
{
    ThrowIf(m_state != ZipObjectWriter::State::ReadyForLfhOrClose, "expecting CDH");

    // write central directories
    auto startOfCdh = m_package->GetOffset();
    std::size_t cdhsSize = 0;
    for (const auto& cdh : m_cdhs)
    {
        cdhsSize += cdh->Size();
        auto cdhBytes = cdh->GetBytes();
        std::size_t bytesWritten = 0;
        m_package->Write(static_cast<void*>(cdhBytes.data()), cdhBytes.size(), &bytesWritten);
    }

    // create and write zip64 record
    auto startOfZip64Record = m_package->GetOffset();
    auto zip64CdhRecord = Zip64EndOfCentralDirectoryRecord(m_cdhs.size(), static_cast<std::uint64_t>(cdhsSize), static_cast<std::uint64_t>(startOfCdh));
    auto zip64CdhRecordBytes = zip64CdhRecord.GetBytes();
    std::size_t bytesWritten = 0;
    m_package->Write(static_cast<void*>(zip64CdhRecordBytes.data()), zip64CdhRecordBytes.size(), &bytesWritten);

    // craete and write zip64 locator
    auto zip64Locator = Zip64EndOfCentralDirectoryLocator(static_cast<std::uint64_t>(startOfZip64Record));
    auto zip64LocatorBytes = zip64Locator.GetBytes();
    bytesWritten = 0;
    m_package->Write(static_cast<void*>(zip64LocatorBytes.data()), zip64LocatorBytes.size(), &bytesWritten);

    // end central directory record
    auto endOfCd = EndCentralDirectoryRecord();
    auto endOfCdBytes = endOfCd.GetBytes();
    bytesWritten = 0;
    m_package->Write(static_cast<void*>(endOfCdBytes.data()), endOfCdBytes.size(), &bytesWritten);

    m_state = ZipObjectWriter::State::Closed;
}
