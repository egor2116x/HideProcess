#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include "Common.h"

// common 
std::wstring GetFullProcessPath();

// hook dlls management functions
BOOL InstallHookDlls(const std::wstring & dllSourceDirPathX86, const std::wstring & dllSourceDirPathX64);
BOOL UninstallHookDlls();

//process/service managament function list
bool SetHideProcessList(const std::vector<std::wstring> & processList);
bool GetHideProcessList(std::vector<std::wstring> & processList);