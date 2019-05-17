//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "AppxPackaging.hpp"
#include "MSIXWindows.hpp"

#include <string>
#include <map>

namespace MsixTest {

    // relative location of test data according build directory
    namespace TestData
    {
        static const char* OutputDirectory = "output";
        static const char* Unpack = "msixtest/testData/unpack/";
    }

    namespace Allocators
    {
        LPVOID STDMETHODCALLTYPE Allocate(SIZE_T cb);
        void STDMETHODCALLTYPE Free(LPVOID pv);
    }

    namespace Directory
    {
        bool CleanDirectory(const std::string& directory);
        bool CompareDirectory(const std::string& directory, const std::map<std::string, std::uint64_t>& files);

        std::string PathAsCurrentPlatform(std::string& path);
    }

    namespace String
    {
        std::string utf16_to_utf8(const std::wstring& utf16string);
        std::wstring utf8_to_utf16(const std::string& utf8string);

        // Helper class to free string buffers obtained from the packaging APIs.
        template<typename T>
        class Text
        {
        public:
            T** operator&() { return &content; }
            ~Text() { Cleanup(); }
            T* Get() { return content; }

            T* content = nullptr;
        protected:
            void Cleanup() { if (content) { Allocators::Free(content); content = nullptr; } }
        };
    }

    namespace Log
    {
        void PrintMsixLog(HRESULT actual, HRESULT result);
    }

}
 
// This is our entrypoint.
int msixtest_main(int argc, char* argv[]);
