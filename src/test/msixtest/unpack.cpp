//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "UnpackTestData.hpp"
#include "FileHelpers.hpp"

#include <iostream>

void RunUnpackTest(HRESULT expected, const std::string& package, MSIX_VALIDATION_OPTION validation,
    MSIX_PACKUNPACK_OPTION packUnpack, bool clean = true)
{
    std::cout << "Testing: " << std::endl;
    std::cout << "\tPackage:" << package << std::endl; 

    auto testData = MsixTest::TestData::GetInstance();

    auto packagePath = testData->GetPath(MsixTest::TestData::Directory::Unpack) + "/" + std::string(package);
    packagePath = MsixTest::Directory::PathAsCurrentPlatform(packagePath);

    auto outputDir = testData->GetPath(MsixTest::TestData::Directory::Output);

    HRESULT actual = UnpackPackage(packUnpack,
                                   validation,
                                   const_cast<char*>(packagePath.c_str()),
                                   const_cast<char*>(outputDir.c_str()));

    CHECK(expected == actual);
    MsixTest::Log::PrintMsixLog(expected, actual);

    // clean directory if succeeded and requested
    if ((actual == S_OK) && clean)
    {
        CHECK(MsixTest::Directory::CleanDirectory(outputDir));
    }
}

// End-to-end unpacking tests
TEST_CASE("Unpack_StoreSigned_Desktop_x64_MoviesTV", "[unpack]")
{
    HRESULT expected                  = S_OK;
    std::string package               = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack, false);

    // Verify all the files extracted on disk are correct
    auto files = MsixTest::Unpack::GetExpectedFiles();
    auto outputDir = MsixTest::TestData::GetInstance()->GetPath(MsixTest::TestData::Directory::Output);
    CHECK(MsixTest::Directory::CompareDirectory(outputDir, files));

    // Clean directory
    CHECK(MsixTest::Directory::CleanDirectory(outputDir));
}

TEST_CASE("Unpack_StoreSigned_Desktop_x64_MoviesTV_pfn", "[unpack]")
{
    HRESULT expected                  = S_OK;
    std::string package               = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER;

    RunUnpackTest(expected, package, validation, packUnpack, false);

    // Verify all the files extracted on disk are correct
    auto files = MsixTest::Unpack::GetExpectedFiles();
    // The expected folder structure should be <output>/Microsoft.ZuneVideo_3.6.25071.0_x64__8wekyb3d8bbwe/<files>
    // Append it to the already existing expected files map
    std::string pfn = "Microsoft.ZuneVideo_3.6.25071.0_x64__8wekyb3d8bbwe/";
    std::map<std::string, uint64_t> filesWithPfn;
    for (const auto& file : files)
    {
        filesWithPfn.emplace(pfn + file.first, file.second);
    }

    auto outputDir = MsixTest::TestData::GetInstance()->GetPath(MsixTest::TestData::Directory::Output);
    CHECK(MsixTest::Directory::CompareDirectory(outputDir, filesWithPfn));

    // Clean directory
    CHECK(MsixTest::Directory::CleanDirectory(outputDir));
}

TEST_CASE("Unpack_Empty", "[unpack]")
{
    HRESULT expected                  = 0x8bad0002;
    std::string package               = "Empty.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_HelloWorld", "[unpack]")
{
    HRESULT expected                  = 0x00000000;
    std::string package               = "HelloWorld.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_NotepadPlusPlus", "[unpack]")
{
    HRESULT expected                  = 0x00000000;
    std::string package               = "NotepadPlusPlus.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_IntlPackage", "[unpack]")
{
    HRESULT expected                  = 0x00000000;
    std::string package               = "IntlPackage.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_SignatureNotLastPart-ERROR_BAD_FORMAT", "[unpack]")
{
    HRESULT expected                  = 0x8bad0042;
    std::string package               = "SignatureNotLastPart-ERROR_BAD_FORMAT.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_SignedTamperedBlockMap-TRUST_E_BAD_DIGEST", "[unpack]")
{
    HRESULT expected                  = 0x8bad0042;
    std::string package               = "SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_SignedTamperedBlockMap-TRUST_E_BAD_DIGEST_sv", "[unpack]")
{
    HRESULT expected                  = 0x8bad0041;
    std::string package               = "SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_SignedTamperedCD-TRUST_E_BAD_DIGEST", "[unpack]")
{
    HRESULT expected                  = 0x8bad0042;
    std::string package               = "SignedTamperedCD-TRUST_E_BAD_DIGEST.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_SignedUntrustedCert", "[unpack]")
{
    HRESULT expected                  = 0x8bad0042;
    std::string package               = "SignedUntrustedCert-CERT_E_CHAINING.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_TestAppxPackage_Win32", "[unpack]")
{
    HRESULT expected                  = 0x00000000;
    std::string package               = "TestAppxPackage_Win32.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_TestAppxPackage_x64", "[unpack]")
{
    HRESULT expected                  = 0x00000000;
    std::string package               = "TestAppxPackage_x64.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_UnsignedZip64WithCI-APPX_E_MISSING_REQUIRED_FILE", "[unpack]")
{
    HRESULT expected                  = 0x8bad0012; // TODO: change this to 0x8bad0031 when merge with packaging
    std::string package               = "UnsignedZip64WithCI-APPX_E_MISSING_REQUIRED_FILE.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_FileDoesNotExist", "[unpack]")
{
    HRESULT expected                  = 0x8bad0001;
    std::string package               = "FileDoesNotExist.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap/Missing_Manifest_in_blockmap", "[unpack]")
{
    HRESULT expected                  = 0x8bad0051;
    std::string package               = "BlockMap/Missing_Manifest_in_blockmap.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap/ContentTypes_in_blockmap", "[unpack]")
{
    HRESULT expected                  = 0x8bad0051;
    std::string package               = "BlockMap/ContentTypes_in_blockmap.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap/Invalid_Bad_Block", "[unpack]")
{
    HRESULT expected                  = 0x8bad0051;
    std::string package               = "BlockMap/Invalid_Bad_Block.msix";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap/Size_wrong_uncompressed", "[unpack]")
{
    HRESULT expected                  = 0x8bad0051;
    std::string package               = "BlockMap/Size_wrong_uncompressed.msix";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap/Extra_file_in_blockmap", "[unpack]")
{
    HRESULT expected                  = 0x80070002;
    std::string package               = "BlockMap/Extra_file_in_blockmap.msix";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap/File_missing_from_blockmap", "[unpack]")
{
    HRESULT expected                  = 0x8bad0051;
    std::string package               = "BlockMap/File_missing_from_blockmap.msix";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap/No_blockmap.appx", "[unpack]")
{
    HRESULT expected                  = 0x8bad0033;
    std::string package               = "BlockMap/No_blockmap.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap/Bad_Namespace_Blockmap", "[unpack]")
{
    HRESULT expected                  = 0x8bad1003;
    std::string package               = "BlockMap/Bad_Namespace_Blockmap.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap/Duplicate_file_in_blockmap", "[unpack]")
{
    HRESULT expected                  = 0x8bad0051;
    std::string package               = "BlockMap/Duplicate_file_in_blockmap.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_IntlCharsInPath", "[unpack]")
{
    HRESULT expected                  = S_OK;
    std::string package               = "महसुस/StoreSigned_Desktop_x64_MoviesTV.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}
