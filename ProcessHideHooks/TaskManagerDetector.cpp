#include "stdafx.h"
#include "TaskManagerDetector.h"
//#include "..\ProcessHideLib\Api.h"
//#include "..\ProcessHideLib\Utils.h"

std::unique_ptr<TaskManagerDetector> TaskManagerDetector::m_instance(nullptr);

typedef struct _MY_SYSTEM_PROCESS_INFORMATION
{
    ULONG                   NextEntryOffset;
    ULONG                   NumberOfThreads;
    LARGE_INTEGER           Reserved[3];
    LARGE_INTEGER           CreateTime;
    LARGE_INTEGER           UserTime;
    LARGE_INTEGER           KernelTime;
    UNICODE_STRING          ImageName;
    ULONG                   BasePriority;
    HANDLE                  ProcessId;
    HANDLE                  InheritedFromProcessId;
} MY_SYSTEM_PROCESS_INFORMATION, *PMY_SYSTEM_PROCESS_INFORMATION;

NTSTATUS(__stdcall * CurNtQuerySystemInformation)(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL) = nullptr;

DWORD ACLEntriesTimedNtQuerySystemInformation[1] = { (DWORD)-1 };
HOOK_TRACE_INFO g_hTimedNtQuerySystemInformationHook = { NULL };


std::unique_ptr<TaskManagerDetector> & TaskManagerDetector::GetInstance()
{
    if (m_instance.get() == nullptr)
    {
        m_instance.reset(new TaskManagerDetector);
    }
    return m_instance;
}

bool TaskManagerDetector::InstallHooks()
{
    CurNtQuerySystemInformation = (NTSTATUS(WINAPI *)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG))GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtQuerySystemInformation");
    if (CurNtQuerySystemInformation && LhInstallHook(
        (void*)CurNtQuerySystemInformation,
        (void*)TimedNtQuerySystemInformation,
        NULL,
        &g_hTimedNtQuerySystemInformationHook) == 0)
    {
        LhSetExclusiveACL(ACLEntriesTimedNtQuerySystemInformation, 1, &g_hTimedNtQuerySystemInformationHook);
    }
    
    return false;
}

NTSTATUS TaskManagerDetector::TimedNtQuerySystemInformation(IN SYSTEM_INFORMATION_CLASS SystemInformationClass, OUT PVOID SystemInformation, IN ULONG SystemInformationLength, OUT PULONG ReturnLength OPTIONAL)
{
   /* NTSTATUS status = CurNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
    PMY_SYSTEM_PROCESS_INFORMATION pCurrent = NULL;
    PMY_SYSTEM_PROCESS_INFORMATION pNext = NULL;
    bool findProcess = false;
    std::vector<std::wstring> processList;
    std::vector<PMY_SYSTEM_PROCESS_INFORMATION> newSystemInformation;

    if (NT_SUCCESS(status) && SystemInformationClass == SystemProcessInformation)
    {
        Api::GetHideProcessList(processList);
        pCurrent = reinterpret_cast<PMY_SYSTEM_PROCESS_INFORMATION>(SystemInformation);
        std::wstring imageName;
        while(pCurrent != 0)
        {    
            pNext = reinterpret_cast<PMY_SYSTEM_PROCESS_INFORMATION>((reinterpret_cast<PUCHAR>(pCurrent + pCurrent->NextEntryOffset)));
            imageName = std::wstring(pCurrent->ImageName.Buffer, pCurrent->ImageName.Length);
            for (const auto & processName : processList)
            {
                if (toLowerW(processName).compare(toLowerW(imageName)) == 0)
                {
                    findProcess = true;
                    break;
                }
            }
            if (findProcess)
            {
                pCurrent->NextEntryOffset += pNext->NextEntryOffset;
                findProcess = false;
            }
            pCurrent = reinterpret_cast<PMY_SYSTEM_PROCESS_INFORMATION>((reinterpret_cast<PUCHAR>(pCurrent + pCurrent->NextEntryOffset)));
        }
    }*/

    return NTSTATUS();
}
