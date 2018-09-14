#pragma once
#include "stdafx.h"

// NtQuerySystemInformation(SystemProcessInformation, 0x000002950049edd0, 307200, 0x000000a71577fa60)	STATUS_SUCCESS


class TaskManagerDetector
{
public:
    static std::unique_ptr<TaskManagerDetector> & GetInstance();
    bool InstallHooks();
private:
    TaskManagerDetector() {}
    TaskManagerDetector(const TaskManagerDetector &) = delete;
    TaskManagerDetector(const TaskManagerDetector &&) = delete;
    TaskManagerDetector & operator=(const TaskManagerDetector &) = delete;
    TaskManagerDetector & operator=(const TaskManagerDetector &&) = delete;
    static NTSTATUS WINAPI TimedNtQuerySystemInformation(IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
                                                         OUT PVOID SystemInformation,
                                                         IN ULONG SystemInformationLength,
                                                         OUT PULONG ReturnLength OPTIONAL);
private:
    static std::unique_ptr<TaskManagerDetector> m_instance;
};

