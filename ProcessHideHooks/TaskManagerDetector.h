#pragma once
#include "stdafx.h"

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
    static NTSTATUS WINAPI TimedNtQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass,
                                                         PVOID SystemInformation,
                                                         ULONG SystemInformationLength,
                                                         PULONG ReturnLength);
private:
    static std::unique_ptr<TaskManagerDetector> m_instance;
};

