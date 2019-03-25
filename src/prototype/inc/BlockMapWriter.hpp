//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once 

#include "Helpers.hpp"
#include "XmlWriter.hpp"

class BlockMapWriter final
{
public:
    BlockMapWriter(std::shared_ptr<ProtoXmlFactory> xmlFactory);
    std::unique_ptr<ProtoXmlElement> AddFile(std::string name, std::uint64_t uncompressedSize, std::uint32_t lfh);
    void AddBlockToElement(ProtoXmlElement& parent, std::string& hash, std::size_t size);
    std::vector<std::uint8_t> GetStream() { return m_xmlWriter->GetDom(); } // this would be like get blockman stream or so

protected:
    std::shared_ptr<ProtoXmlElement> m_root;
    std::unique_ptr<ProtoXmlWriter> m_xmlWriter;
    std::shared_ptr<ProtoXmlFactory> m_xmlFactory;
};
