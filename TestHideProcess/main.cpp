#include "stdafx.h"
#include "RpcClient.h"
#include "LogWriter.h"
#include "Api.h"
#include "Utils.h"

enum class COMMANDS {INSTALL, INSTALL_SRV, UNINSTALL, START_SRV, STOP_SRV, SET_PROCESS_LIST, GET_PROCESS_LIST, SHOW_PROCESS_LIST, INJECT_DLL, UNKNOWN};

COMMANDS ParseUserInput(const std::wstring & userInput)
{
    if (userInput.compare(L"install srv") == 0)
    {
        return COMMANDS::INSTALL_SRV;
    }
    else if (userInput.compare(L"uninstall") == 0)
    {
        return COMMANDS::UNINSTALL;
    }
    else if (userInput.compare(L"install") == 0)
    {
        return COMMANDS::INSTALL;
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
    else if (userInput.compare(L"start srv") == 0)
    {
        return COMMANDS::START_SRV;
    }
    else if (userInput.compare(L"stop srv") == 0)
    {
        return COMMANDS::STOP_SRV;
    }

    return COMMANDS::UNKNOWN;
}

int wmain(int argc, const wchar_t ** argv)
{
        
    std::vector<std::wstring> setProcessList = {L"TestHideProcess.exe", L"calc.exe", L"notepad.exe"};
    std::vector<std::wstring> getProcessList;
    auto & client = RpcClient::GetInstance();

    std::wstring userInput;
    std::wcout << L">> ";
    try
    {
        while (std::getline(std::wcin, userInput))
        {
            if (userInput.empty())
            {
                std::wcout << L"Incorrect command. Try again" << std::endl;
                std::wcout << L">> ";
                continue;
            }

            switch (ParseUserInput(userInput))
            {
            case COMMANDS::INSTALL_SRV:
            {
                //service install
                if (!Api::installSrv())
                {
                    std::wcout << L"Service was installed unsuccessfully" << std::endl;
                    break;
                }
                std::wcout << L"Service was installed successfully" << std::endl;
                break;
            }
            case COMMANDS::INSTALL:
            {
                // dll install
                std::wstring x86_DLL_directory = GetFullCurrentProcessPathToFolder();
                x86_DLL_directory += L"\\hookDLL\\x86";
                std::wstring x64_DLL_directory = GetFullCurrentProcessPathToFolder();
                x64_DLL_directory += L"\\hookDLL\\x64";
                LogWriter::GetInstance()->Print(LOG_FATAL, L"Install hook dll", GetCurrentProcessId(), GetCurrentThreadId(), GetCurrentProcessName());
                if (!client->Install(x86_DLL_directory, x64_DLL_directory))
                {
                    std::wcout << L"Loading for service mode dlls failed" << std::endl;
                    break;
                }
                std::wcout << L"Hook dlls successfully loaded for service mode" << std::endl;
                if (!Api::Install(x86_DLL_directory, x64_DLL_directory))
                {
                    std::wcout << L"Loading for user mode dlls failed" << std::endl;
                    break;
                }
                std::wcout << L"Hook dlls successfully loaded for user mode" << std::endl;
            }
            break;
            case COMMANDS::START_SRV:
                Api::startSrv();
                std::wcout << L"Service was started successfully" << std::endl;
                break;
            case COMMANDS::STOP_SRV:
                Api::stopSrv();
                std::wcout << L"Service was stopped successfully" << std::endl;
                break;
            case COMMANDS::UNINSTALL:
                if (!client->Uninstall())
                {
                    std::wcout << L"Uninstall dlls failed" << std::endl;
                    break;
                }
                std::wcout << L"Hook dlls successfuly uninstalled" << std::endl;
                break;
            case COMMANDS::SET_PROCESS_LIST:
                if (!client->SetProcessList(setProcessList))
                {
                    std::wcout << L"Could not install process list" << std::endl;
                    break;
                }
                std::wcout << L"The process list was successfully installed" << std::endl;
                break;
            case COMMANDS::GET_PROCESS_LIST:
                if (!client->GetProcessList(getProcessList))
                {
                    std::wcout << L"Failed to get list of processes" << std::endl;
                    break;
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
                    std::wcout << L"Failed to service inject dll" << std::endl;
                    break;
                }
                std::wcout << L"Service inject dll was successful" << std::endl;

                break;
            default:
                std::wcout << L"Incorrect command. Try again" << std::endl;
            }

            userInput.clear();
            std::wcout << L">> ";
        }
    }
    catch (const std::runtime_error & e)
    {
        DWORD error = GetLastError();
        std::wcout << e.what() << std::endl;
    }

    return 0;
}
