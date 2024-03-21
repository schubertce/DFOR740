#include <iostream>
#include <iomanip>
#include "TaskLister.h"
#include "TaskKiller.h"


// Function to elevate privilege
// hToken is the handle to access the token for a process
// lpszPrivilege is the pointer to the name of the privilege
// bEnablePrivilege indicates whether to enable or disable the privilage
bool SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege) {
    TOKEN_PRIVILEGES tp;
    LUID luid;

    // Converts privilege name to Local Unique Identifier (LUID)
    if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid)) {
        std::cerr << "LookupPrivilegeValue error: " << GetLastError() << std::endl;
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege) {
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    }
    else {
        tp.Privileges[0].Attributes = 0;
    }

    // Makes the change to the privilege
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)) {
        std::cerr << "AdjustTokenPrivileges error: " << GetLastError() << std::endl;
        return false;
    }

    // Error handling
    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
        std::cerr << "The token does not have the specified privilege. \n";
        return false;
    }

    return true;
}

// Function to increase privileges of the current process by enabling the SE_DEBUG_NAME privilege
void ElevatePrivileges() {
    HANDLE hToken;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
        SetPrivilege(hToken, SE_DEBUG_NAME, TRUE);
        CloseHandle(hToken);
    }
}

int main() {
    TaskLister taskLister;
    TaskKiller taskKiller;

    ElevatePrivileges();

    std::cout << "Task Manager\n\n";
    std::cout << "1. List Tasks\n";
    std::cout << "2. List Tasks (Verbose)\n";
    std::cout << "3. Kill Task by PID\n";
    std::cout << "4. Kill Task by Name\n\n";
    std::cout << "Enter your choice: ";

    int choice;
    std::cin >> choice;

    switch (choice) {
    case 1: {
        const int pidWidth = 10;
        const int nameWidth = 45;
        const int serviceWidth = 75;
        const int consoleWidth = 100;

        std::cout << "\nCurrent Running Processes\n";
        std::string equalsline(consoleWidth, '=');
        std::cout << equalsline << std::endl;
        std::cout << '\n';
        std::cout << std::left << std::setw(pidWidth) << "PID"
                  << std::setw(nameWidth) << "Name"
                  << std::setw(serviceWidth) << "Services";
        std::string dashline(consoleWidth, '-');
        std::cout << '\n';
        std::cout << dashline << std::endl;
        taskLister.listTasks(false); // false for non-verbose
        break;
    }
    case 2: {
        const int pidWidth = 10;
        const int nameWidth = 45;
        const int serviceWidth = 75;
        const int sessionNumWidth = 10;
        const int userWidth = 25;
        const int memUsageWidth = 30;
        const int cpuTimeWidth = 20;
        const int consoleWidth = 220;

        std::cout << "\nCurrent Running Processes - Verbose\n";
        std::string equalsline(consoleWidth, '=');
        std::cout << equalsline << std::endl;
        std::cout << '\n';
        std::cout << std::left << std::setw(pidWidth) << "PID"
            << std::setw(nameWidth) << "Name"
            << std::setw(serviceWidth) << "Services"
            << std::setw(userWidth) << "User"
            << std::setw(sessionNumWidth) << "Session"
            << std::setw(memUsageWidth) << "Memory Usage (bytes)"
            << std::setw(cpuTimeWidth) << "CPU Time";
        std::string dashline(consoleWidth, '-');
        std::cout << '\n';
        std::cout << dashline << std::endl;
        taskLister.listTasks(true); // true for verbose
        break;
    }
    case 3: {
        int pid;
        std::cout << "\nEnter PID to kill: ";
        std::cin >> pid;

        char killChildChoice;
        std::cout << "\nAlso kill child processes? (Y/N): ";
        std::cin >> killChildChoice;

        if (killChildChoice == 'Y' || killChildChoice == 'y') {
            taskKiller.terminateChildProcesses(pid);
        }

        taskKiller.killTask(pid);
        break;
    }
    case 4: {
        std::string processName;
        std::cout << "\nEnter Name of Process to kill: ";
        std::cin >> processName;

        char killChildChoice;
        std::cout << "\nAlso kill child processes? (Y/N): ";
        std::cin >> killChildChoice;
        bool killChildren = (killChildChoice == 'Y' || killChildChoice == 'y');

        taskKiller.killProcessByName(processName, killChildren);
        break;
    }
    default:
        std::cout << "Invalid choice.\n";
    }

    return 0;
}
