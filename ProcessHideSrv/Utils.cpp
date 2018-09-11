#include "Utils.h"

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