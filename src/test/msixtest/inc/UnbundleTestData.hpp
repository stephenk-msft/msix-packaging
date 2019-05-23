//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include <map>
#include <string>

namespace MsixTest { 

    namespace Unbundle {

        // Expected files for full applicability of StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle
        // in a system that only has English as its language
        const std::map<std::string, std::uint64_t>& GetExpectedFilesFullApplicable();

        // Expected files for full applicability of StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle
        // in a system that only has English as its language that are NOT expected to be extracted.
        const std::map<std::string, std::uint64_t>& GetExpectedFilesNoApplicable();
    }
}
