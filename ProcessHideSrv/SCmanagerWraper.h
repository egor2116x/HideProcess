#pragma once
#include "stdafx.h"

class CServiceHandle
{
public:
    CServiceHandle() : m_h(nullptr) {}
    CServiceHandle(CServiceHandle& h);
    explicit CServiceHandle(SC_HANDLE h) : m_h(h) {}
    virtual ~CServiceHandle();

    CServiceHandle& operator=(CServiceHandle& h);
    operator SC_HANDLE() const;

    // Attach to an existing handle (takes ownership).
    void Attach(SC_HANDLE h);
    // Detach the handle from the object (releases ownership).
    SC_HANDLE Detach();
    // Close the handle.
    void Close();

public:
    SC_HANDLE m_h;
};

class SCManagerWrapper
{
public:
    SCManagerWrapper();
    void CreateService(LPCTSTR lpServiceName,
        LPCTSTR lpDisplayName,
        LPCTSTR lpBinaryPathName,
        DWORD   dwServiceType,
        DWORD   dwStartType,
        bool    bStart,
        LPCTSTR lpDependencies = NULL);

    void DeleteService(LPCTSTR lpServiceName, BOOL fStop);
    void StartService(LPCTSTR lpServiceName);
    void StopService(LPCTSTR lpServiceName);
    bool IsServiceRunning(LPCTSTR lpServiceName);
    bool IsServiceStopPending(LPCTSTR lpServiceName);
    bool IsServiceExsist(LPCTSTR lpServiceName);
    CServiceHandle & GetSCManagerHandle() { return m_ScHandle; }
private:
    bool SetRecoveryServiceSettingsDefault(CServiceHandle & service);
private:
    CServiceHandle m_ScHandle;
};

