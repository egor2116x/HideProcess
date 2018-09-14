// ProcessHideSrv.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Utils.h"
#include "LogWriter.h"
#include "Utils.h"
#include "Service.h"
#include "ExceptionService.h"


int wmain(int argc, TCHAR* argv[])
{
    //Intercept unhandled exceptions
    SetUnhandledExceptionFilter(TerminateUnhandledExceptionFilter);
    auto handle = std::set_unexpected(TerminateStdUnhandledException);
    UNREFERENCED_PARAMETER(handle);
    LogWriter::GetInstance()->Print(LOG_FATAL, L"wmain\n", GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
    SERVICE_TABLE_ENTRY dispatchTable[] = { { LOG_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)Service::ServiceMain },{ NULL, NULL } };
    try
    {
        if (!::StartServiceCtrlDispatcher(dispatchTable))
        {
                std::wstring mes(L"StartServiceCtrlDispatcher error : ");
                mes += std::to_wstring(static_cast<unsigned long>(GetLastError()));
                mes += L'\n';
                LogWriter::GetInstance()->Print(LOG_FATAL, mes.c_str(), GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
        }    
    }
    catch (const ServiceError & e)
    {
        std::wstring mes(L"ServiceError caught error : ");
        mes += std::to_wstring(static_cast<unsigned long>(e.GetError()));
        mes += ' ';
        mes += str2wstr(e.what());
        mes += L'\n';
        LogWriter::GetInstance()->Print(LOG_FATAL, mes.c_str(), GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
    }
    catch (const RpcServerError & e)
    {
        std::wstring mes(L"RpcServerError caught error : ");
        mes += std::to_wstring(static_cast<unsigned long>(e.GetError()));
        mes += ' ';
        mes += str2wstr(e.what());
        mes += L'\n';
        LogWriter::GetInstance()->Print(LOG_FATAL, mes.c_str(), GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
    }
    catch (const WriterLoggerError & e)
    {
        std::wstring mes(L"WriterLoggerError caught error : ");
        mes += std::to_wstring(static_cast<unsigned long>(e.GetError()));
        mes += ' ';
        mes += str2wstr(e.what());
        mes += L'\n';
        LogWriter::GetInstance()->Print(LOG_FATAL, mes.c_str(), GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
    }
    return 0;
}

