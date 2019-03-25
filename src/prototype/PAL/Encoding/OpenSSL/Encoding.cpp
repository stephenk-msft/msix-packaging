//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Encoding.hpp"

#include "openssl/sha.h"
#include "openssl/evp.h"

#include <string>
#include <vector>

bool SHA256::ComputeHash(std::uint8_t *buffer, std::uint32_t cbBuffer, std::vector<uint8_t>& hash)
{
    hash.resize(SHA256_DIGEST_LENGTH);
    ::SHA256(buffer, cbBuffer, hash.data());
    return true;
}

std::string Base64::ComputeBase64(std::uint8_t *buffer, std::uint32_t cbBuffer)
{
    std::vector<std::uint8_t> result(((cbBuffer +2)/3)*4);
    EVP_EncodeBlock(static_cast<unsigned char*>(result.data()) , static_cast<unsigned char*>(buffer), cbBuffer);
    return std::string(result.begin(), result.end());
}

    // TODO: base64 using openssl
