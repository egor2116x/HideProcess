#include "RpcUtils.h"

#define STATUS_INSTRUCTION_MISALIGNMENT  ((DWORD)0xC00000AAL)
#define STATUS_POSSIBLE_DEADLOCK         ((DWORD)0xC0000194L)
#define STATUS_HANDLE_NOT_CLOSABLE       ((DWORD)0xC0000235L)
#ifndef STATUS_STACK_BUFFER_OVERRUN
#define STATUS_STACK_BUFFER_OVERRUN      ((DWORD)0xC0000409L)
#endif
#ifndef STATUS_ASSERTION_FAILURE
#define STATUS_ASSERTION_FAILURE         ((DWORD)0xC0000420L)    
#endif

namespace RPC
{
    BindingVectorGuard g_bindingVector(nullptr);
    std::unique_ptr<UUID_VECTOR> g_uuidVector(nullptr);

    DWORD HandleClientRpcException(DWORD exceptionCode)
    {
        return (exceptionCode == STATUS_ACCESS_VIOLATION ||
            exceptionCode == STATUS_POSSIBLE_DEADLOCK ||
            exceptionCode == STATUS_INSTRUCTION_MISALIGNMENT ||
            exceptionCode == STATUS_DATATYPE_MISALIGNMENT ||
            exceptionCode == STATUS_PRIVILEGED_INSTRUCTION ||
            exceptionCode == STATUS_ILLEGAL_INSTRUCTION ||
            exceptionCode == STATUS_BREAKPOINT ||
            exceptionCode == STATUS_STACK_OVERFLOW ||
            exceptionCode == STATUS_HANDLE_NOT_CLOSABLE ||
            exceptionCode == STATUS_IN_PAGE_ERROR ||
            exceptionCode == STATUS_ASSERTION_FAILURE ||
            exceptionCode == STATUS_STACK_BUFFER_OVERRUN ||
            exceptionCode == STATUS_GUARD_PAGE_VIOLATION)
            ? EXCEPTION_CONTINUE_SEARCH : EXCEPTION_EXECUTE_HANDLER;
    }

    void ServerRegisterIf(RPC_IF_HANDLE ifSpec, UUID* mgrTypeUuid, RPC_MGR_EPV* mgrEpv)
    {
        RPC_STATUS status = ::RpcServerRegisterIf(ifSpec, mgrTypeUuid, mgrEpv);
        CHK_RPC(status, "RpcServerRegisterIf failed");
    }

    void ServerUseProtseq(const WCHAR* protSeq, unsigned int maxCalls, void* securityDescriptor)
    {
        RPC_STATUS status = ::RpcServerUseProtseqW(WstrToRpcWstr(protSeq), maxCalls, securityDescriptor);
        CHK_RPC(status, "RpcServerUseProtseq failed");
    }

    void ServerUseProtseqEp(const WCHAR* protSeq, const WCHAR* endpoint, unsigned int maxCalls, void* securityDescriptor)
    {
        RPC_STATUS status = ::RpcServerUseProtseqEpW(WstrToRpcWstr(protSeq), maxCalls, WstrToRpcWstr(endpoint), securityDescriptor);
        CHK_RPC(status, "RpcServerUseProtseqEp failed");
    }

    void RegisterEndpoint(RPC_IF_HANDLE ifSpec, UUID_VECTOR* uuidVector, const WCHAR* annotation)
    {
        RPC_BINDING_VECTOR* bindingVector = NULL;

        RPC_STATUS status = ::RpcServerInqBindings(&bindingVector);
        CHK_RPC(status, "RpcServerInqBindings failed");

        g_bindingVector.reset(bindingVector);

        status = ::RpcEpRegisterW(ifSpec, bindingVector, uuidVector, WstrToRpcWstr(annotation));
        g_uuidVector.reset(uuidVector);
        CHK_RPC(status, "RpcEpRegister failed");
    }

    void UnregisterEndpoint(RPC_IF_HANDLE IfSpec)
    {
        RPC_STATUS status = ::RpcEpUnregister(IfSpec, g_bindingVector.get(), g_uuidVector.get());
        CHK_RPC(status, "RpcEpUnregister failed");
        g_bindingVector.reset();
        g_uuidVector.reset();
    }

    void BindingFromString(
        RPC_BINDING_HANDLE& handle,
        const WCHAR*        objUuid,
        const WCHAR*        protSeq,
        const WCHAR*        networkAddr,
        const WCHAR*        endPoint,
        const WCHAR*        options)
    {
        RPC_WSTR stringBinding = NULL;

        RPC_STATUS status = ::RpcStringBindingComposeW(WstrToRpcWstr(objUuid),
            WstrToRpcWstr(protSeq),
            WstrToRpcWstr(networkAddr),
            WstrToRpcWstr(endPoint),
            WstrToRpcWstr(options),
            &stringBinding);
        CHK_RPC(status, "RpcStringBindingCompose failed");

        StringGuard<RPC_WSTR> guard(stringBinding);

        status = ::RpcBindingFromStringBindingW(stringBinding, &handle);
        CHK_RPC(status, "RpcBindingFromStringBinding failed");
    }

    void ServerListen(
        unsigned int    minCallThreads,
        unsigned int    maxCalls,
        bool            dontWait)
    {
        RPC_STATUS status = ::RpcServerListen(minCallThreads, maxCalls, (unsigned)dontWait);
        CHK_RPC(status, "RpcServerListen failed");
    }

    void Unregistred(RPC_IF_HANDLE ifspec, UUID * uid, unsigned int waitFoCall)
    {
        RPC_STATUS status = ::RpcServerUnregisterIf(ifspec, uid, waitFoCall);
        CHK_RPC(status, "RpcServerUnregisterIf failed");
    }

    void WaitServerListen()
    {
        RPC_STATUS status = ::RpcMgmtWaitServerListen();
        CHK_RPC(status, "RpcMgmtWaitServerListen failed");
    }

    void StopServerListening(RPC_BINDING_HANDLE binding)
    {
        RPC_STATUS status = ::RpcMgmtStopServerListening(binding);
        CHK_RPC(status, "RpcMgmtStopServerListening failed");
    }
}