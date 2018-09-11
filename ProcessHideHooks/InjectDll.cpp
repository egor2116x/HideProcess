// ProcessHideHooks.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "easyhook.h"

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





