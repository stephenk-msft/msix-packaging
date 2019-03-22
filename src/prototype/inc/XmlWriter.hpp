//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <string>
#include <memory>
#include <vector>

// This will become real interfaces for production
class ProtoXmlElement
{
public:
    virtual void AddAttribute(std::string name, std::string value) = 0;
    virtual void AppendChild(ProtoXmlElement* protoXmlElement) = 0;
};

class ProtoXmlWriter
{
public:
    virtual std::shared_ptr<ProtoXmlElement> GetRoot() = 0;
    virtual std::unique_ptr<ProtoXmlElement> CreateElement(std::string name) = 0;
    virtual std::vector<std::uint8_t> GetDom() = 0;
};

class ProtoXmlFactory
{
public:
    virtual std::unique_ptr<ProtoXmlWriter> CreateDom(std::string xmlNamespace, std::string root) = 0;
};

std::unique_ptr<ProtoXmlFactory> CreateXmlFactory();
