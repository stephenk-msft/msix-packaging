//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <cstdint>
#include <vector>
#include <string>

class SHA256
{
public:
    static bool ComputeHash(std::uint8_t *buffer, std::uint32_t cbBuffer, std::vector<uint8_t>& hash);
};

// We use openssl or the windows api to encode to base64. I don't remember why we implemted
// GetBase64DecodedValue for msix, so maybe for parity we would implement encode too.
// using the apis is easier tho...
class Base64
{
public:
    static std::string ComputeBase64(std::uint8_t *buffer, std::uint32_t cbBuffer);
};

