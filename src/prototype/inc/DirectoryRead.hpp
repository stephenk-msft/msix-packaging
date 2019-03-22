//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
// This will probably end up in DiretoryObject
#pragma once

#include <string>
#include <map>

// Get directory, return vector with all files
void GetAllFilesInDirectory(std::string& directory, std::string& relativePath, std::multimap<std::uint64_t, std::string>& files);
