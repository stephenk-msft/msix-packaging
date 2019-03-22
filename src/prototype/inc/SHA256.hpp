//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <vector>

    class SHA256
    {
    public:
        static bool ComputeHash(std::uint8_t *buffer, std::uint32_t cbBuffer, std::vector<uint8_t>& hash);
    };

    // This will probably end up in encoding.cpp
    class Base64
    {
    public:
        static std::string ComputeBase64(std::uint8_t *buffer, std::uint32_t cbBuffer);
    };

