#include "LogWriter.h"
#include "Utils.h"
#include "ExceptionService.h"

LogWriter * LogWriter::m_instance = nullptr;

LogWriter * LogWriter::GetInstance()
{
    if (m_instance == nullptr)
    {
        std::wstring fName;
        unsigned int flags = 0;
        LSTATUS st = LoadLogConfigurationXPOrGreater(fName, flags);
        if (st != ERROR_SUCCESS)
        {
            throw WriterLoggerError("LoadLogConfiguration failed", static_cast<DWORD>(st));
        }
        m_instance = new LogWriter(fName, flags);
    }
    return m_instance;
}

void LogWriter::SetFilterFlags(unsigned int flags)
{
    m_filterFlags = flags;
}

LogWriter::LogWriter(const std::wstring & fileName, unsigned int flags) : m_filterFlags(flags)
{
    m_fPath += GetFullCurrentProcessPathToFolder();
    m_fPath += L'\\';
    m_fPath += fileName;
    m_out.open(m_fPath, std::ios::app);
}

std::wstring LogWriter::GetFilterFlagName(const unsigned long & flags)
{
    std::wstring flagNames;
    if (flags & LOG_FATAL)
    {
        flagNames += L"FATAL ";
    }
    if (flags & LOG_ERROR)
    {
        flagNames += L"ERROR ";
    }
    if (flags & LOG_WARNING)
    {
        flagNames += L"WARN ";
    }
    if (flags & LOG_INFO)
    {
        flagNames += L"INFO ";
    }
    if (flags & LOG_FAILED)
    {
        flagNames += L"FAILED ";
    }
    flagNames = flagNames.substr(0, flagNames.size() - 1); // remove last whitespace
    flagNames += L'\t';
    return flagNames;
}

std::wstring LogWriter::GetLocalTime()
{
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto timer = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&timer);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << std::endl;

    std::wstring localTime = str2wstr(ss.str());
    localTime = localTime.substr(0, localTime.size() - 1); // remove \n
    localTime += L'.';
    localTime += std::to_wstring(ms.count()); // add millisec

    localTime += L'\t';
    return localTime;
}

LSTATUS LogWriter::LoadLogConfiguration(std::wstring & fileName, unsigned int & filterFlags)
{
    std::wstring buff;
    DWORD size = 0;
    LSTATUS st = ERROR_SUCCESS;
    for (;;)
    {
        st = RegGetValueW(HKEY_LOCAL_MACHINE,
            CONFIG_REGISTRY_USER_MODE_SUBKEY,
            LOG_KEY_FILE_NAME,
            RRF_RT_REG_SZ,
            NULL,
            reinterpret_cast<PVOID>(&buff[0]),
            &size);
        if (!buff.empty())
        {
            break;
        }
        buff.resize(size / sizeof(wchar_t));
    }

    if (st != ERROR_SUCCESS)
    {
        return st;
    }

    fileName = buff;

    DWORD value = 0;
    size = sizeof(value);

    st = RegGetValueW(HKEY_LOCAL_MACHINE,
        CONFIG_REGISTRY_USER_MODE_SUBKEY,
        LOG_KEY_FLAGS,
        RRF_RT_REG_DWORD,
        NULL,
        reinterpret_cast<PVOID>(&value),
        &size);
    if (st != ERROR_SUCCESS)
    {
        return st;
    }

    filterFlags = value;
    return st;
}

LSTATUS LogWriter::LoadLogConfigurationXPOrGreater(std::wstring & fileName, unsigned int & filterFlags)
{
    HKEY key;
    std::wstring buff;
    DWORD size = 0;
    LSTATUS st = ERROR_SUCCESS;

    st = RegOpenKey(HKEY_LOCAL_MACHINE,
        CONFIG_REGISTRY_USER_MODE_SUBKEY,
        &key);

    if (st != ERROR_SUCCESS)
    {
        return st;
    }

    for (;;)
    {
        st = RegQueryValueEx(key,
            LOG_KEY_FILE_NAME,
            NULL,
            NULL,
            reinterpret_cast<LPBYTE>(&buff[0]),
            &size);
        if (!buff.empty())
        {
            break;
        }
        buff.resize(size / sizeof(wchar_t));
    }

    fileName = buff;

    DWORD value = 0;
    size = sizeof(value);

    st = RegQueryValueEx(key,
        LOG_KEY_FLAGS,
        NULL,
        NULL,
        reinterpret_cast<LPBYTE>(&value),
        &size);

    if (st != ERROR_SUCCESS)
    {
        RegCloseKey(key);
        return st;
    }

    filterFlags = value;
    RegCloseKey(key);

    return st;
}

LogWriter::~LogWriter()
{
    if (m_out.is_open())
    {
        m_out.close();
    }
}

void LogWriter::Print(unsigned long messagePrintFlags,
    const std::wstring & outputStr,
    unsigned long pid,
    unsigned long tid,
    const std::wstring & processName)
{
    if (messagePrintFlags & m_filterFlags)
    {
        std::wstring flagNames = GetFilterFlagName(messagePrintFlags);
        std::wstring localTime = GetLocalTime();

        std::lock_guard<std::mutex> mtxGuard(m_mtx);
        m_out << localTime
            << flagNames
            << L"[pid: " << pid << L"]\t"
            << L"[tid: " << tid << L"]\t"
            << L'[' << processName << L"]\t"
            << outputStr;
    }
}
