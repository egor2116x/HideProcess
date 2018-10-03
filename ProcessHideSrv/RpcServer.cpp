#include "RpcServer.h"
#include "Utils.h"
#include "Service_h.h"
#include "Api.h"
#include "LogWriter.h"
#include "RpcAllocator.h"
#include "RpcUtils.h"


void RpcServer::Init()
{
    RPC::ServerRegisterIf(IHideProcess_v1_0_s_ifspec);
    m_ifHandleGuard.reset(IHideProcess_v1_0_s_ifspec);
    RPC::ServerUseProtseq(L"ncalrpc");
    //logger::ServerUseProtseqEp(L"ncalrpc", PRINT_LOG_END_POINT);
    RPC::RegisterEndpoint(IHideProcess_v1_0_s_ifspec);
}

void RpcServer::Listen()
{
    RPC::ServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, true);
}

void RpcServer::StopListening()
{
    RPC::StopServerListening();
}

void RpcServer::Unregistred()
{
    if (m_ifHandleGuard.get() != NULL)
    {
        RPC::UnregisterEndpoint(m_ifHandleGuard.get());
        RPC::Unregistred(m_ifHandleGuard.get(), NULL, TRUE);
    }
}

void RpcServer::Wait()
{
    RPC::WaitServerListen();
}


// IDL interface IHideProcess
extern "C"
boolean Install(handle_t h1, const wchar_t * dllX86Path, const wchar_t * dllX64Path)
{
    LogWriter::GetInstance()->Print(LOG_FATAL, L"Install hook dll", GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());  
    return Api::Install(dllX86Path, dllX64Path);
}

extern "C"
boolean Uninstall(handle_t h1)
{
    LogWriter::GetInstance()->Print(LOG_FATAL, L"Uninstall hook dll", GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
    return Api::Uninstall();
}

extern "C"
boolean SetProcessList(handle_t h1, const wchar_t * processList)
{
    LogWriter::GetInstance()->Print(LOG_FATAL, L"Set process list", GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
    std::wstring tmp(processList);
    std::vector<std::wstring> processListArr;
    std::wstring::size_type pos = tmp.find(CONFIG_REGISTRY_PROCESS_HIDE_DELIMITER);
    size_t idx = 0;
    while(pos != std::wstring::npos)
    {
        processListArr.push_back(tmp.substr(idx, pos));
        idx = pos + 1;
        pos = tmp.find(CONFIG_REGISTRY_PROCESS_HIDE_DELIMITER, idx);
    }

    if (processListArr.empty())
    {
        return false;
    }
    
    return Api::SetHideProcessList(processListArr);
}

extern "C"
boolean GetProcessList(handle_t h1, wchar_t * processList, long * size)
{
    LogWriter::GetInstance()->Print(LOG_FATAL, L"Get process list", GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
    std::wstring tmp;

    std::vector<std::wstring> processListArr;
    BOOL result = Api::GetHideProcessList(processListArr);

    if (!result || processListArr.empty())
    {
        return result;
    }

    for (const auto & str : processListArr)
    {
        tmp += str;
        tmp += CONFIG_REGISTRY_PROCESS_HIDE_DELIMITER;
    }

    processList = new wchar_t[tmp.size()];
    *size = tmp.size() / sizeof(wchar_t);
    memcpy(processList, &tmp[0], *size);

    return result;
}

extern "C"
boolean InjectDll(handle_t h1)
{
    LogWriter::GetInstance()->Print(LOG_FATAL, L"Inject dll", GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
    return Api::Inject();
}
