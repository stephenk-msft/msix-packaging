//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "ContentTypeWriter.hpp"

ContentTypeWriter::ContentTypeWriter(std::shared_ptr<ProtoXmlFactory> xmlFactory) : m_xmlFactory(std::move(xmlFactory))
{
    m_xmlWriter = m_xmlFactory->CreateDom("http://schemas.openxmlformats.org/package/2006/content-types", "Types");
    m_root = m_xmlWriter->GetRoot();
}

/*
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
    <Default ContentType="image/png" Extension="png"/>
    <Default ContentType="application/octet-stream" Extension="winmd"/>
    <Default ContentType="application/vnd.ms-appx.manifest+xml" Extension="xml"/>
    <Override ContentType="application/vnd.ms-appx.blockmap+xml" PartName="/AppxBlockMap.xml"/>
</Types>
*/
void ContentTypeWriter::AddDefault(std::string& contentType, std::string& extension, bool force)
{
    auto result = m_extensions.insert(extension);
    // Only add content type when the extension hasn't been added yet
    if (result.second || force)
    {
        auto defaultElement = m_xmlWriter->CreateElement("Default");
        defaultElement->AddAttribute("ContentType", contentType);
        defaultElement->AddAttribute("Extension", extension);
        m_root->AppendChild(defaultElement.get());
    }
}
void ContentTypeWriter::AddOverride(std::string contentType, std::string partName)
{
    auto defaultElement = m_xmlWriter->CreateElement("Override");
    defaultElement->AddAttribute("ContentType", contentType);
    defaultElement->AddAttribute("PartName", partName);
    m_root->AppendChild(defaultElement.get());
}
