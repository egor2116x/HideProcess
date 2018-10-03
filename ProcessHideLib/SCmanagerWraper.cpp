#include "SCmanagerWraper.h"

#define THROW_LAST_ERROR_IF(f, mess)          {if (f) throw std::runtime_error(mess); }

CServiceHandle::CServiceHandle(CServiceHandle& h) : m_h(nullptr)
{
    Attach(h.Detach());
}

CServiceHandle::~CServiceHandle()
{
    Close();
}

inline CServiceHandle& CServiceHandle::operator=(CServiceHandle& h)
{
    if (this != &h)
    {
        Close();
        Attach(h.Detach());
    }

    return (*this);
}

inline CServiceHandle::operator SC_HANDLE() const
{
    return m_h;
}

inline void CServiceHandle::Attach(SC_HANDLE h)
{
    _ASSERTE(nullptr == m_h);
    m_h = h;  // Take ownership
}

inline SC_HANDLE CServiceHandle::Detach()
{
    SC_HANDLE h = m_h;  // Release ownership

    m_h = nullptr;

    return h;
}

inline void CServiceHandle::Close()
{
    if (nullptr != m_h)
    {
        ::CloseServiceHandle(m_h);
        m_h = nullptr;
    }
}

SCManagerWrapper::SCManagerWrapper()
{
    m_ScHandle.m_h = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    THROW_LAST_ERROR_IF(NULL == m_ScHandle.m_h, "OpenSCManager failed");
}

bool SCManagerWrapper::SetRecoveryServiceSettingsDefault(CServiceHandle & service)
{
    // Recovery setting
    SERVICE_FAILURE_ACTIONS servFailActions;
    SC_ACTION failActions[3];

    failActions[0].Type = SC_ACTION_RESTART;    // Failure action: Restart Service
    failActions[0].Delay = 60000;               // number of seconds to wait before performing failure action, in milliseconds
    failActions[1].Type = SC_ACTION_RESTART;
    failActions[1].Delay = 60000;
    failActions[2].Type = SC_ACTION_RESTART;
    failActions[2].Delay = 60000;

    servFailActions.dwResetPeriod = 86400;      // Reset Failures Counter, in Seconds = 1day
    servFailActions.lpCommand = NULL;           // Command to perform due to service failure, not used
    servFailActions.lpRebootMsg = NULL;         // Message during rebooting computer due to service failure, not used
    servFailActions.cActions = 3;               // Number of failure action to manage
    servFailActions.lpsaActions = failActions;

    return ChangeServiceConfig2(service.m_h, SERVICE_CONFIG_FAILURE_ACTIONS, &servFailActions) != 0; //Apply above settings
}

void SCManagerWrapper::CreateService(LPCTSTR lpServiceName,
    LPCTSTR lpDisplsyName,
    LPCTSTR lpBinaryPathName,
    DWORD   dwServiceType,
    DWORD   dwStartType,
    bool    bStart,
    LPCTSTR lpDependencies
)
{
    CServiceHandle service;
    service.m_h = ::CreateService(
        m_ScHandle,
        lpServiceName,
        lpDisplsyName,
        SERVICE_START | SERVICE_CHANGE_CONFIG,
        dwServiceType,
        dwStartType,
        SERVICE_ERROR_NORMAL,
        lpBinaryPathName,
        NULL,
        NULL,
        lpDependencies,
        NULL,
        NULL
    );
    THROW_LAST_ERROR_IF(NULL == service.m_h, "CreateService failed");

    bool result = SetRecoveryServiceSettingsDefault(service);
    UNREFERENCED_PARAMETER(result);
}

void SCManagerWrapper::DeleteService(LPCTSTR lpServiceName, BOOL fStop)
{
    CServiceHandle service;
    service.m_h = ::OpenService(m_ScHandle, lpServiceName, DELETE | SERVICE_STOP);
    THROW_LAST_ERROR_IF(NULL == service.m_h, "OpenService failed");
    if (fStop)
    {
        SERVICE_STATUS ServiceStatus;
        ::ControlService(service, SERVICE_CONTROL_STOP, &ServiceStatus);
    }

    THROW_LAST_ERROR_IF(!::DeleteService(service), "DeleteService failed");
}

void SCManagerWrapper::StartService(LPCTSTR lpServiceName)
{
    CServiceHandle service;
    service.m_h = ::OpenService(m_ScHandle, lpServiceName, SERVICE_START);
    THROW_LAST_ERROR_IF(NULL == service.m_h, "OpenService failed");

    THROW_LAST_ERROR_IF(!::StartService(service, 0, NULL), "StartService failed");
}

void SCManagerWrapper::StopService(LPCTSTR lpServiceName)
{
    CServiceHandle service;
    service.m_h = ::OpenService(m_ScHandle, lpServiceName, DELETE | SERVICE_STOP);
    THROW_LAST_ERROR_IF(NULL == service.m_h, "OpenService failed");

    SERVICE_STATUS ServiceStatus;
    THROW_LAST_ERROR_IF(!::ControlService(service, SERVICE_CONTROL_STOP, &ServiceStatus), "ControlService failed");
}

bool SCManagerWrapper::IsServiceRunning(LPCTSTR lpServiceName)
{
    CServiceHandle service;
    service.m_h = ::OpenService(m_ScHandle, lpServiceName, SERVICE_QUERY_STATUS);
    THROW_LAST_ERROR_IF(NULL == service.m_h, "OpenService failed");

    SERVICE_STATUS_PROCESS ServiceStatus;
    DWORD BytesNeeded;

    THROW_LAST_ERROR_IF(!::QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&ServiceStatus, sizeof(ServiceStatus), &BytesNeeded), "QueryServiceStatusEx failed");

    return ServiceStatus.dwCurrentState == SERVICE_RUNNING;
}

bool SCManagerWrapper::IsServiceStopPending(LPCTSTR lpServiceName)
{
    CServiceHandle service;
    service.m_h = ::OpenService(m_ScHandle, lpServiceName, SERVICE_QUERY_STATUS);
    THROW_LAST_ERROR_IF(NULL == service.m_h, "OpenService failed");

    SERVICE_STATUS_PROCESS ServiceStatus;
    DWORD BytesNeeded;

    THROW_LAST_ERROR_IF(!::QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&ServiceStatus, sizeof(ServiceStatus), &BytesNeeded), "QueryServiceStatusEx failed");

    return ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING;
}

bool SCManagerWrapper::IsServiceExsist(LPCTSTR lpServiceName)
{
    CServiceHandle service;
    service.m_h = OpenService(m_ScHandle,
        lpServiceName,
        SERVICE_ALL_ACCESS);
    THROW_LAST_ERROR_IF(NULL == service.m_h, "OpenService failed");

    return true;
}