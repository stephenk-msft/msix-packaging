//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "msixtest.hpp"
#include "catch.hpp"

#include "Windows.h"

#include <string>
#include <iostream>

namespace MsixTest { namespace Helpers {

    namespace Directory {

        // best effort to clean the directory.
        template<>
        bool CleanDirectory(const std::wstring& path)
        {
            static std::wstring dot(L".");
            static std::wstring dotdot(L"..");

            auto directory = path + L"\\*";

            WIN32_FIND_DATA findFileData = {};
            std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(&::FindClose)> handle(
                FindFirstFile(reinterpret_cast<LPCWSTR>(directory.c_str()), &findFileData),
                &FindClose);

            if (handle.get() == INVALID_HANDLE_VALUE)
            {
                return false;
            }

            do
            {
                auto file = std::wstring(findFileData.cFileName);
                auto toDel = path + L"\\" + file;
                if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (dot != file && dotdot != file)
                    {
                        CleanDirectory(toDel);
                    }
                }
                else
                {
                    if (!DeleteFile(toDel.c_str()))
                    {
                        return false;
                    }
                }
            } while(FindNextFile(handle.get(), &findFileData));

            if (!RemoveDirectory(path.c_str()))
            {
                return false;
            }

            return true;
        }

        // Converts path to windows speparator
        std::string PathAsCurrentPlatform(std::string& path)
        {
            std::string result(path);
            std::replace(result.begin(), result.end(), '/', '\\');
            return result;
        }
    }
} } // MsixTest::Helpers
