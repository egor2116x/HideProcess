#include "RpcClient.h"
#include "Utils.h"
#include "RpcUtils.h"
#include "ExceptionService.h"
#include "RpcAllocator.h"
#include "RpcGuards.h"
#include "Service_h.h"

std::unique_ptr<RpcClient> RpcClient::m_instance(nullptr);

std::unique_ptr<RpcClient>& RpcClient::GetInstance()
{
    if (m_instance.get() != nullptr)
    {
        return m_instance;
    }

    m_instance.reset(new RpcClient(L""));
    return m_instance;
}

bool RpcClient::Install(const std::wstring & dllX86Path, const std::wstring & dllX64Path)
{
    return ::Install(m_bindingHandle, dllX86Path.c_str(), dllX64Path.c_str());
}

bool RpcClient::Uninstall()
{
    return ::Uninstall(m_bindingHandle);
}

bool RpcClient::SetProcessList(const std::vector<std::wstring> & processList)
{
    std::wstring tmp;
    for (const auto & processName : processList)
    {
        tmp += processName;
        tmp += CONFIG_REGISTRY_PROCESS_HIDE_DELIMITER;
    }

    return ::SetProcessList(m_bindingHandle, tmp.c_str());
}

bool RpcClient::GetProcessList(std::vector<std::wstring> & processList)
{
    wchar_t * tmp;
    long size = 0;

    BOOL result = ::GetProcessList(m_bindingHandle, tmp, &size);
    if (!result)
    {
        return false;
    }

    std::wstring list(tmp, size);
    if (list.empty())
    {
        return false;
    }

    size_t idx = 0;
    std::wstring::size_type pos = list.find(CONFIG_REGISTRY_PROCESS_HIDE_DELIMITER, idx);
    while (pos != std::wstring::npos)
    {
        processList.push_back(list.substr(idx, pos));
        idx = pos + 1;
        pos = list.find(CONFIG_REGISTRY_PROCESS_HIDE_DELIMITER, idx);
    }
    
    return true;
}

bool RpcClient::InjectDll()
{
    return ::InjectDll(m_bindingHandle);
}

RpcClient::RpcClient(const std::wstring & endpoint) : m_bindingHandle(NULL)
{
    try
    {
        RPC::BindingFromString(m_bindingHandle, NULL, L"ncalrpc", NULL, endpoint.c_str());
        m_bindingHandleGuard.reset(&m_bindingHandle);
    }
    catch (const RpcServerError & e)
    {
        unsigned long err = e.GetError();
        std::cerr << "BindingFromString was failed " << err << std::endl;
    }
}