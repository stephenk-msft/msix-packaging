//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#import "NSXmlParserDelegateWrapper.h"
#import "XmlDocumentReader.hpp"

@implementation NSXmlParserDelegateWrapper{
    MSIX::XmlDocumentReader* m_xmlDocumentReader;
}

- (id) initWithXmlDocumentReader:(void *)xmlDocumentReader{
    self = [super init];
    if (self)
    {
        m_xmlDocumentReader = static_cast<MSIX::XmlDocumentReader*>(xmlDocumentReader);
    }
    return self;
}

- (void) parserDidStartDocument:(NSXMLParser *)parser {
}

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict {
    std::unique_ptr<MSIX::XmlNode> node(new MSIX::XmlNode());
    
    NSArray* list = [elementName componentsSeparatedByString:@":"];
    if ([list count] == 2)
    {
        // Has a namespace
        node->NamespaceUri = std::string([list[0] UTF8String]);
        node->NodeName = std::string([list[1] UTF8String]);
    }
    else
    {
        node->NodeName = std::string([list[0] UTF8String]);
    }
    
    if (qName)
    {
        node->QualifiedNodeName = std::string([qName UTF8String]);
    }
    for(id key in attributeDict)
    {
        node->Attributes.emplace(std::string([key UTF8String]), std::string([[attributeDict objectForKey:key] UTF8String]));
    }
    m_xmlDocumentReader->ProcessNodeBegin(std::move(node));
}

-(void) parser:(NSXMLParser *)parser foundCharacters:(NSString *)string {
    m_xmlDocumentReader->ProcessCharacters([string UTF8String]);
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName {
     m_xmlDocumentReader->ProcessNodeEnd(std::string([elementName UTF8String]));
}

- (void) parserDidEndDocument:(NSXMLParser *)parser {
}

- (void) parser:(NSXMLParser *)parser parseErrorOccurred:(NSError *)parseError {
}
@end
