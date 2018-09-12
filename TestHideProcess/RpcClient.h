#pragma once
#include "RPCGuards.h"
#include "stdafx.h"

class RpcClient
{
public:
    static std::unique_ptr<RpcClient> & GetInstance();
    bool Install(const std::wstring & dllX86Path, const std::wstring & dllX64Path);
    bool Uninstall();
    bool SetProcessList(const std::vector<std::wstring> & processList);
    bool GetProcessList(std::vector<std::wstring> & processList);
    bool InjectDll();
private:
    RpcClient(const std::wstring & endpoint);
    RpcClient(const RpcClient &) = delete;
    RpcClient(const RpcClient &&) = delete;
    RpcClient & operator=(const RpcClient &) = delete;
    RpcClient & operator=(const RpcClient &&) = delete;
private:
    static std::unique_ptr<RpcClient> m_instance;
    RPC_BINDING_HANDLE m_bindingHandle;
    BindingHandleGuard m_bindingHandleGuard;
};

