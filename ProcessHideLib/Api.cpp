#include "Api.h"
#include "SCmanagerWraper.h"
#include "Utils.h"

namespace
{
    SCManagerWrapper g_scManager;
    CServiceHandle g_service;
}

std::wstring GetSystemDirPathWow64()
{

    WCHAR SystemWow64Directory[MAX_PATH + 1];

    UINT(WINAPI * CurGetSystemWow64Directory)(__out LPTSTR lpBuffer, __in UINT uSize) = NULL;
    CurGetSystemWow64Directory =
        (UINT(WINAPI *)(LPTSTR, UINT))GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "GetSystemWow64DirectoryW");
    if (!CurGetSystemWow64Directory)
        return L"";

    UINT sz = CurGetSystemWow64Directory(SystemWow64Directory, MAX_PATH + 1);
    if (sz == 0 || GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
    {
        return L"";
    }

    return std::wstring(SystemWow64Directory, sz);
}

BOOL IfFileExist(const std::wstring & fullPathToFile)
{
    HANDLE hFile = ::CreateFile(fullPathToFile.c_str(),
        FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, // security attributes
        OPEN_EXISTING, // disposition
        FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN, // flags & attributes
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    ::CloseHandle(hFile);
    return TRUE;
}

BOOL MoveDllEx(const std::wstring & dllSysDirPath, const std::wstring & dllName)
{
    BOOL res = FALSE;
    std::wstring Target = dllSysDirPath;
    Target += L'\\';
    Target += dllName;

    if (!IfFileExist(Target))
    {
        return res;
    }

    WCHAR buff[MAX_PATH];
    UINT sz = GetTempFileName(dllSysDirPath.c_str(),
        dllName.c_str(),
        0,
        buff);
    if (sz == 0)
    {
        return res;
    }

    res = DeleteFile(buff); // delete empty tmp file, because rename will be fail
    if (!res)
    {
        return res;
    }

    //rename
    res = MoveFile(Target.c_str(), buff);
    if (!res)
    {
        return res;
    }

    //delete
    res = MoveFileEx(buff, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    if (!res)
    {
        return res;
    }
    return res;
}

BOOL MoveDll(const std::wstring & dllSysDirPath, int bitness)
{
    BOOL result = FALSE;
    DWORD error;

    if (bitness == 32)
        result = MoveDllEx(dllSysDirPath, PROCESS_HIDE_X86_DLL);
    else
        result = MoveDllEx(dllSysDirPath, PROCESS_HIDE_X64_DLL);

    if (!result)
    {
        error = GetLastError();
    }
    return result;
}

BOOL UnInstallDllFromSys32(int bitness)
{
    BOOL res = FALSE;

    WCHAR buff[MAX_PATH + 1];

    UINT sz = GetSystemDirectory(buff, MAX_PATH + 1); // system32 dir
    if (sz == 0)
    {
        return res;
    }

    res = MoveDll(buff, bitness);

    return res;
}

BOOL UnInstallDllFromSysWow64()
{
    BOOL res = FALSE;

    std::wstring buff = GetSystemDirPathWow64(); // syswow64

    res = MoveDll(buff, 32);

    return res;
}

BOOL UninstallHookDlls()
{
    if (!GetSystemDirPathWow64().empty()) // if windows_x64
    {
        BOOL resSys32 = UnInstallDllFromSys32(64);
        BOOL resSysWow64 = UnInstallDllFromSysWow64();
        return (resSys32 || resSysWow64);
    }

    return  UnInstallDllFromSys32(32);
}

BOOL CopyFromTo(const std::wstring & dllSourceDirPath, const std::wstring & dllSysDirPath, int bitness)
{
    BOOL res = FALSE;

    std::wstring Source = dllSourceDirPath;
    Source += L'\\';

    if (bitness == 32)
        Source += PROCESS_HIDE_X86_DLL;
    else
        Source += PROCESS_HIDE_X64_DLL;

    std::wstring Target = dllSysDirPath;
    Target += L'\\';

    if (bitness == 32)
        Target += PROCESS_HIDE_X86_DLL;
    else
        Target += PROCESS_HIDE_X64_DLL;

    res = CopyFile(Source.c_str(), Target.c_str(), FALSE);
    DWORD err;
    if (!res)
    {
        err = GetLastError();
        return res;
    }
    return res;
}

BOOL InstallDllToSys32(const std::wstring & dllSourceDirPath, int bitness)
{
    BOOL res = FALSE;
    if (dllSourceDirPath.empty())
    {
        return res;
    }

    WCHAR buff[MAX_PATH + 1];

    UINT sz = GetSystemDirectory(buff, MAX_PATH + 1); // system32 dir
    if (sz == 0)
    {
        return res;
    }

    res = CopyFromTo(dllSourceDirPath, buff, bitness);

    return res;
}

BOOL InstallDllToSysWow64(const std::wstring & dllSourceDirPath)
{
    BOOL res = FALSE;
    if (dllSourceDirPath.empty())
    {
        return res;
    }

    res = CopyFromTo(dllSourceDirPath, GetSystemDirPathWow64(), 32);

    return res;
}

BOOL InstallHookDlls(const std::wstring & dllSourceDirPathX86, const std::wstring & dllSourceDirPathX64)
{
    if (!GetSystemDirPathWow64().empty()) // if windows_x64
    {
        return (InstallDllToSys32(dllSourceDirPathX64, 64) &&
            InstallDllToSysWow64(dllSourceDirPathX86));
    }

    return  (InstallDllToSys32(dllSourceDirPathX86, 32));
}

BOOL Api::Install(const std::wstring & dllSourceDirPathX86, const std::wstring & dllSourceDirPathX64)
{
    BOOL result = InstallHookDlls(dllSourceDirPathX86, dllSourceDirPathX64);
    if (!result)
    {
        return result;
    }

#ifdef _M_X64
    HMODULE hModule = LoadLibraryW(X64HookDllName);
#else
    HMODULE hModule = LoadLibrary(X86HookDllName);
#endif
    if (hModule == nullptr)
    {
        DWORD result = GetLastError();
        result = FALSE;
    }
    return result;
}

BOOL Api::Uninstall()
{
#ifdef _M_X64
    HMODULE hModule = GetModuleHandle(X64HookDllName);
#else
    HMODULE hModule = GetModuleHandle(X86HookDllName);
#endif

    if (hModule == nullptr)
    {
        return FALSE;
    }
    FreeLibrary(hModule);

    return UninstallHookDlls();
}

BOOL Api::Inject()
{
    bool(WINAPI * CurStart)() = nullptr;
#ifdef _M_X64
    HMODULE hModule = GetModuleHandle(X64HookDllName);
    CurStart = (bool(WINAPI *)())GetProcAddress(hModule, "?Start@@YA_NXZ");
#else
    HMODULE hModule = GetModuleHandle(X86HookDllName);
    CurStart = (bool(WINAPI *)())GetProcAddress(hModule, "?Start@@YA_NXZ");
#endif
    if (hModule == nullptr)
    {
        DWORD result = GetLastError();
        return FALSE;
    }
    //
    // Inject support dll
    //
    if (CurStart != nullptr)
    {
        bool result = CurStart();
        if (!result)
        {
            return FALSE;
        }

    }

    return TRUE;
}

bool Api::installSrv()
{
    g_service.m_h = ::OpenService(g_scManager.GetSCManagerHandle(), LOG_SERVICE_NAME, SERVICE_QUERY_STATUS);
    if (g_service.m_h != nullptr)
    {
        std::wcout << L"Service already exists" << std::endl;
        std::wcout << L">> ";
        return true;
    }

    std::wstring fullpath(GetFullCurrentProcessPathToFolder());
    fullpath += L'\\';
    fullpath += LOG_SERVICE_BIN_NAME;
    g_scManager.CreateService(LOG_SERVICE_NAME,
        LOG_SERVICE_NAME,
        fullpath.c_str(),
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_DEMAND_START,
        true);
    return true;
}

void Api::startSrv()
{
    g_scManager.StartService(LOG_SERVICE_NAME);
}

void Api::stopSrv()
{
    g_scManager.StopService(LOG_SERVICE_NAME);
}

bool Api::SetHideProcessList(const std::vector<std::wstring>& processList)
{
    if (processList.empty())
    {
        return false;
    }

    LSTATUS status = 0;
    HKEY  hKey;

    status = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        CONFIG_REGISTRY_USER_MODE_SUBKEY,
        0,
        KEY_READ | KEY_WRITE | KEY_QUERY_VALUE,
        &hKey);

    if (status != ERROR_SUCCESS)
    {
        return false;
    }
    
    std::wstring buff;
    for (const auto & name : processList)
    {
        buff += name;
        buff += L'\0';
    }
    buff += L'\0';

    status = RegSetValueExW(hKey, CONFIG_REGISTRY_USER_MODE_SUBKEY, 0, REG_MULTI_SZ, reinterpret_cast<const BYTE*>(&buff[0]), buff.size() / sizeof(wchar_t));
    if (status != ERROR_SUCCESS)
    {
        return false;
    }

    return true;
}

bool Api::GetHideProcessList(std::vector<std::wstring>& processList)
{
    return LoadMultiString(CONFIG_REGISTRY_PROCESS_HIDE, processList);
}
