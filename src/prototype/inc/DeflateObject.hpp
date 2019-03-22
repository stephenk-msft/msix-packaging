//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
// This will end up in DeflateObject or maybe not to avoid adding functionality to msix.dll
#pragma once

#include "Helpers.hpp"

#include <zlib.h>
#include <iostream>

// Note: either there's a here bug or zlib and opc result in the same output
// eg. unpacking and packing TestAppxPackage_Win32.appx file TestAppxPackage.exe 3 blocks
// ill keep looking in to it, but a package craete can be succesfully unpacked via makeappx so...
// makeappx | Z_DEFAULT_COMPRESSION | Z_BEST_COMPRESSION | Z_BEST_SPEED
//    20070                   20066                19980          22990
//    24274                   24351                24150          26839
//    21197                   21190                21132          22513
// Using Z_BEST_COMPRESSION and hoping for the best
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
    void Deflate(int flush)
    {
        std::vector<std::uint8_t> deflateBuffer(1024); // experiment with bigger buffers
        do
        {
            SetOutput(deflateBuffer.data(), deflateBuffer.size());
            auto result = deflate(&m_zstrm, flush);
            ThrowIf(
                (result != Z_OK) && 
                (flush == Z_FINISH && result != Z_STREAM_END), // Z_FINISH returns Z_STREAM_END when is done
                "zlib error");
            auto have = deflateBuffer.size() - GetAvailableDestinationSize();
            m_compressedBuffer.insert(m_compressedBuffer.end(), deflateBuffer.data(), deflateBuffer.data() + have);
            //The way we tell that deflate() has no more output is by seeing that it did not fill the output
            // buffer, leaving avail_out greater than zero.
        } while (GetAvailableDestinationSize() == 0);
    }

    std::vector<std::uint8_t> Finish()
    {
        // If the parameter flush is set to Z_FINISH, pending input is processed,
        // pending output is flushed and deflate returns with Z_STREAM_END if there was
        // enough output space. If deflate returns with Z_OK or Z_BUF_ERROR, this
        // function must be called again with Z_FINISH and more output space (updated
        // avail_out) but no more input data, until it returns with Z_STREAM_END or an
        // error.
        Deflate(Z_FINISH);
        return m_compressedBuffer;
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
    std::vector<std::uint8_t> m_compressedBuffer;
};