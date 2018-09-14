#include "stdafx.h"
#include "RpcClient.h"
#include "Utils.h"
#include "SCmanagerWraper.h"

enum class COMMANDS {INSTALL, UNINSTALL, START, STOP, SET_PROCESS_LIST, GET_PROCESS_LIST, SHOW_PROCESS_LIST, INJECT_DLL, UNKNOWN};

COMMANDS ParseUserInput(const std::wstring & userInput)
{
    if (userInput.compare(L"install") == 0)
    {
        return COMMANDS::INSTALL;
    }
    else if (userInput.compare(L"uninstall") == 0)
    {
        return COMMANDS::UNINSTALL;
    }
    else if (userInput.compare(L"set") == 0)
    {
        return COMMANDS::SET_PROCESS_LIST;
    }
    else if (userInput.compare(L"get") == 0)
    {
        return COMMANDS::GET_PROCESS_LIST;
    }
    else if (userInput.compare(L"show") == 0)
    {
        return COMMANDS::SHOW_PROCESS_LIST;
    }
    else if (userInput.compare(L"inject") == 0)
    {
        return COMMANDS::INJECT_DLL;
    }
    else if (userInput.compare(L"start") == 0)
    {
        return COMMANDS::START;
    }
    else if (userInput.compare(L"stop") == 0)
    {
        return COMMANDS::STOP;
    }

    return COMMANDS::UNKNOWN;
}

int main()
{
    std::vector<std::wstring> setProcessList = {L"TestHideProcess.exe", L"calc.exe", L"notepad.exe"};
    std::vector<std::wstring> getProcessList;
    auto & client = RpcClient::GetInstance();
    SCManagerWrapper scManager;
    CServiceHandle service;

    std::wstring userInput;
    std::wcout << L">> ";
    while (std::getline(std::wcin, userInput))
    {
        if (userInput.empty())
        {
            std::wcout << L"Incorrect command. Try again" << std::endl;
            continue;
        }

        switch(ParseUserInput(userInput))
        {
        case COMMANDS::INSTALL: 
        {
            // dll install
            std::wstring x86_DLL_directory = GetFullCurrentProcessPathToFolder();
            x86_DLL_directory += L"\\hookDLL\\x86";
            std::wstring x64_DLL_directory = GetFullCurrentProcessPathToFolder();
            x64_DLL_directory += L"\\hookDLL\\x64";
            if (!client->Install(x86_DLL_directory, x64_DLL_directory))
            {
                std::wcout << L"Loading dlls failed" << std::endl;
            }
            std::wcout << L"Hook dlls successfully loaded" << std::endl;

            //service install
            service.m_h = ::OpenService(scManager.GetSCManagerHandle(), LOG_SERVICE_NAME, SERVICE_QUERY_STATUS);
            if (service.m_h != nullptr)
            {
                SendToConsoleIf<IF_DEBUG>(std::wcout, L"Service already exists");
                std::wcout << L">> ";
                userInput.clear();
                continue;
            }

            std::wstring fullpath(GetFullCurrentProcessPathToFolder());
            fullpath += L'\\';
            fullpath += LOG_SERVICE_BIN_NAME;
            scManager.CreateService(LOG_SERVICE_NAME,
                LOG_SERVICE_NAME,
                fullpath.c_str(),
                SERVICE_WIN32_OWN_PROCESS,
                SERVICE_DEMAND_START,
                true);
            SendToConsoleIf<IF_DEBUG>(std::wcout, L"Service was installed successfully");
        }
            break;
        case COMMANDS::START:
            scManager.StartService(LOG_SERVICE_NAME);
            break;
        case COMMANDS::STOP:
            scManager.StopService(LOG_SERVICE_NAME);
            break;
        case COMMANDS::UNINSTALL: 
            if (!client->Uninstall())
            {
                std::wcout << L"Uninstall dlls failed" << std::endl;
            }
            std::wcout << L"Hook dlls successfuly uninstaled" << std::endl;
            break;
        case COMMANDS::SET_PROCESS_LIST: 
            if (!client->SetProcessList(setProcessList))
            {
                std::wcout << L"Could not install process list" << std::endl;
            }
            std::wcout << L"The process list was successfully installed" << std::endl;
            break;
        case COMMANDS::GET_PROCESS_LIST: 
            if (!client->GetProcessList(getProcessList))
            {
                std::wcout << L"Failed to get list of processes" << std::endl;
            }
            std::wcout << L"Getting the list of processes was successful" << std::endl;
            break;       
        case COMMANDS::SHOW_PROCESS_LIST: 
            for (const auto proccessName : getProcessList)
            {
                std::wcout << L"Process name: " << proccessName << std::endl;
            }
            break;
        case COMMANDS::INJECT_DLL:
        if (!client->InjectDll())
        {
             std::wcout << L"Failed to inject dll" << std::endl;
        }
        std::wcout << L"Inject dll was successful" << std::endl;
        break;
        break;
        default:
            std::wcout << L"Incorrect command. Try again" << std::endl;
        }

        userInput.clear();
    }

    return 0;
}
