//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "unpack_testdata.hpp"

#include <iostream>

void RunUnpackTest(HRESULT expected, const char* package, MSIX_VALIDATION_OPTION option, bool clean = true)
{
    std::cout << "Testing: " << std::endl;
    std::cout << "\tPackage:" << package << std::endl; 

    auto testData = MsixTest::TestData::GetInstance();

    auto packagePath = testData->GetPath(MsixTest::TestData::Directory::Unpack) + "/" + std::string(package);
    packagePath = MsixTest::Directory::PathAsCurrentPlatform(packagePath);

    auto outputDir = testData->GetPath(MsixTest::TestData::Directory::Output);

    HRESULT actual = UnpackPackage(MSIX_PACKUNPACK_OPTION_NONE,
                                   option,
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
    HRESULT expected              = S_OK;
    const char* package           = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_FULL;

    RunUnpackTest(expected, package, option, false);

    // Compare that all the files extracted on disk are correct
    auto files = MsixTest::Unpack::GetExpectedFiles();
    auto outputDir = MsixTest::TestData::GetInstance()->GetPath(MsixTest::TestData::Directory::Output);
    CHECK(MsixTest::Directory::CompareDirectory(outputDir, files));

    // Clean directory
    CHECK(MsixTest::Directory::CleanDirectory(outputDir));
}

TEST_CASE("Unpack_Empty", "[unpack]")
{
    HRESULT expected              = 0x8bad0002;
    const char* package           = "Empty.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_HelloWorld", "[unpack]")
{
    HRESULT expected = 0x00000000;
    const char* package = "HelloWorld.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_NotepadPlusPlus", "[unpack]")
{
    HRESULT expected = 0x00000000;
    const char* package = "NotepadPlusPlus.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_IntlPackage", "[unpack]")
{
    HRESULT expected = 0x00000000;
    const char* package = "IntlPackage.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_SignatureNotLastPart-ERROR_BAD_FORMAT", "[unpack]")
{
    HRESULT expected = 0x8bad0042;
    const char* package = "SignatureNotLastPart-ERROR_BAD_FORMAT.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_FULL;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_SignedTamperedBlockMap-TRUST_E_BAD_DIGEST", "[unpack]")
{
    HRESULT expected = 0x8bad0042;
    const char* package = "SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_FULL;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_SignedTamperedBlockMap-TRUST_E_BAD_DIGEST_sv", "[unpack]")
{
    HRESULT expected = 0x8bad0041;
    const char* package = "SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_SignedTamperedCD-TRUST_E_BAD_DIGEST", "[unpack]")
{
    HRESULT expected = 0x8bad0042;
    const char* package = "SignedTamperedCD-TRUST_E_BAD_DIGEST.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_FULL;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_SignedUntrustedCert", "[unpack]")
{
    HRESULT expected = 0x8bad0042;
    const char* package = "SignedUntrustedCert-CERT_E_CHAINING.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_FULL;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_TestAppxPackage_Win32", "[unpack]")
{
    HRESULT expected = 0x00000000;
    const char* package = "TestAppxPackage_Win32.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_TestAppxPackage_x64", "[unpack]")
{
    HRESULT expected = 0x00000000;
    const char* package = "TestAppxPackage_x64.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_UnsignedZip64WithCI-APPX_E_MISSING_REQUIRED_FILE", "[unpack]")
{
    HRESULT expected = 0x8bad0012; // TODO: change this to 0x8bad0031 when merge with packaging
    const char* package = "UnsignedZip64WithCI-APPX_E_MISSING_REQUIRED_FILE.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_FULL;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_FileDoesNotExist", "[unpack]")
{
    HRESULT expected = 0x8bad0001;
    const char* package = "FileDoesNotExist.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_BlockMap/Missing_Manifest_in_blockmap", "[unpack]")
{
    HRESULT expected = 0x8bad0051;
    const char* package = "BlockMap/Missing_Manifest_in_blockmap.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_BlockMap/ContentTypes_in_blockmap", "[unpack]")
{
    HRESULT expected = 0x8bad0051;
    const char* package = "BlockMap/ContentTypes_in_blockmap.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_BlockMap/Invalid_Bad_Block", "[unpack]")
{
    HRESULT expected = 0x8bad0051;
    const char* package = "BlockMap/Invalid_Bad_Block.msix";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_BlockMap/Size_wrong_uncompressed", "[unpack]")
{
    HRESULT expected = 0x8bad0051;
    const char* package = "BlockMap/Size_wrong_uncompressed.msix";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_BlockMap/Extra_file_in_blockmap", "[unpack]")
{
    HRESULT expected = 0x80070002;
    const char* package = "BlockMap/Extra_file_in_blockmap.msix";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_BlockMap/File_missing_from_blockmap", "[unpack]")
{
    HRESULT expected = 0x8bad0051;
    const char* package = "BlockMap/File_missing_from_blockmap.msix";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;


    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_BlockMap/No_blockmap.appx", "[unpack]")
{
    HRESULT expected = 0x8bad0033;
    const char* package = "BlockMap/No_blockmap.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_BlockMap/Bad_Namespace_Blockmap", "[unpack]")
{
    HRESULT expected = 0x8bad1003;
    const char* package = "BlockMap/Bad_Namespace_Blockmap.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;

    RunUnpackTest(expected, package, option);
}

TEST_CASE("Unpack_BlockMap/Duplicate_file_in_blockmap", "[unpack]")
{
    HRESULT expected = 0x8bad0051;
    const char* package = "BlockMap/Duplicate_file_in_blockmap.appx";
    MSIX_VALIDATION_OPTION option = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;

    RunUnpackTest(expected, package, option);
}

