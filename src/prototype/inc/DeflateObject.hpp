//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
// This will end up in DeflateObject or maybe not to avoid adding functionality to msix.dll
#pragma once

#include "Helpers.hpp"

#include <zlib.h>
#include <iostream>

class DeflateObject final
{
public:
    DeflateObject()
    {
        // Z_NO_COMPRESSION         0
        // Z_BEST_SPEED             1
        // Z_BEST_COMPRESSION       9
        // Z_DEFAULT_COMPRESSION  (-1)
        m_zstrm.zalloc = Z_NULL;
        m_zstrm.zfree = Z_NULL;
        m_zstrm.opaque = Z_NULL;
        auto result = deflateInit2(&m_zstrm, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
        ThrowIf(result != Z_OK, "error deflateinit2");
    }

    ~DeflateObject()
    {
        deflateEnd(&m_zstrm);
    }

    // Z_NO_FLUSH      0
    // Z_PARTIAL_FLUSH 1
    // Z_SYNC_FLUSH    2
    // Z_FULL_FLUSH    3
    // Z_FINISH        4
    // Z_BLOCK         5
    // Z_TREES         6
    std::vector<std::uint8_t> Deflate()
    {
        std::vector<std::uint8_t> compressedBuffer;
        std::vector<std::uint8_t> deflateBuffer(1024);
        do
        {
            SetOutput(deflateBuffer.data(), deflateBuffer.size());
            auto result = deflate(&m_zstrm, Z_BLOCK);
            ThrowIf(result != Z_OK, "zlib error");
            auto have = deflateBuffer.size() - GetAvailableDestinationSize();
            compressedBuffer.insert(compressedBuffer.end(), deflateBuffer.data(), deflateBuffer.data() + have);
            //The way we tell that deflate() has no more output is by seeing that it did not fill the output
            // buffer, leaving avail_out greater than zero.
        } while (GetAvailableDestinationSize() == 0);
        return std::move(compressedBuffer);
    }

    size_t GetAvailableSourceSize() noexcept
    {
        return m_zstrm.avail_in;
    }

    size_t GetAvailableDestinationSize() noexcept
    {
        return m_zstrm.avail_out;
    }

    void SetInput(std::uint8_t* buffer, size_t size) noexcept
    {
        m_zstrm.next_in = buffer;
        m_zstrm.avail_in = static_cast<std::uint32_t>(size);
    }

private:
    void SetOutput(std::uint8_t* buffer, size_t size) noexcept
    {
        m_zstrm.next_out = buffer;
        m_zstrm.avail_out = static_cast<std::uint32_t>(size);
    }

    z_stream m_zstrm;
};