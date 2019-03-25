//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "BlockMapWriter.hpp"
#include "XmlWriter.hpp"
#include "Encoding.hpp"

#include <memory>
#include <map>
#include <string>

BlockMapWriter::BlockMapWriter(std::shared_ptr<ProtoXmlFactory> xmlFactory) : m_xmlFactory(std::move(xmlFactory))
{
    // Create xml dom
    // there are 3 blockmap namespaces... add them later
    m_xmlWriter = m_xmlFactory->CreateDom("http://schemas.microsoft.com/appx/2010/blockmap", "BlockMap");

    // Add hash method to the root. There are like 3 hash methods, but we only use sha256 here
    // actually the msix sdk only supports sha256 ups...
    m_root = m_xmlWriter->GetRoot();
    m_root->AddAttribute("HashMethod", "http://www.w3.org/2001/04/xmlenc#sha256");
}

/*
    <File Name="Assets\Wide310x150Logo.scale-200.png" Size="3204" LfhSize="66">
        <Block Hash="tbd1SDLAjlj6rP5k6kufi1m1KmWOTupKqzeQz7ifqgM="/>
    </File>
    <File Name="resources.pri" Size="3760" LfhSize="43">
        <Block Hash="omadFn5zXbBfDtmAZjbjF54bh3HKZbrcD8UpBoUTiRY=" Size="1501"/>
    </File>
*/
std::unique_ptr<ProtoXmlElement> BlockMapWriter::AddFile(std::string name, std::uint64_t uncompressedSize, std::uint32_t lfh)
{
    // we are always use windows separators for the blockmap
    std::replace(name.begin(), name.end(), '/', '\\');
    auto fileElement = m_xmlWriter->CreateElement("File");
    fileElement->AddAttribute("Name", name);
    fileElement->AddAttribute("LfhSize", std::to_string(lfh));
    fileElement->AddAttribute("Size", std::to_string(uncompressedSize));
    m_root->AppendChild(fileElement.get());
    return fileElement;
}

void BlockMapWriter::AddBlockToElement(ProtoXmlElement& parent, std::string& hash, std::size_t size)
{
    auto blockElement = m_xmlWriter->CreateElement("Block");
    blockElement->AddAttribute("Hash", hash);
    // The Size attribute isn't specified if the file is not compressed;
    // if the Size attribute isn't specified, the value defaults to 64 KB, or the
    // remainder of the file size divided by 64 KB if the block is the last block.
    if (size != 0)
    {
        blockElement->AddAttribute("Size", std::to_string(size));
    }
    parent.AppendChild(blockElement.get());
}



