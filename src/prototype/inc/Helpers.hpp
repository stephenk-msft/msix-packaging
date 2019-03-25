//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#pragma once

// Trying to keep msix files not involved at all in the prototype, 
// so when i needed something i just copy it here.

#ifdef WIN32
#include <windows.h>
#include <stringapiset.h>
#undef max
#undef min
#endif

#include <cstdio>
#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <stdexcept>
#include <locale>
#include <codecvt>

#define ThrowIf(a, m) if (a) { throw std::runtime_error(m); }

// appxpackaging.hpp
typedef enum APPX_COMPRESSION_OPTION
{
    APPX_COMPRESSION_OPTION_NONE = 0,
    APPX_COMPRESSION_OPTION_NORMAL = 1,
} APPX_COMPRESSION_OPTION;

// UnicodeConversion.cpp 
inline std::wstring utf8_to_wstring(const std::string& utf8string)
{
    #ifdef WIN32
    int size = MultiByteToWideChar(CP_UTF8, 0, utf8string.data(), static_cast<int>(utf8string.size()), nullptr, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8string.data(), static_cast<int>(utf8string.size()), &result[0], size);
    #else
    auto converted = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(utf8string.data());
    std::wstring result(converted.begin(), converted.end());
    #endif
    return result;
}

inline std::string wstring_to_utf8(const std::wstring& utf16string)
{
    #ifdef WIN32
    int size = WideCharToMultiByte(CP_UTF8, 0, utf16string.data(), static_cast<int>(utf16string.size()), nullptr, 0, nullptr, nullptr);
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, utf16string.data(), static_cast<int>(utf16string.size()), &result[0], size, nullptr, nullptr);
    #else
    auto result = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
    #endif
    return result;
}

// FileStream.hpp
class File final
{
public:
    enum Mode { READ = 0, WRITE, APPEND, READ_UPDATE, WRITE_UPDATE, APPEND_UPDATE };

    File(const std::string& name, Mode mode) : m_name(name)
    {
        static const char* modes[] = { "rb", "wb", "ab", "r+b", "w+b", "a+b" };
        #ifdef WIN32
        errno_t err = fopen_s(&m_file, name.c_str(), modes[mode]);
        ThrowIf(err != 0, "file error");
        #else
        m_file = std::fopen(name.c_str(), modes[mode]);
        ThrowIf(!m_file, "cant open file" + name);
        #endif

        fseek(m_file, 0, SEEK_END);
        m_size = ftell(m_file);
        fseek(m_file, 0, SEEK_SET);
    }

    virtual ~File() { Close(); }

    void Close()
    {
        if (m_file)
        {
            std::fclose(m_file);
            m_file = nullptr;
        }
    }

    void Read(void* buffer, std::size_t countBytes, std::size_t* bytesRead)
    {
        if (bytesRead) { *bytesRead = 0; }
        auto result = std::fread(buffer, sizeof(std::uint8_t), countBytes, m_file);
        ThrowIf(!(result == countBytes || Feof()), "error reading file")
        m_offset = Ftell();
        if (bytesRead) { *bytesRead = result; }
    }

    void Write(const void *buffer, std::size_t countBytes, std::size_t * bytesWritten)
    {
        if (bytesWritten) { *bytesWritten = 0; }
        auto result = std::fwrite(buffer, sizeof(std::uint8_t), countBytes, m_file);
        ThrowIf(result != countBytes, "write failed");
        m_offset = Ftell();
        if (bytesWritten) { *bytesWritten = result; }
    }

    std::string GetName() { return m_name; }
    std::uint64_t GetSize() { return m_size; }
    std::uint64_t GetOffset() { return m_offset; }

protected:
    inline int Ferror() { return std::ferror(m_file); }
    inline bool Feof()  { return 0 != std::feof(m_file); }
    inline void Flush() { std::fflush(m_file); }

    inline std::uint64_t Ftell()
    {
        auto result = std::ftell(m_file);
        return static_cast<std::uint64_t>(result);
    }

    std::uint64_t m_offset = 0;
    std::uint64_t m_size = 0;
    std::string m_name;
    FILE* m_file;
};

// Encoding.cpp
const std::size_t PercentageEncodingTableSize = 0x7F;
const std::array<const wchar_t*, PercentageEncodingTableSize> PercentageEncoding =
{   nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    L"%20",  L"%21",  nullptr, L"%23",  L"%24",  L"%25",  L"%26",  L"%27",   // [space] ! # $ % & '
    L"%28",  L"%29",  nullptr, L"%2B",  L"%2C",  nullptr, nullptr, nullptr, // ( ) + ,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, L"%3B",  nullptr, L"%3D",  nullptr, nullptr, // ; =
    L"%40",  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // @
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, L"%5B",  nullptr, L"%5D",  nullptr, nullptr, // [ ]
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, L"%7B",  nullptr, L"%7D",  nullptr,          // { }
};

struct EncodingChar
{
    const wchar_t* encode;
    wchar_t        decode;

    bool operator==(const std::wstring& rhs) const {
        return rhs == encode;
    }
    EncodingChar(const wchar_t* e, wchar_t d) : encode(e), decode(d) {}
};

// Returns the file name percentage encoded.
inline std::string EncodeFileName(const std::string& fileName)
{
    if (fileName.empty()) { return ""; } // lets just return empty if failed
    std::wstring fileNameW = utf8_to_wstring(fileName);
    std::wstring result = L"";

    for (std::uint32_t index = 0; index < fileNameW.length(); index++)
    {
        std::uint32_t codepoint = static_cast<std::uint32_t>(fileNameW[index]);

        // Start of double wchar UTF-16 sequence
        if ((codepoint & 0xFC00) == 0xD800)
        {
            if ((fileNameW[index] & 0xFC00) == 0xD800 &&
                (fileNameW[index+1] & 0xFC00) == 0xDC00)
            {
                codepoint = (((fileNameW[index] & 0x03C0) + 0x0040) | (fileNameW[index] & 0x003F)) << 10;
                codepoint |= (fileNameW[index+1] & 0x03FF);
            }
            else
            {
                return ""; // lets just return empty if failed
            }
            index++;
        }
        else if ((codepoint & 0xFC00) == 0xDC00)
        {
            return ""; // lets just return empty if failed
        }

        // See if it's one of the special cases we encode
        if (codepoint < PercentageEncodingTableSize && PercentageEncoding[codepoint] != nullptr)
        {   result += PercentageEncoding[codepoint];
        }
        else if (fileNameW[index] == '\\') // replace backslash
        {   result.push_back('/');
        }
        else if (codepoint > PercentageEncodingTableSize)
        {   // Returns the length of the UTF-8 byte sequence associated with the given codepoint
            // We already know is > 0x7F, so it can't be 1 byte
            std::uint8_t totalBytes = 0;
            if (codepoint <= 0x07FF) { totalBytes = 2; }
            else if (codepoint <= 0xFFFF) { totalBytes = 3; }
            else { totalBytes = 4; }

            const std::wstring hexadecimal = L"0123456789ABCDEF";
            for (size_t byteIndex = 0; byteIndex < totalBytes; byteIndex++)
            {
                std::uint32_t mychar;
                switch (totalBytes - byteIndex)
                {
                case 1:
                    if (totalBytes == 1) { mychar = codepoint; } 
                    else { mychar = 0x80 | (codepoint & 0x003F); }
                    break;
                case 2:
                    if (totalBytes == 2) { mychar = 0xC0 | ((codepoint & 0x07C0) >> 6); } 
                    else { mychar = 0x80 | ((codepoint & 0x0FC0) >> 6); }
                    break;
                case 3:
                    if (totalBytes == 3) { mychar = 0xE0 | ((codepoint & 0xF000) >> 12); } 
                    else { mychar = 0x80 | ((codepoint & 0x03F000) >> 12); }
                    break;
                case 4:
                    mychar = 0xF0 | ((codepoint & 0x1C0000) >> 18);
                    break;
                default:
                    return ""; // lets just return empty if failed
                    break;
                }

                auto highDigit = mychar / hexadecimal.size();
                auto lowDigit = mychar % hexadecimal.size();

                if (highDigit > hexadecimal.size() || lowDigit  > hexadecimal.size()) { return ""; } // lets just return empty if failed
                result.push_back('%'); // we are percentage encoding
                result.push_back(hexadecimal[highDigit]);
                result.push_back(hexadecimal[lowDigit]);
            }
        }
        else
        {   result.push_back(fileNameW[index]);
        }
    }
    return wstring_to_utf8(result);
}

// appxpackageobject.cpp
#define APPXBLOCKMAP_XML       "AppxBlockMap.xml"
#define APPXMANIFEST_XML       "AppxManifest.xml"
#define CODEINTEGRITY_CAT      "AppxMetadata/CodeIntegrity.cat"
#define APPXSIGNATURE_P7X      "AppxSignature.p7x"
#define CONTENT_TYPES_XML      "[Content_Types].xml"
#define APPXBUNDLEMANIFEST_XML "AppxMetadata/AppxBundleManifest.xml"

static const std::array<const char*, 4> footprintFiles =
{   APPXMANIFEST_XML,
    APPXBLOCKMAP_XML,
    APPXSIGNATURE_P7X,
    CODEINTEGRITY_CAT,
};

