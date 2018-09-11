#pragma once

#define PROCESS_HIDE_X86_DLL    L"processHide_x86.dll"
#define PROCESS_HIDE_X64_DLL    L"processHide_x64.dll"

static const wchar_t* X86HookDllName = PROCESS_HIDE_X86_DLL;
static const wchar_t* X64HookDllName = PROCESS_HIDE_X64_DLL;

#define CONFIG_REGISTRY_USER_MODE_SUBKEY L"HKLM\\Software\\ProcessHide"
#define CONFIG_REGISTRY_USER_MODE_SUBKEY_S "HKLM\\Software\\ProcessHide"
#define CONFIG_REGISTRY_PROCESS_HIDE_DELIMITER        L';'
#define CONFIG_REGISTRY_PROCESS_HIDE L"ProcessHideList"
