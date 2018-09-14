// ProcessHideHooks.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "easyhook.h"
#include "InjectDll.h"
#include "TaskManagerDetector.h"

extern HMODULE hCurrentModule;
HHOOK g_hook = nullptr;

static
LRESULT
CALLBACK
HookProc(
    int    nCode,
    WPARAM wParam,
    LPARAM lParam
)
{
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

bool InstallHooks()
{
    if (!TaskManagerDetector::GetInstance()->InstallHooks())
    {
        return false;
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






