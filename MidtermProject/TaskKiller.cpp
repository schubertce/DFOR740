#include "TaskKiller.h"
#include <windows.h>
#include <iostream>
#include <TlHelp32.h>

TaskKiller::TaskKiller() {
        // Constructor implementation
        // Initializes newly created object TaskKiller
}

bool TaskKiller::killTask(unsigned long pid) const {
    // Terminate the main process and return success or error message
    if (terminateProcess(pid)) {
        std::cout << "\nProcess with PID " << pid << " has been terminated.\n" << std::endl;
        return true;
    }
    else {
        std::cout << "\nFailed to terminate process with PID " << pid << ".\n" << std::endl;
        return false;
    }
}

// Function to terminate process by PID
bool TaskKiller::terminateProcess(unsigned long pid) const {
    // Get handle to the process
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        return false;
    }

    // Uses TerminateProcess rather than ExitProcess to forcefully kill the  process
    BOOL result = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
    return result != 0;
}

// Function to terminate a process by its name
bool TaskKiller::killProcessByName(const std::string& processName, bool killChildProcesses) const {
    bool success = false;
    // Creates snapshot of all current processes
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        // Iterates over processes in the snapshot
        if (Process32First(hSnapshot, &pe32)) {
            do {
                // Converts the name of current process from wide character to string
                std::wstring wideName(pe32.szExeFile);
                // Compares with name from user
                if (std::string(wideName.begin(), wideName.end()) == processName) {
                    // If user chooses to also kill child processes it calles that funcntion
                    if (killChildProcesses) {
                        terminateChildProcesses(pe32.th32ProcessID); // First kill child processes
                    }

                    // Terminates the process
                    if (terminateProcess(pe32.th32ProcessID)) {
                        std::cout << "\nProcess with the Name " << processName << " has been terminated.\n" << std::endl;
                        success = true;
                    }
                    else {
                        std::cout << "\nFailed to terminate process with the Name: " << processName << ".\n" << std::endl;
                    }
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    return success;
}

void TaskKiller::terminateChildProcesses(unsigned long parentPid) const {
    // Creates snapshot of processes
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        // Iterates over processes
        if (Process32First(hSnapshot, &pe32)) {
            do {
                // Checks for child processes and terminates them
                if (pe32.th32ParentProcessID == parentPid) {
                    // Recursively terminate any children of this child process
                    terminateChildProcesses(pe32.th32ProcessID);
                    
                    // Then terminate the child process itself
                    terminateProcess(pe32.th32ProcessID);
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
}