#pragma once
#include "stdafx.h"

class LogWriter
{
public:
    static LogWriter * GetInstance();
    void SetFilterFlags(unsigned int flags);
    void Print(unsigned long messagePrintFlags,
        const std::wstring & outputStr,
        unsigned long pid,
        unsigned long tid,
        const std::wstring & processName);
    ~LogWriter();
private:
    LogWriter(const std::wstring & FilePath, unsigned int flags);
    LogWriter(const LogWriter &) = delete;
    LogWriter(const LogWriter &&) = delete;
    LogWriter & operator=(const LogWriter &) = delete;
    LogWriter & operator=(const LogWriter &&) = delete;
    std::wstring GetFilterFlagName(const unsigned long & flags);
    std::wstring GetLocalTime();
    static LSTATUS LoadLogConfiguration(std::wstring & fileName, unsigned int & filterFlags);
    static LSTATUS LoadLogConfigurationXPOrGreater(std::wstring & fileName, unsigned int & filterFlags);
private:
    static LogWriter * m_instance;
    std::wofstream m_out;
    unsigned long m_filterFlags;
    std::wstring m_fPath;
    std::mutex m_mtx;
};

#define LOG_PRINT(message) LogWriter::GetInstance()->Print(LOG_FATAL, message, GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName()); 

