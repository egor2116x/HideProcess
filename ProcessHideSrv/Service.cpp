#include "Service.h"
#include "ExceptionService.h"
#include "Utils.h"
#include "LogWriter.h"
#include "RpcUtils.h"

SERVICE_STATUS        Service::m_ServiceStatus;
SERVICE_STATUS_HANDLE Service::m_ServiceStatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

std::unique_ptr<Service> Service::m_instance(nullptr);

std::unique_ptr<Service> & Service::GetInstance()
{
    if (m_instance.get() == nullptr)
    {
        m_instance.reset(new Service());
    }
    return m_instance;
}

VOID WINAPI Service::ServiceMain(DWORD argc, LPTSTR * argv)
{
    try
    {
        m_ServiceStatusHandle = ::RegisterServiceCtrlHandlerEx(LOG_SERVICE_NAME, ServiceCtrlHandlerEx, NULL);
        THROW_LAST_ERROR_IF(NULL == m_ServiceStatusHandle, "RegisterServiceCtrlHandlerEx failed");

        // set status of service to "start pending"
        m_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        m_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
        m_ServiceStatus.dwWaitHint = 0;
        m_ServiceStatus.dwCheckPoint = 0;
        m_ServiceStatus.dwWin32ExitCode = 0;
        m_ServiceStatus.dwServiceSpecificExitCode = 0;
        m_ServiceStatus.dwCheckPoint = 0;

        ::SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);

        m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
        m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE | SERVICE_ACCEPT_SHUTDOWN;

        m_ServiceStatus.dwWaitHint = 0;
        m_ServiceStatus.dwCheckPoint = 0;

        ::SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
        LogWriter::GetInstance()->Print(LOG_FATAL, L"start run\n", GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
        Service::GetInstance()->Run();

        WaitForSingleObject(Service::GetInstance()->m_StopEvent, INFINITE);

        ServiceCtrlHandler(SERVICE_CONTROL_STOP);
    }
    catch (DWORD dwError)
    {
        // set status of service to "stopped"
        m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        m_ServiceStatus.dwControlsAccepted = 0;
        m_ServiceStatus.dwWin32ExitCode = dwError;
        m_ServiceStatus.dwWaitHint = 0;
        m_ServiceStatus.dwCheckPoint = 0;

        ::SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
    }
}

unsigned int WINAPI Service::WorkerThread(PVOID context)
{
    Service * thisPtr = reinterpret_cast<Service*>(context);
    try
    {
        thisPtr->m_server.Listen();
        WaitForSingleObject(thisPtr->m_StopEvent, INFINITE);

        thisPtr->m_server.StopListening();
    }
    catch (const RpcServerError & e)
    {
        return 1;
    }

    return 0;
}

void Service::Run()
{
    LogWriter::GetInstance()->Print(LOG_FATAL, L"Service run\n", GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
    m_workerThreadFuture = std::async(std::launch::async, &Service::WorkerThread, this);
}

void Service::Stop()
{
    LogWriter::GetInstance()->Print(LOG_FATAL, L"Service stop\n", GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
    SetEvent(m_StopEvent);
    if (m_workerThreadFuture.valid())
    {
        m_workerThreadFuture.wait();
    }
}

VOID WINAPI Service::ServiceCtrlHandler(DWORD opcode)
{
    switch (opcode)
    {
    case SERVICE_CONTROL_STOP:
    {
        LogWriter::GetInstance()->Print(LOG_FATAL, L"service ServiceCtrlHandler SERVICE_CONTROL_STOP\n", GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
        // set status of service to "stopped"
        m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        m_ServiceStatus.dwWaitHint = 0;
        m_ServiceStatus.dwCheckPoint = 0;
        ::SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
        break;
    }

    case SERVICE_CONTROL_INTERROGATE:
    {
        ::SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
        break;
    }
    }
}

DWORD WINAPI Service::ServiceCtrlHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    switch (dwControl)
    {
    case SERVICE_CONTROL_STOP:
    {
        LogWriter::GetInstance()->Print(LOG_FATAL, L"service ServiceCtrlHandlerEx SERVICE_CONTROL_STOP\n", GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
        if (m_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
            break;
        Service::GetInstance()->Stop();

        // set status of service to "stop pending"
        m_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        m_ServiceStatus.dwWaitHint = 0;
        m_ServiceStatus.dwCheckPoint = 0;
        ::SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);

        break;
    }

    case SERVICE_CONTROL_PRESHUTDOWN:
    {
        break;
    }

    case SERVICE_CONTROL_SHUTDOWN:
    {
        break;
    }

    case SERVICE_CONTROL_INTERROGATE:
    {
        ::SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
        break;
    }
    case SERVICE_CONTROL_SESSIONCHANGE:
    {
        break;
    }
    case SERVICE_CONTROL_DEVICEEVENT:
    {
        break;
    }
    }
    return NO_ERROR;
}

Service::~Service()
{
    Stop();
    if (m_StopEvent)
    {
        CloseHandle(m_StopEvent);
    }

    m_server.Unregistred();
}

Service::Service()
{
    m_server.Init();

    m_StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_StopEvent == nullptr)
    {
        throw STATUS_INVALID_HANDLE;
    }
}
