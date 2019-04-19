//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "AppxPackaging.hpp"
#include "AppxSignature.hpp"

namespace MSIX {

    class SignatureCreator
    {
    public:
        static ComPtr<AppxSignatureObject> Create(
            IAppxPackageReader* package);
    };
}

