#include <QApplication>
#include <QtWidgets>

#include <wtsapi32.h>
#include <tchar.h>
#include <WinBase.h>

#include "servicebase.h"
#include "frengine.h"
#include "surveillanceservice.h"
#include "base.h"

#pragma comment(lib, "Advapi32")

UINT gSpawnTimerID = 0;
DWORD gSpwanSeesionID = 0;
int gFirstFlag = 0;

// Service data
SERVICE_STATUS          gServiceStatus;
SERVICE_STATUS_HANDLE   gServiceStatusHandle;
WCHAR                   gServiceBinPath[MAX_PATH];

// Synchronisation events
HANDLE ForceExit(NULL);
HANDLE ForceExitAck(NULL);
HANDLE ProviderStart(NULL);

#define BUF_SIZE 256
TCHAR szName[]=L"Global\\VMSService_video_Map";
TCHAR szMsg[]=L"Message from first process.";

// Critical section handle to ensure we only launch one provider
CRITICAL_SECTION        gCriticalSection;

// Process ID of the provider
DWORD                   gProviderID(0);
// Write results to log file
int LOG(char* str)
{
    FILE* log;
    SYSTEMTIME lt;
    if (fopen_s(&log, SERVICE_LOGFILE, "a+") != 0)
        return -1;
    GetLocalTime(&lt);
    fprintf(log, "%02d:%02d:%02d %s\n", lt.wHour, lt.wMinute, lt.wSecond, str);
    fclose(log);
    return 0;
}


// Callback function to execute when the service requests that the provider
// exits.
VOID CALLBACK ForceExitCallback(PVOID, BOOLEAN)
{
    HANDLE hForceExitAck;
    if(qApp)
        qApp->quit();

    // Ack the request
    hForceExitAck = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"Global\\EasenVMS_video_ForceExitAck");
    if (hForceExitAck)
    {
        SetEvent(hForceExitAck);
        CloseHandle(hForceExitAck);
    }
}

// Main provider
int ProviderMain(QApplication* app)
{
    HANDLE hMapFile = CreateFileMapping(
                    INVALID_HANDLE_VALUE,    // use paging file
                    NULL,                    // default security
                    PAGE_READWRITE,          // read/write access
                    0,                       // maximum object size (high-order DWORD)
                    BUF_SIZE,                // maximum object size (low-order DWORD)
                    szName);                 // name of mapping object

    bool ok = true;

    HANDLE hProviderStart = NULL;
    HANDLE hForceExit = NULL;
    HANDLE hThread = NULL;

    // Signal that ProviderMain is running
    if (ok && (hProviderStart = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"Global\\EasenVMS_video_ProviderStart")))
    {
        SetEvent(hProviderStart);
    }
    else
    {
        ok = false;
    }

    // Register for force exit notification from service
    if (ok && (hForceExit = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"Global\\EasenVMS_video_ForceExit")))
    {
        RegisterWaitForSingleObject(&hThread, hForceExit, ForceExitCallback, 0, INFINITE, WT_EXECUTEONLYONCE);
    }
    else
    {
        ok = false;
    }

    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);

#ifdef _ACTIVATION_
    int ret = SetActivation(setting.value("activation key").toString().toUtf8().data());
    if(ret == 0)
#endif
    {
        SurveillanceService* surveillanceService = new SurveillanceService;
        surveillanceService->create();
        surveillanceService->start();

        app->exec();

        surveillanceService->release();

        delete surveillanceService;
    }

    // Cleanup
    CloseHandle(hProviderStart);
    CloseHandle(hForceExit);
    UnregisterWait(hThread);

    CloseHandle(hMapFile);

    return ok;
}

void TerminateProvider()
{
    HANDLE h;

    // Check whether provider is active
    if ((h = OpenProcess(PROCESS_TERMINATE, FALSE, gProviderID)) != 0)
    {
        ResetEvent(ForceExitAck);
        // Ask provider to terminate and wait for response
        SignalObjectAndWait(ForceExit, ForceExitAck, 5000, FALSE);
        ResetEvent(ForceExit);
    }
    gProviderID = 0;
}

// Spawn the provider process in the specified session.
bool SpawnProvider(DWORD sessionId)
{
    bool ok = true;
    bool res = false;

    wchar_t exe[MAX_PATH * 2] = L"";
    wchar_t cmdLine[MAX_PATH * 2] = L"";
    HANDLE svctoken = NULL;
    HANDLE provtoken = NULL;

    // Close any active provider
    TerminateProvider();
    ResetEvent(ProviderStart);

    // Build the path to the executable and arguments
    wcscpy(exe, gServiceBinPath);
    wcscpy(cmdLine, exe);
    wcscat(cmdLine, L" ");
    wcscat(cmdLine, SERVICE_ARG_SPAWN_PROVIDER);

    do
    {
        // Create token for new process by duplicating our own token
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &svctoken) == 0)
        {
            break;
        }
        DuplicateTokenEx(svctoken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &provtoken);
        CloseHandle(svctoken);
        svctoken = NULL;

        // Move the new token to the target session
        if (!SetTokenInformation(provtoken, TokenSessionId, &sessionId, sizeof(DWORD)))
        {
            break;
        }

        // Set up the new process information
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi;
        const wchar_t *dsktp = L"WinSta0\\Default";

        si.cb = sizeof(si);
        si.lpDesktop = (wchar_t *)dsktp;

        if (!CreateProcessAsUser(provtoken, exe, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            break;
        }

        // Save Process ID
        gProviderID = pi.dwProcessId;

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        res = true;
    } while (!res);

    if (provtoken)
    {
        CloseHandle(provtoken);
    }
    if (res)
    {
        // Wait till provider starts up or timeout
        WaitForSingleObject(ProviderStart, 5000);
    }
    return res;
}



// The ServiceHandlerEx function handles events from the Service Control Manager.
DWORD WINAPI ServiceHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    DWORD dwReturn = NO_ERROR;
    PWTSSESSION_NOTIFICATION pSessionNotify = NULL;

    switch (dwControl)
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        // Shutdown the provider
        EnterCriticalSection(&gCriticalSection);
        TerminateProvider();
        LeaveCriticalSection(&gCriticalSection);
        // Stop the service
        gServiceStatus.dwWin32ExitCode = 0;
        gServiceStatus.dwCurrentState = SERVICE_STOPPED;
        // Cleanup
        CloseHandle(ForceExit);
        CloseHandle(ForceExitAck);
        CloseHandle(ProviderStart);
        DeleteCriticalSection(&gCriticalSection);
        break;

    case SERVICE_CONTROL_SESSIONCHANGE:
        if (lpEventData != NULL && dwEventType == WTS_CONSOLE_CONNECT)
        {
            // Spawn provider in new session
            pSessionNotify = (PWTSSESSION_NOTIFICATION)lpEventData;
            EnterCriticalSection(&gCriticalSection);
            SpawnProvider(pSessionNotify->dwSessionId);
            gSpwanSeesionID = pSessionNotify->dwSessionId;
            LeaveCriticalSection(&gCriticalSection);
        }
        break;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
        break;
    }

    // Always report current status
    SetServiceStatus(gServiceStatusHandle, &gServiceStatus);
    return dwReturn;
}



// When the service is started, ServiceMain is called to set up the event
// handler and configure the service.
VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
    bool ok = true;

    // Configure the service
    gServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    gServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_SESSIONCHANGE;
    gServiceStatus.dwWin32ExitCode = 0;
    gServiceStatus.dwServiceSpecificExitCode = 0;
    gServiceStatus.dwCheckPoint = 0;
    gServiceStatus.dwWaitHint = 0;

    // Register the service
    if (ok && (gServiceStatusHandle = RegisterServiceCtrlHandlerEx(SERVICE_NAME, (LPHANDLER_FUNCTION_EX)ServiceHandlerEx, NULL)) != 0)
    {
        gServiceStatus.dwCurrentState = SERVICE_START_PENDING;
        SetServiceStatus(gServiceStatusHandle, &gServiceStatus);
    }
    else
    {
        // Registering Control Handler failed
        ok = false;
    }

    if (ok)
    {
        // Report the running status to SCM.
        gServiceStatus.dwCurrentState = SERVICE_RUNNING;
        SetServiceStatus(gServiceStatusHandle, &gServiceStatus);
    }

    if (ok)
    {
        // Create events to communicate with the provider when it is started
        ForceExit = CreateEvent(NULL, FALSE, FALSE, L"Global\\EasenVMS_video_ForceExit");
        ForceExitAck = CreateEvent(NULL, FALSE, FALSE, L"Global\\EasenVMS_video_ForceExitAck");
        ProviderStart = CreateEvent(NULL, FALSE, FALSE, L"Global\\EasenVMS_video_ProviderStart");
    }

    // Start up provider if a session is already active
    // Required when service is started up manually (and thus does not get
    // CONSOLE_CONNECT notification). This will be the case when there is an
    // active session running.
    if (ok)
    {
        DWORD sessions = 0;
        WTS_SESSION_INFO *si;
        if (WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &si, &sessions))
        {
            // Create the provider in the active session only.
            for (DWORD i = 0; i < sessions; i++)
            {
                if (si[i].State == WTSActive)
                {
                    SpawnProvider(si[i].SessionId);                    
                    gSpwanSeesionID = si[i].SessionId;
                    break;
                }
            }
            WTSFreeMemory(si);
        }
    }

    while(gServiceStatus.dwCurrentState == SERVICE_RUNNING)
    {
        HANDLE hMapFile = OpenFileMapping(
                           FILE_MAP_ALL_ACCESS,   // read/write access
                           FALSE,                 // do not inherit the name
                           szName);

        if(hMapFile != NULL && gFirstFlag == 0)
            gFirstFlag = 1;

        if(hMapFile == NULL && gSpwanSeesionID != 0 && gFirstFlag == 1)
            SpawnProvider(gSpwanSeesionID);

        CloseHandle(hMapFile);

        Sleep(500);
    }
}


// Use the Service Control Manager to install the service
int InstallService()
{
    bool ok = true;

    SC_HANDLE SCM = NULL;
    SC_HANDLE service = NULL;
    DWORD dwReturn = ERROR_SUCCESS;

    // Get a handle to the Service Control Manager
    if (ok && (SCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) == NULL)
    {
        dwReturn = ERROR_FAILED_SERVICE_CONTROLLER_CONNECT;
        ok = false;
    }

    for (int retries = 40; ok && (!service) && retries--; Sleep(100))
    {
        // Try to create the service
        if ((service = CreateService(SCM, SERVICE_NAME, SERVICE_DESC, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START,
            SERVICE_ERROR_NORMAL, gServiceBinPath, L"Video\0\0", NULL, L"nvlddmkm\0\0", NULL, NULL)) == NULL)
        {

            // If creating the service failed because it is marked for delete
            // then we will retry, otherwise we return the error
            dwReturn = GetLastError();
            if (dwReturn != ERROR_SERVICE_MARKED_FOR_DELETE)
            {
                ok = false;
            }
        }
    }


    // Cleanup
    if (service)
        CloseServiceHandle(service);
    if (SCM)
        CloseServiceHandle(SCM);
    return ok ? NO_ERROR : dwReturn;
}

// Use the Service Control Manager to uninstall the service
int UninstallService()
{
    bool ok = true;

    SC_HANDLE SCM = NULL;
    SC_HANDLE service = NULL;
    DWORD dwReturn = ERROR_SUCCESS;
    SERVICE_STATUS status;

    // Get a handle to the Service Control Manager
    if (ok && (SCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) == NULL)
    {
        dwReturn = ERROR_FAILED_SERVICE_CONTROLLER_CONNECT;
        ok = false;
    }

    // Get a handle to the service
    if ((service = OpenService(SCM, SERVICE_NAME, SERVICE_ALL_ACCESS)) == NULL)
    {
        dwReturn = GetLastError();
        if (dwReturn == ERROR_SERVICE_DOES_NOT_EXIST)
        {
            dwReturn = NO_ERROR;
        }
        ok = false;
    }

    // Try to stop the service
    if (ok && ControlService(service, SERVICE_CONTROL_STOP, &status))
    {
        // Wait for the service to stop
        for (int retries = 100; retries-- && (status.dwCurrentState != SERVICE_STOPPED);)
        {
            Sleep(100);
            if (!QueryServiceStatus(service, &status))
            {
                break;
            }
        }
        if (status.dwCurrentState == SERVICE_STOPPED)
        {
            Sleep(1000);
        }
        else
        {
        }
    }
    else
    {
    }

    // Try to delete the service
    if (ok && !DeleteService(service))
    {
        dwReturn = GetLastError();
        if (dwReturn == ERROR_SERVICE_MARKED_FOR_DELETE)
        {
            Sleep(1000);
        }
        else
        {
            ok = false;
        }
    }

    // Cleanup
    if (service)
        CloseServiceHandle(service);
    if (SCM)
        CloseServiceHandle(SCM);
    return ok ? NO_ERROR : dwReturn;
}

int main(int argc, char *argv[])
{
    static const SERVICE_TABLE_ENTRY serviceTable[2] =
    {
        { (wchar_t*)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

    GetModuleFileName(NULL, gServiceBinPath, MAX_PATH);
    if(argc > 1)
    {
        QString argvUtf8Str = QString::fromUtf8(argv[1]);

        if(!_wcsicmp((wchar_t*)argvUtf8Str.utf16(), SERVICE_ARG_INSTALL))
        {
            return InstallService();
        }
        else if(!_wcsicmp((wchar_t*)argvUtf8Str.utf16(), SERVICE_ARG_UNINSTALL))
        {
            return UninstallService();
        }

        argvUtf8Str = QString::fromUtf8(argv[argc - 1]);

        if(!_wcsicmp((wchar_t*)argvUtf8Str.utf16(), SERVICE_ARG_SPAWN_PROVIDER))
        {
            QApplication* a = new QApplication(argc, argv);
            return ProviderMain(a);
        }
    }
    else
    {
        // If called with no options then start the service
        // The Service Control Manager will launch the executable with no
        // arguments.

        InitializeCriticalSection(&gCriticalSection);
        StartServiceCtrlDispatcher(serviceTable);
    }
    return 0;
}
