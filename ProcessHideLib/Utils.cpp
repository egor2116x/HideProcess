#include "Utils.h"

std::wstring GetFullCurrentProcessPath()
{
    DWORD dwResult = NO_ERROR;
    std::vector<WCHAR> pathBuffer;
    pathBuffer.resize(MAX_PATH);
    std::wstring fullPath;

    for (;;)
    {
        dwResult = GetModuleFileName(NULL, &pathBuffer.at(0), static_cast<DWORD>(pathBuffer.size()));
        if (dwResult == 0)
        {
            return std::wstring();
        }
        else if (dwResult == pathBuffer.size() && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            pathBuffer.resize(pathBuffer.size() * 2);
            continue;
        }
        break;
    }

    fullPath.assign(&pathBuffer.at(0), dwResult);

    return fullPath;
}

std::wstring GetFullCurrentProcessPathToFolder()
{
    std::wstring fullPath = GetFullCurrentProcessPath();

    std::wstring::size_type pos = fullPath.find_last_of(L'\\');
    if (pos == std::wstring::npos)
    {
        return std::wstring();
    }

    return fullPath.substr(0, pos);
}


std::wstring GetCurrentProcessName()
{
    std::wstring fullPath = GetFullCurrentProcessPath();

    std::wstring::size_type pos = fullPath.find_last_of(L'\\');
    if (pos == std::wstring::npos)
    {
        return std::wstring();
    }

    return fullPath.substr(pos + 1, fullPath.size() - pos);
}

std::string wstr2str(const std::wstring & str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(str);
}
std::wstring str2wstr(const std::string & str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str.c_str());
}

BOOL GenerateDump(EXCEPTION_POINTERS* pExceptionPointers)
{
    BOOL bMiniDumpSuccessful = FALSE;
    std::wstring fPath(GetFullCurrentProcessPathToFolder());
    WCHAR szFileName[MAX_PATH];
    DWORD dwBufferSize = MAX_PATH;
    HANDLE hDumpFile;
    SYSTEMTIME stLocalTime;
    MINIDUMP_EXCEPTION_INFORMATION ExpParam;

    GetLocalTime(&stLocalTime);
    UINT res = GetTempFileName(fPath.c_str(),
        L"CRASH_",
        NULL,
        szFileName);
    DWORD err = 0;
    if (!res)
    {
        err = GetLastError();
    }
    DeleteFile(szFileName);
    std::wstring nTmp(szFileName);
    std::wstring::size_type pos = nTmp.find_last_of(L'.');
    if (pos != std::wstring::npos)
    {
        nTmp = nTmp.substr(0, pos - 1);
        nTmp += L".dmp";
    }
    hDumpFile = CreateFile(nTmp.c_str(), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

    if (hDumpFile != NULL)
    {
        ExpParam.ThreadId = GetCurrentThreadId();
        ExpParam.ExceptionPointers = pExceptionPointers;
        ExpParam.ClientPointers = TRUE;

        bMiniDumpSuccessful = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
            hDumpFile, MiniDumpWithHandleData, &ExpParam, NULL, NULL);
    }

    return bMiniDumpSuccessful;

}

LONG WINAPI TerminateUnhandledExceptionFilter(_EXCEPTION_POINTERS * ExceptionInfo)
{
    DWORD errorCode = ExceptionInfo->ExceptionRecord->ExceptionCode;
    std::wofstream wout(LOG_FILE_NAME, std::ios::app);
    wout << L"TerminateUnhandledExceptionFilter : an unknown exception occurred" << std::endl;
    BOOL res = GenerateDump(ExceptionInfo);
    if (res)
    {
        wout << L"TerminateUnhandledExceptionFilter mini user damp was created" << std::endl;
    }
    else
    {
        wout << L"TerminateUnhandledExceptionFilter mini user damp wasn't created" << std::endl;
    }
    SendToConsoleIf(std::cout, "Caught an error: ", static_cast<unsigned long>(errorCode));

    return EXCEPTION_EXECUTE_HANDLER;
}

void _cdecl TerminateStdUnhandledException()
{
    std::wofstream wout(LOG_FILE_NAME, std::ios::app);
    wout << L"TerminateStdUnhandledException : an unknown exception occurred" << std::endl;
    SendToConsoleIf(std::cout, "Caught an error: TerminateStdUnhandledException");
}