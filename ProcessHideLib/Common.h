#pragma once

#define PROCESS_HIDE_X86_DLL    L"processHide_x86.dll"
#define PROCESS_HIDE_X64_DLL    L"processHide_x64.dll"

static const wchar_t* X86HookDllName = PROCESS_HIDE_X86_DLL;
static const wchar_t* X64HookDllName = PROCESS_HIDE_X64_DLL;

#define CONFIG_REGISTRY_USER_MODE_SUBKEY L"HKLM\\Software\\ProcessHide"
#define CONFIG_REGISTRY_USER_MODE_SUBKEY_S "HKLM\\Software\\ProcessHide"
#define CONFIG_REGISTRY_PROCESS_HIDE_DELIMITER        L';'
#define CONFIG_REGISTRY_PROCESS_HIDE L"ProcessHideList"

#define LOG_KEY_FILE_NAME L"HideProcessLog"
#define LOG_FILE_NAME L"HideProcessLog.txt"
#define LOG_SERVICE_NAME L"ProcessHideSrv"
#define LOG_KEY_FLAGS L"LogFlags"

// flags for messages filtering
#define LOG_FATAL   0x00000001
#define LOG_ERROR   0x00000002
#define LOG_WARNING 0x00000004
#define LOG_INFO    0x00000008
#define LOG_FAILED  0x00000010

#ifdef _DEBUG
#define IF_DEBUG 1
#else
#define IF_DEBUG 0
#endif

