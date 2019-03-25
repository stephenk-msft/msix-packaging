//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "DirectoryRead.hpp"
#include "Helpers.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

// one day C++17... one day
void GetAllFilesInDirectory(std::string& directory, std::string& relativePath, std::multimap<std::uint64_t, std::string>& files)
{
    DIR* dir;
    ThrowIf(((dir = opendir(directory.c_str())) == NULL), "Invalid direectory");
    struct dirent* dp;
    while ((dp = readdir(dir)) != NULL)
    {
        std::string fileName = std::string(dp->d_name);
        if (dp->d_type == DT_DIR)
        {
            if ((fileName != ".") && (fileName != ".."))
            {
                std::string newRelativePath;
                std::string newPath;
                if (relativePath.empty())
                {
                    newPath = directory + '/' + fileName;
                    newRelativePath = fileName;
                }
                else
                {
                    newPath = directory + '/' + relativePath + '/' + fileName;
                    newRelativePath = relativePath + '/' + fileName;
                }
                GetAllFilesInDirectory(newPath, newRelativePath, files);
            }
        }
        else
        {
            // TODO: ignore .DS_STORE for mac?
            // Get last modified timestruct stat sb;
            struct stat sb;
            std::string fullPath = directory + "/" + fileName;
            ThrowIf((stat(fullPath.c_str(), &sb)) == -1, "stat call failed" + std::to_string(errno));
            if (!relativePath.empty())
            {
                fileName = relativePath + '/' + fileName;
            }
            files.insert(std::make_pair(static_cast<std::uint64_t>(sb.st_mtime), std::move(fileName)));
        }
    }
    closedir(dir);
}
