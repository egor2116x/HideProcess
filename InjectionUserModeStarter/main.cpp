#include <Windows.h>
#include "Common.h"
#include <string>
#include <memory>

BOOL UserModeInject();

int wmain(int argc, wchar_t ** argv)
{
    std::wstring cmd;
    for (auto i = 0; i < argc; i++)
    {
        cmd += argv[i];
    }

    if (cmd.find(L"inject") != std::wstring::npos)
    {
        if (!UserModeInject())
        {
            return 1;
        }       
        system("pause");
    }
    return 0;
}

BOOL UserModeInject()
{
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
    bool(WINAPI * CurStart)() = nullptr;
#ifdef _M_X64
    hModule = GetModuleHandle(X64HookDllName);
    CurStart = (bool(WINAPI *)())GetProcAddress(hModule, "?Start@@YA_NXZ");
#else
    hModule = GetModuleHandle(X86HookDllName);
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