// dllmain.cpp : Defines the entry point for the DLL application.
#include <easyhook.h>
#include <iostream>
#include <future>
#include <winternl.h>
#include <string>
#include <vector>

std::vector<std::wstring> g_hideProcessList;
HOOK_TRACE_INFO hHook = { NULL };
ULONG ACLEntries[1] = { 0 };
HMODULE ntdll = nullptr;
HANDLE thEvent = nullptr;

HOOK_TRACE_INFO hHookMessageBox = { NULL };
ULONG ACLEntriesMessageBox[1] = { 0 };

NTSTATUS(__stdcall * procHandle)(IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID                   SystemInformation,
    IN ULONG                    SystemInformationLength,
    OUT PULONG ReturnLength     OPTIONAL) = nullptr;


NTSTATUS __stdcall HookNtQuerySystemInformation(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID                   SystemInformation,
    IN ULONG                    SystemInformationLength,
    OUT PULONG ReturnLength     OPTIONAL);

void WorkerThread();
DWORD LoadFromRegistryHideProcessList();
std::wstring GetCurrentProcessName();
bool IsCurrentProcessNeedToHide(const std::wstring & processName);


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

void WorkerThread()
{
    std::cout << "Start worker thread" << std::endl;
    // Install the hook
    NTSTATUS result = LhInstallHook(
    procHandle,
    HookNtQuerySystemInformation,
    NULL,
    &hHook);
    if (FAILED(result))
    {
    std::cout << "Install NtQuerySystemInformation was failed" << std::endl;
    return;
    }
    LhSetInclusiveACL(ACLEntries, 1, &hHook);

    WaitForSingleObject(thEvent, INFINITE);
    std::cout << "End worker thread" << std::endl;
}

NTSTATUS __stdcall HookNtQuerySystemInformation(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID                   SystemInformation,
    IN ULONG                    SystemInformationLength,
    OUT PULONG ReturnLength     OPTIONAL)
{
    std::cout << "It's mine hook" << std::endl;
    NTSTATUS st = procHandle(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
    return st;
}

DWORD LoadFromRegistryHideProcessList()
{
    // Here is needed to load from the registry

    g_hideProcessList.emplace_back(L"calc.exe");
    g_hideProcessList.emplace_back(L"notepad.exe");

    return 1L;
}

std::wstring GetCurrentProcessName()
{
    return std::wstring();
}

bool IsCurrentProcessNeedToHide(const std::wstring & processName)
{
    return false;
}
