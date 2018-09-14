#include "RpcClient.h"
#include "Utils.h"
#include "RpcUtils.h"
#include "ExceptionService.h"
#include "RpcAllocator.h"
#include "RpcGuards.h"

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
    return false;
}

bool RpcClient::Uninstall()
{
    return false;
}

bool RpcClient::SetProcessList(const std::vector<std::wstring> & processList)
{
    return false;
}

bool RpcClient::GetProcessList(std::vector<std::wstring> & processList)
{
    return false;
}

bool RpcClient::InjectDll()
{
    return false;
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