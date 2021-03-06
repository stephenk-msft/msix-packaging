#include "ProcessPotentialUpdate.hpp"
#include <filesystem>
#include "MsixTraceLoggingProvider.hpp"
#include "Database.hpp"

using namespace MsixCoreLib;
const PCWSTR ProcessPotentialUpdate::HandlerName = L"ProcessPotentialUpdate";

HRESULT ProcessPotentialUpdate::ExecuteForAddRequest()
{
    /// This design chooses the simplest solution of removing the existing package in the family before proceeding with the install
    /// This is currently good enough for our requirements; it leverages existing removal codepaths.
    /// An alternate, more complicated design would have each handler to expose a new Update verb (e.g. ExecuteForUpdate that takes in the old package)
    /// and each handler would have the opportunity to reason between the old and new packages to perform more efficient updating.
    std::wstring currentPackageFamilyName = m_msixRequest->GetPackageInfo()->GetPackageFamilyName();

    std::vector<std::wstring> installedPackages;
    RETURN_IF_FAILED(Database::FindPackagesForCurrentUser(installedPackages));

    for (auto& package : installedPackages)
    {
        std::wstring installedPackageFamilyName = GetFamilyNameFromFullName(package);
        if (CaseInsensitiveEquals(currentPackageFamilyName, installedPackageFamilyName)
            && !CaseInsensitiveEquals(m_msixRequest->GetPackageInfo()->GetPackageFullName(), package))
        {
            RETURN_IF_FAILED(RemovePackage(package));
            return S_OK;
        }
    }

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Not an update, nothing to do.");
    return S_OK;
}

HRESULT ProcessPotentialUpdate::RemovePackage(std::wstring packageFullName)
{
    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Found an update to an existing package, removing package",
        TraceLoggingValue(packageFullName.c_str(), "PackageToBeRemoved"));

    AutoPtr<MsixRequest> localRequest;
    RETURN_IF_FAILED(MsixRequest::Make(OperationType::Remove, nullptr, packageFullName, MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &localRequest));

    const HRESULT hrProcessRequest = localRequest->ProcessRequest();
    if (FAILED(hrProcessRequest))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Failed to remove package",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(hrProcessRequest, "HR"));
    }

    return S_OK;
}

HRESULT ProcessPotentialUpdate::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<ProcessPotentialUpdate> localInstance(new ProcessPotentialUpdate(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}