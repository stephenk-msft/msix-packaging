#pragma once
#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "RegistryKey.hpp"
#include <vector>

class StartupTask : IPackageHandler
{
public:

    /// Creates a scheduled task to run the executable listed in the windows.startupTask extension when the user logs on.
    HRESULT ExecuteForAddRequest();
    HRESULT ExecuteForRemoveRequest();

    static const PCWSTR HandlerName;
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~StartupTask() {}
private:
    
    MsixRequest* m_msixRequest = nullptr;
    
    std::wstring m_executable;

    /// Parses the manifest to find the executable to run as part of startup task
    HRESULT ParseManifest();

    StartupTask() {}
    StartupTask(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

    static const PCWSTR TaskDefinitionXmlPrefix;
    static const PCWSTR TaskDefinitionXmlPostfix;
};
