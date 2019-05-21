//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "AppxPackaging.hpp"
#include "MSIXWindows.hpp"

#include "msixtest.hpp"

#include <string>
#include <map>

namespace MsixTest {

    // Singleton class that return the relative path of the test data
    class TestData
    {
    public:
        typedef enum
        {
            Output,
            Unpack,
            Unbundle,
            Flat,
            BadFlat
        } Directory;

        static TestData* GetInstance();
        void SetRoot(const char* root);
        std::string GetRoot();
        std::string GetPath(Directory opt);

    private:
        TestData() {}
        TestData(const TestData&);
        TestData& operator=(const TestData&);

        static TestData* m_instance;
        std::string m_root;
    };

    namespace Allocators
    {
        LPVOID STDMETHODCALLTYPE Allocate(SIZE_T cb);
        void STDMETHODCALLTYPE Free(LPVOID pv);
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
