#include <windows.h>
#include <fstream>

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
void WINAPI ServiceCtrlHandler(DWORD);

// Funcția care scrie mesajul nostru
void WriteLog(const char* msg) {
    std::ofstream logFile("C:\\temp\\service_log.txt", std::ios_base::app);
    logFile << msg << std::endl;
}

int main() {
    SERVICE_TABLE_ENTRY ServiceTable[] = {
        {(LPWSTR)L"CppHelloWorldService", (LPSERVICE_MAIN_FUNCTION)ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher(ServiceTable) == FALSE) {
        return GetLastError();
    }

    return 0;
}

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {
    g_StatusHandle = RegisterServiceCtrlHandler(L"CppHelloWorldService", ServiceCtrlHandler);

    if (g_StatusHandle == NULL) return;

    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;

    // report service start
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
    ;
    WriteLog("Hello World!");

    // create event to keep the service active
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    // report that the service is active
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    // wait until the service receives the stop command
    WaitForSingleObject(g_ServiceStopEvent, INFINITE);

    // report the end of the service
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

void WINAPI ServiceCtrlHandler(DWORD CtrlCode) {
    switch (CtrlCode) {
    case SERVICE_CONTROL_STOP:
        if (g_ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
            SetEvent(g_ServiceStopEvent);
        }
        break;
    default:
        break;
    }
}
