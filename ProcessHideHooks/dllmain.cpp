// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ProcessHideHooks.h"
#include "Utils.h"

HMODULE hCurrentModule = nullptr;
HMODULE hNtDll = nullptr;

bool IsProcessNeedHide(const std::wstring & processName)
{
    std::vector<std::wstring> processListArr;
    bool result = LoadMultiString(CONFIG_REGISTRY_PROCESS_HIDE, processListArr);

    if (!result || processListArr.empty())
    {
        return result;
    }

    for (const auto & pn : processListArr)
    {
        if (toLowerW(pn).compare(toLowerW(processName)) == 0)
        {
            return true;
        }
    }

    return false;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:     
        if ((hNtDll = LoadLibraryA("ntdll.dll")) == NULL)
        {
            return FALSE;
        }

         hCurrentModule = hModule;        
         if (IsProcessNeedHide(GetCurrentProcessName()))
         {
             if (!InstallHooks())
             {
                 // somethings wrong            
             }
         }
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

