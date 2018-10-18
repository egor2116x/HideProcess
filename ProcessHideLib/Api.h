#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include "Common.h"

namespace Api
{
    // hook dlls management functions
    BOOL Install(const std::wstring & dllSourceDirPathX86, const std::wstring & dllSourceDirPathX64);
    BOOL Uninstall();
    BOOL Inject();
    UINT InjectIntoUserSession(LPVOID pParam);

    //service managament function list
    //gen exception std::runtime_error
    bool installSrv();
    void startSrv();
    void stopSrv();

    // manage hide process list
    bool SetHideProcessList(const std::vector<std::wstring> & processList);
    bool GetHideProcessList(std::vector<std::wstring> & processList);
}