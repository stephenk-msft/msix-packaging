//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "UnbundleTestData.hpp"
#include "FileHelpers.hpp"

#include <iostream>

void RunUnbundleTest(HRESULT expected, const std::string& bundle, MSIX_VALIDATION_OPTION validation,
    MSIX_PACKUNPACK_OPTION packUnpack, MSIX_APPLICABILITY_OPTIONS applicability, bool clean = true)
{
    std::cout << "Testing: " << std::endl;
    std::cout << "\tBundle: " << bundle << std::endl; 

    auto testData = MsixTest::TestData::GetInstance();

    auto bundlePath = testData->GetPath(MsixTest::TestData::Directory::Unbundle) + "/" + std::string(bundle);
    bundlePath = MsixTest::Directory::PathAsCurrentPlatform(bundlePath);

    auto outputDir = testData->GetPath(MsixTest::TestData::Directory::Output);

    HRESULT actual = UnpackBundle(packUnpack,
                                  validation,
                                  applicability,
                                  const_cast<char*>(bundlePath.c_str()),
                                  const_cast<char*>(outputDir.c_str()));

    CHECK(expected == actual);
    MsixTest::Log::PrintMsixLog(expected, actual);

    // clean directory if succeeded and requested
    if ((actual == S_OK) && clean)
    {
        CHECK(MsixTest::Directory::CleanDirectory(outputDir));
    }
}

TEST_CASE("Unbundle_StoreSigned_Desktop_x86_x64_MoviesTV", "[unbundle]")
{
    HRESULT expected                         = 0x00000000;
    std::string bundle                       = "StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability, false);

    auto outputDir = MsixTest::TestData::GetInstance()->GetPath(MsixTest::TestData::Directory::Output);

    // Verify all the files extracted on disk are correct
    auto files = MsixTest::Unbundle::GetExpectedFilesFullApplicable();
    CHECK(MsixTest::Directory::CompareDirectory(outputDir, files));

    // Verify non expected files aren't present
    auto filesNotApplicable = MsixTest::Unbundle::GetExpectedFilesNoApplicable();
    CHECK(!MsixTest::Directory::CompareDirectory(outputDir, filesNotApplicable));

    // Clean directory
    CHECK(MsixTest::Directory::CleanDirectory(outputDir));
}

TEST_CASE("Unbundle_StoreSigned_Desktop_x86_x64_MoviesTV_pfn", "[unbundle]")
{
    HRESULT expected                         = 0x00000000;
    std::string bundle                       = "StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability, false);

    // The expected folder structure should be <output>/Microsoft.ZuneVideo_2019.6.25071.0_neutral_~_8wekyb3d8bbwe/<files>
    // Append it to the already existing expected files map
    std::string pfn = "Microsoft.ZuneVideo_2019.6.25071.0_neutral_~_8wekyb3d8bbwe/";
    auto files = MsixTest::Unbundle::GetExpectedFilesFullApplicable();
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

TEST_CASE("Unbundle_StoreSigned_Desktop_x86_x64_MoviesTV_lang_applicability_off", "[unbundle]")
{
    HRESULT expected                         = 0x00000000;
    std::string bundle                       = "StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_SKIPLANGUAGE;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability, false);

    auto outputDir = MsixTest::TestData::GetInstance()->GetPath(MsixTest::TestData::Directory::Output);

    // Verify all the files extracted on disk are correct
    // We ran without language applicability, so expect all files to be there.
    auto files = MsixTest::Unbundle::GetExpectedFilesFullApplicable();
    auto filesNotApplicable = MsixTest::Unbundle::GetExpectedFilesNoApplicable();

    std::map<std::string, std::uint64_t> allFiles;
    allFiles.insert(files.begin(), files.end());
    allFiles.insert(filesNotApplicable.begin(), filesNotApplicable.end());
    CHECK(MsixTest::Directory::CompareDirectory(outputDir, allFiles));

    // Clean directory
    CHECK(MsixTest::Directory::CleanDirectory(outputDir));
}

TEST_CASE("Unbundle_BlockMapContainsPayloadPackage", "[unbundle]")
{
    HRESULT expected                         = 0x8bad0051;
    std::string bundle                       = "BlockMapContainsPayloadPackage.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_BlockMapIsMissing", "[unbundle]")
{
    HRESULT expected                         = 0x8bad0033;
    std::string bundle                       = "BlockMapIsMissing.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_BlockMapViolatesSchema", "[unbundle]")
{
    HRESULT expected                         = 0x8bad1002;
    std::string bundle                       = "BlockMapViolatesSchema.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_ContainsNoPayload", "[unbundle]")
{
    HRESULT expected                         = 0x8bad1002;
    std::string bundle                       = "ContainsNoPayload.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_ContainsOnlyResourcePackages", "[unbundle]")
{
    HRESULT expected                         = 0x8bad0061;
    std::string bundle                       = "ContainsOnlyResourcePackages.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_MainBundle", "[unbundle]")
{
    HRESULT expected                         = 0x00000000;
    std::string bundle                       = "MainBundle.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_ManifestIsMissing", "[unbundle]")
{
    HRESULT expected                         = 0x8bad0034;
    std::string bundle                       = "ManifestIsMissing.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_ManifestPackageHasIncorrectSize", "[unbundle]")
{
    HRESULT expected                         = 0x8bad0061;
    std::string bundle                       = "ManifestPackageHasIncorrectSize.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_ManifestViolatesSchema", "[unbundle]")
{
    HRESULT expected                         = 0x8bad1002;
    std::string bundle                       = "ManifestViolatesSchema.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_PayloadPackageHasNonAppxExtension", "[unbundle]")
{
    HRESULT expected                         = 0x8bad0061;
    std::string bundle                       = "PayloadPackageHasNonAppxExtension.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_PayloadPackageIsCompressed", "[unbundle]")
{
    HRESULT expected                         = 0x8bad0061;
    std::string bundle                       = "PayloadPackageIsCompressed.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_PayloadPackageIsEmpty", "[unbundle]")
{
    HRESULT expected                         = 0x8bad0003;
    std::string bundle                       = "PayloadPackageIsEmpty.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_PayloadPackageIsNotAppxPackage", "[unbundle]")
{
    HRESULT expected                         = 0x80070057;
    std::string bundle                       = "PayloadPackageIsNotAppxPackage.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_SignedUntrustedCert-CERT_E_CHAINING", "[unbundle]")
{
    HRESULT expected                         = 0x8bad0042;
    std::string bundle                       = "SignedUntrustedCert-CERT_E_CHAINING.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_BundleWithIntlPackage", "[unbundle]")
{
    HRESULT expected                         = 0x00000000;
    std::string bundle                       = "BundleWithIntlPackage.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}
