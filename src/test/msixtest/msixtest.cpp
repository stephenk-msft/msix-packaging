//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#define CATCH_CONFIG_RUNNER // don't use catch2 main CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "msixtest.hpp"

#include <iostream>
#include <locale>
#include <codecvt>

namespace MsixTest {

    namespace Allocators
    {
        LPVOID STDMETHODCALLTYPE Allocate(SIZE_T cb)  { return std::malloc(cb); }
        void STDMETHODCALLTYPE Free(LPVOID pv)        { std::free(pv); }
    }

    namespace String {
        std::string utf16_to_utf8(const std::wstring& utf16string)
        {
            auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
            std::string result(converted.begin(), converted.end());
            return result;
        }

        std::wstring utf8_to_utf16(const std::string& utf8string)
        {
            #ifdef WIN32
            auto converted = std::wstring_convert<std::codecvt_utf8_utf16<unsigned short>, unsigned short>{}.from_bytes(utf8string.data());
            #else
            auto converted = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(utf8string.data());
            #endif
            std::wstring result(converted.begin(), converted.end());
            return result;
        }
    }

    namespace Log {
        void PrintMsixLog(HRESULT expect, HRESULT result)
        {
            std::cout << "\tExpect:\t" << std::hex << expect << ", Got: " << result << std::endl;
            if (result != S_OK)
            {
                std::cout << "\tError:\t" << std::hex << result << std::endl;
                String::Text<char> text;
                auto logResult = GetLogTextUTF8(Allocators::Allocate, &text);
                if (0 == logResult)
                {
                    std::cout << "LOG:" << std::endl << text.content << std::endl;
                }
                else 
                {
                    std::cout << "UNABLE TO GET LOG WITH HR=" << std::hex << logResult << std::endl;
                }
            }
            std::cout << std::string(CATCH_CONFIG_CONSOLE_WIDTH, '-') << std::endl;
        }
    }

}

int msixtest_main(int argc, char* argv[])
{
    // Forward the arguments to Catch2
    int result = Catch::Session().run(argc, argv);

    return result;
}
