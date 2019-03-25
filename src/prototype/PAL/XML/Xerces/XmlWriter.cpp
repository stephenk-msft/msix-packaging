//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include <string>
#include <iostream>
#include <vector>
#include <memory>

// xerces headers
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLUni.hpp> // helpful XMLChr*
#include <xercesc/framework/MemBufFormatTarget.hpp>

#include "XmlWriter.hpp"
#include "Helpers.hpp"

XERCES_CPP_NAMESPACE_USE

// We need to put this in a common header
template<class T>
class XercesPtr
{
public:
    XercesPtr() : m_ptr(nullptr) {}
    XercesPtr(T* p)  : m_ptr(p) {}

    // move ctor
    XercesPtr(XercesPtr &&right) : m_ptr(nullptr)
    {
        if (this != reinterpret_cast<XercesPtr*>(&reinterpret_cast<std::int8_t&>(right)))
        {   Swap(right);
        }
    } 

    virtual ~XercesPtr() { InternalRelease(); }

    void InternalRelease()
    {
        T* temp = m_ptr;
        if (temp)
        {
            m_ptr = nullptr;
            temp->release();
        }
    }
    
    XercesPtr& operator=(XercesPtr&& right)
    {   XercesPtr(std::move(right)).Swap(*this);
        return *this;
    }

    T* operator->() const { return m_ptr; }
    T* Get() const { return m_ptr; }
protected:
    inline void Swap(XercesPtr& right ) { std::swap(m_ptr, right.m_ptr); }
    T* m_ptr = nullptr;
};

// this too
class XercesXMLChPtr
{
public:
    XercesXMLChPtr(const char* const value)
    {
        m_ptr = XMLString::transcode(value);
    }
    ~XercesXMLChPtr() { InternalRelease(); }

    // move ctor
    XercesXMLChPtr(XercesXMLChPtr &&right) : m_ptr(nullptr)
    {
        if (this != reinterpret_cast<XercesXMLChPtr*>(&reinterpret_cast<std::int8_t&>(right)))
        {   Swap(right);
        }
    }

    void InternalRelease()
    {
        XERCES_CPP_NAMESPACE::XMLString::release(&m_ptr);
        m_ptr = nullptr;
    }

    XercesXMLChPtr& operator=(XercesXMLChPtr&& right)
    {
        XercesXMLChPtr(std::move(right)).Swap(*this);
        return *this;
    }

    XMLCh* operator->() const { return m_ptr; }
    XMLCh* Get() const { return m_ptr; }
protected:
    inline void Swap(XercesXMLChPtr& right ) { std::swap(m_ptr, right.m_ptr); }
    XMLCh* m_ptr = nullptr;
};

// These classes will implement something similar to IXmlDom, IXmlElement, etc...
class XmlElement final : public ProtoXmlElement
{
public:
    XmlElement(XERCES_CPP_NAMESPACE::DOMElement* element) : m_element(element)
    {
    }

    virtual void AddAttribute(std::string name, std::string value)
    {
        m_element->setAttribute(
            XercesXMLChPtr(name.c_str()).Get(),
            XercesXMLChPtr(value.c_str()).Get());
    }

    virtual void AppendChild(ProtoXmlElement* protoXmlElement)
    {
        auto other = static_cast<XmlElement*>(protoXmlElement);
        this->m_element->appendChild(other->m_element);
    }

protected:
XERCES_CPP_NAMESPACE::DOMElement* m_element;
};

class XercesWriter final : public ProtoXmlWriter
{
public:
    XercesWriter(std::string xmlNamespace, std::string root)
    {
        // do this when possible to avoid transcoding
        static const XMLCh coreStr[] = { chLatin_C, chLatin_o, chLatin_r, chLatin_e, chNull };
        m_domImpl = DOMImplementationRegistry::getDOMImplementation(coreStr);
        m_namespace = XercesXMLChPtr(xmlNamespace.c_str());
        m_domDoc = XercesPtr<XERCES_CPP_NAMESPACE::DOMDocument>(m_domImpl->createDocument(m_namespace.Get(), XercesXMLChPtr(root.c_str()).Get(), 0));
        m_protoRoot = std::make_unique<XmlElement>(m_domDoc->getDocumentElement());
    }

    virtual std::shared_ptr<ProtoXmlElement> GetRoot() override
    {
        return m_protoRoot;
    }

    virtual std::unique_ptr<ProtoXmlElement> CreateElement(std::string name) override
    {
        XERCES_CPP_NAMESPACE::DOMElement* element = m_domDoc->createElementNS(m_namespace.Get(), XercesXMLChPtr(name.c_str()).Get());
        return std::make_unique<XmlElement>(element);
    }

    virtual std::vector<std::uint8_t> GetDom() override
    {
        static const XMLCh cs[3] = {chLatin_L, chLatin_S, chNull};
        DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(cs);
        
        auto serializer = XercesPtr<DOMLSSerializer>((static_cast<DOMImplementationLS*>(impl))->createLSSerializer());
        auto lsOutput = XercesPtr<DOMLSOutput>((static_cast<DOMImplementationLS*>(impl))->createLSOutput());
        
        // set encoding to UTF-8
        static const XMLCh utf8Str[] = {chLatin_U, chLatin_T, chLatin_F, chDash, chDigit_8, chNull};
        lsOutput->setEncoding(utf8Str);

        DOMConfiguration* serializerConfig = serializer->getDomConfig();
        // TODO: consider implementing an error handler just in case...

        // This is just to make it human readable, we dont need it but is useful to have
        //if (serializerConfig->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true))
        //{
        //    serializerConfig->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
        //}

        if (serializerConfig->canSetParameter(XMLUni::fgDOMXMLDeclaration, true))
        {
            serializerConfig->setParameter(XMLUni::fgDOMXMLDeclaration, true);
        }

        std::unique_ptr<MemBufFormatTarget> formatTarget = std::make_unique<MemBufFormatTarget>();
        lsOutput->setByteStream(static_cast<XMLFormatTarget*>(formatTarget.get()));
        serializer->write(m_domDoc.Get(), lsOutput.Get());

        auto formatTargetBuffer = formatTarget->getRawBuffer();
        std::size_t formatTargetBufferSize = static_cast<std::size_t>(formatTarget->getLen());
        std::vector<std::uint8_t> result(formatTargetBufferSize);
        for (std::size_t i = 0; i < formatTargetBufferSize; i++)
        {
            result[i] = formatTargetBuffer[i];
        }
        return result;
    }

protected:
    XercesXMLChPtr m_namespace = nullptr;
    XercesPtr<XERCES_CPP_NAMESPACE::DOMDocument> m_domDoc;
    std::shared_ptr<ProtoXmlElement> m_protoRoot = nullptr;
    DOMImplementation* m_domImpl;
};

class XercesFactory final : public ProtoXmlFactory
{
public:
    XercesFactory()
    {
        XERCES_CPP_NAMESPACE::XMLPlatformUtils::Initialize();
    }

    ~XercesFactory()
    {
        XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate();
    }

    virtual std::unique_ptr<ProtoXmlWriter> CreateDom(std::string xmlNamespace, std::string root) override
    {
        return std::make_unique<XercesWriter>(xmlNamespace, root);
    }
};

std::unique_ptr<ProtoXmlFactory> CreateXmlFactory() { return std::make_unique<XercesFactory>(); }

