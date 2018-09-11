#include <iostream>
#include <string>
#include <vector>

enum class COMMANDS {INSTALL, UNINSTALL, SET_PROCESS_LIST, GET_PROCESS_LIST, SHOW_PROCESS_LIST, INJECT_DLL, UNKNOWN};

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
    return COMMANDS::UNKNOWN;
}

int main()
{
    std::vector<std::wstring> setProcessList = {L"TestHideProcess.exe", L"calc.exe", L"notepad.exe"};
    std::vector<std::wstring> getProcessList;

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
            if (!Install())
            {
                std::wcout << L"Loading dlls failed" << std::endl;
            }
            std::wcout << L"Hook dlls successfully loaded" << std::endl;
            break;
        case COMMANDS::UNINSTALL: 
            if (!Uninstall())
            {
                std::wcout << L"Uninstall dlls failed" << std::endl;
            }
            std::wcout << L"Hook dlls successfuly uninstaled" << std::endl;
            break;
        case COMMANDS::SET_PROCESS_LIST: 
            if (!SetHideProcessList(setProcessList))
            {
                std::wcout << L"Could not install process list" << std::endl;
            }
            std::wcout << L"The process list was successfully installed" << std::endl;
            break;
        case COMMANDS::GET_PROCESS_LIST: 
            if (!GetHideProcessList(getProcessList))
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
        {
            bool(WINAPI * CurStart)() = nullptr;
#ifdef _M_X64
            CurStart = (bool(WINAPI *)())GetProcAddress(GetModuleHandle(X64HookDllName), "?Start@@YA_NXZ");
#else
            CurStart = (bool(WINAPI *)())GetProcAddress(GetModuleHandle(X86HookDllName), "?Start@@YA_NXZ");
#endif
            //
            // Inject support dll
            //
            if (CurStart != nullptr)
                CurStart();
        }
        break;
        default:
            std::wcout << L"Incorrect command. Try again" << std::endl;
        }

        userInput.clear();
    }

    return 0;
}
