#pragma once
#include "stdafx.h"
#include "Common.h"
#include "RpcGuards.h"
#include "ExceptionService.h"

std::wstring GetFullCurrentProcessPath();
std::wstring GetFullCurrentProcessPathToFolder();
std::wstring GetCurrentProcessName();

std::string wstr2str(const std::wstring & str);
std::wstring str2wstr(const std::string & str);

BOOL GenerateDump(EXCEPTION_POINTERS* pExceptionPointers);
LONG WINAPI TerminateUnhandledExceptionFilter(_EXCEPTION_POINTERS * ExceptionInfo);
void _cdecl TerminateStdUnhandledException();

template<bool condition = true, typename Output, typename Arg, typename ... Args>
inline void SendToConsoleIf(Output & out, Arg && arg, Args &&... args)
{
    if (condition)
    {
        out << std::forward<Arg>(arg);
        using expander = int[];
        (void)expander {
            0, (void(out << std::forward<Args>(args)), 0)...
        };
        out << std::endl;
    }
}

#define CHK_RPC(status, message) if ((status) != RPC_S_OK) throw RpcServerError(message, (DWORD)status);
#define THROW_LAST_ERROR_IF(f, mess)          {if (f) throw ServiceError(mess); }

namespace RPC
{
    extern BindingVectorGuard g_bindingVector;
    extern std::unique_ptr<UUID_VECTOR> g_uuidVector;

    inline RPC_WSTR WstrToRpcWstr(const WCHAR* str)
    {
        return reinterpret_cast<RPC_WSTR>(const_cast<WCHAR*>(str));
    }

    DWORD HandleClientRpcException(DWORD exceptionCode);

    void ServerRegisterIf(
        RPC_IF_HANDLE   ifSpec,
        UUID*           mgrTypeUuid = NULL,
        RPC_MGR_EPV*    mgrEpv = NULL);

    void ServerUseProtseq(
        const WCHAR*    protSeq,
        unsigned int    maxCalls = RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
        void*           securityDescriptor = NULL);

    void ServerUseProtseqEp(
        const WCHAR*    protSeq,
        const WCHAR*    endpoint,
        unsigned int    maxCalls = RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
        void*           securityDescriptor = NULL);

    void RegisterEndpoint(
        RPC_IF_HANDLE   ifSpec,
        UUID_VECTOR*    uuidVector = NULL,
        const WCHAR*    annotation = NULL);

    void UnregisterEndpoint(
        RPC_IF_HANDLE      IfSpec);

    void BindingFromString(
        RPC_BINDING_HANDLE& handle,
        const WCHAR*        objUuid = NULL,
        const WCHAR*        protSeq = NULL,
        const WCHAR*        networkAddr = NULL,
        const WCHAR*        endPoint = NULL,
        const WCHAR*        options = NULL);

    void ServerListen(
        unsigned int    minCallThreads = 1,
        unsigned int    maxCalls = RPC_C_LISTEN_MAX_CALLS_DEFAULT,
        bool            dontWait = false);
    void Unregistred(RPC_IF_HANDLE ifspec, UUID * uid, unsigned int waitFoCall);

    void WaitServerListen();

    void StopServerListening(RPC_BINDING_HANDLE binding = NULL);
}