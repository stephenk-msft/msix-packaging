//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "SharedOpenSSL.hpp"

namespace MSIX
{
    namespace
    {
        struct CustomObjectDef
        {
            CustomOpenSSLObjectName name;
            const char* oid;
            const char* shortName;
            const char* longName;
        };

#define MSIX_MAKE_CUSTOM_OBJECT_DEF(_name_,_oid_) { CustomOpenSSLObjectName:: ## _name_, _oid_, #_name_, #_name_ }

        CustomObjectDef customObjects[]
        {
            MSIX_MAKE_CUSTOM_OBJECT_DEF(spcIndirectDataContext, "1.3.6.1.4.1.311.2.1.4"),
            MSIX_MAKE_CUSTOM_OBJECT_DEF(spcSipInfoObjID, "1.3.6.1.4.1.311.2.1.30"),
            MSIX_MAKE_CUSTOM_OBJECT_DEF(spcSpOpusInfo, "1.3.6.1.4.1.311.2.1.12"),
            MSIX_MAKE_CUSTOM_OBJECT_DEF(spcStatementType, "1.3.6.1.4.1.311.2.1.11"),
            MSIX_MAKE_CUSTOM_OBJECT_DEF(individualCodeSigning, "1.3.6.1.4.1.311.2.1.21"),
        };
    }

    CustomOpenSSLObjects::CustomOpenSSLObjects()
    {
        for (const auto& obj : customObjects)
        {
            int nid = OBJ_create(obj.oid, obj.shortName, obj.longName);
            if (nid == NID_undef)
            {
                ThrowOpenSSLError();
            }
            objects.emplace_back(obj.name, nid);
        }
    }

    CustomOpenSSLObjects::~CustomOpenSSLObjects()
    {
        OBJ_cleanup();
    }

    const CustomOpenSSLObject& CustomOpenSSLObjects::Get(CustomOpenSSLObjectName name) const
    {
        for (const auto& obj : objects)
        {
            if (obj.GetName() == name)
            {
                return obj;
            }
        }

        UNEXPECTED;
    }
} // namespace MSIX
