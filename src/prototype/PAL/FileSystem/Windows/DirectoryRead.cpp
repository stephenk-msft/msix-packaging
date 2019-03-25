//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include <Windows.h>
#include "DirectoryRead.hpp"
#include "Helpers.hpp"

#include <map>
#include <string>
#include <iostream>

void GetAllFilesInDirectory(std::string& directory, std::string& relativePath, std::multimap<std::uint64_t, std::string>& files)
{
    auto directory = utf8_to_wstring(directoryUtf8);
    directory.append(L"\\*");
    WIN32_FIND_DATAW fileData;
    HANDLE fileHandle;
    if ((fileHandle = FindFirstFileW(directory.c_str(), &fileData)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            auto fileName = wstring_to_utf8(fileData.cFileName);
            if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if ((fileName != ".") && (fileName != ".."))
                {
                    std::string newRelativePath;
                    std::string newPath;
                    if (relativePath.empty())
                    {
                        newPath = directoryUtf8 + '\\' + fileName;
                        newRelativePath = fileName;
                    }
                    else
                    {
                        newPath = directoryUtf8 + '\\' + relativePath + '\\' + fileName;
                        newRelativePath = relativePath + '\\' + fileName;
                    }
                    GetAllFilesInDirectory(newPath, newRelativePath, files);
                }
            }
            else
            {
                ULARGE_INTEGER fileTime;
                fileTime.HighPart = fileData.ftLastWriteTime.dwHighDateTime;
                fileTime.LowPart = fileData.ftLastWriteTime.dwLowDateTime;
                if (!relativePath.empty())
                {
                    fileName = relativePath + '\\' + fileName;
                }
                files.insert(std::make_pair(static_cast<std::uint64_t>(fileTime.QuadPart), std::move(fileName)));
            }
        } while (FindNextFileW(fileHandle, &fileData) != 0);
        FindClose(fileHandle);
    }
    return;
}