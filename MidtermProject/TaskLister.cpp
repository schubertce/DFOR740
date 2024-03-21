#include "TaskLister.h"
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <codecvt> 
#include <locale>
#include <map>
#include <vector>
#include <tchar.h>
#include <sstream>
#include <iomanip>
#include <sddl.h>
#include <psapi.h>

TaskLister::TaskLister() {
    // Constructor implementation
    // Initializes newly created object Tasklister
}
// Helper function to convert FILETIME to total seconds
ULONGLONG FileTimeToTotalSeconds(const FILETIME& ft) {
    ULONGLONG total = ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    return total / 10000000; // Convert 100-nanosecond units to seconds
}

// Main function to gather process information
std::vector<TaskInfo> TaskLister::fetchTasks() const {
    //ElevatePrivileges(); // Elevate privileges
    std::vector<TaskInfo> tasks;
    auto servicesMap = fetchServices(); // Fetch services and their PIDs
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // Creates snapshot of the process, threads, etc.
    
    // Checks if snapshot is valid and starts pulling information if it is
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        // Structure that contains information about the processes in the snapshot
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);

        // loops through all the processes in the snapshot assigning data to the pe32 object
        if (Process32First(hSnapshot, &pe32)) {
            do {
                // Opens the process to query information
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
                if (hProcess) {
                    // Structure that stores information about the memory of the process
                    PROCESS_MEMORY_COUNTERS pmc;
                    // Stores all the process information
                    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                        TaskInfo task;
                        task.pid = pe32.th32ProcessID; // Assign PID to variable
                        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter; // Convert WCHAR array to std::string
                        task.name = converter.to_bytes(pe32.szExeFile); // Assign converted Process Name to variable
                        DWORD sessionId;
                        if (ProcessIdToSessionId(pe32.th32ProcessID, &sessionId)) {
                            task.sessionNumber = sessionId; // Assign session ID to variable
                        }
                        task.memoryUsage = pmc.WorkingSetSize; // Assign memory usage to variable

                        tasks.push_back(task);
                    }
                    CloseHandle(hProcess);
                };
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    // Get services
    for (auto& task : tasks) {
        auto it = servicesMap.find(task.pid);
        if (it != servicesMap.end()) {
            task.services = it->second;
        }
    }

    // Fetch username
    for (auto& task : tasks) {
        // Opens handle to process identified by PID stores info in hProcess
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, task.pid);
        // Checks that hProcess is not null and then calls the GetUserNameFromProcess function
        if (hProcess != NULL) {
            task.username = GetUserNameFromProcess(hProcess);
            CloseHandle(hProcess);
        }
        else {
            task.username = "Unavailable";
        }
    }

         
    // Fetch CPU Time
    for (auto& task : tasks) {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, task.pid);
        if (hProcess) {
            FILETIME creationTime, exitTime, kernelTime, userTime;
            // Retrieves process times
            if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
                ULONGLONG totalSeconds = FileTimeToTotalSeconds(kernelTime) + FileTimeToTotalSeconds(userTime);

                // Convert totalSeconds to HH:MM:SS
                std::ostringstream oss;
                oss << std::setw(2) << std::setfill('0') << totalSeconds / 3600 << ":"  // hours
                    << std::setw(2) << std::setfill('0') << (totalSeconds / 60) % 60 << ":"  // minutes
                    << std::setw(2) << std::setfill('0') << totalSeconds % 60;  // seconds
                task.cpuTime = oss.str();
            }
            CloseHandle(hProcess);
        }
    }

    return tasks;
}


// Get Service associated with process
std::map<unsigned long, std::vector<std::string>> TaskLister::fetchServices() {
    // Initializes serviceMap
    std::map<unsigned long, std::vector<std::string>> serviceMap;

    // Open service control manager to enumerate services
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (scm == NULL) {
        return serviceMap;
    }

    // Determine buffer size for services and allocates enough to hold services information
    DWORD bytesNeeded = 0, servicesReturned = 0, resumeHandle = 0;
    EnumServicesStatusEx(
        scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
        NULL, 0, &bytesNeeded, &servicesReturned, &resumeHandle, NULL);

    
    // Calls EnumServicesStatusEx with allocated buffer to retrieve services info
    BYTE* buffer = new BYTE[bytesNeeded];
    EnumServicesStatusEx(
        scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
        buffer, bytesNeeded, &bytesNeeded, &servicesReturned, &resumeHandle, NULL);

    // Casts the buffer to LPENUM_SERVICE_STATUS_PROCESS
    LPENUM_SERVICE_STATUS_PROCESS services = (LPENUM_SERVICE_STATUS_PROCESS)buffer;

    // Iterates through the services and adds them to the servicemap by corresponding PID
    for (unsigned long i = 0; i < servicesReturned; ++i) {
        auto& service = services[i];
        DWORD pid = service.ServiceStatusProcess.dwProcessId;

        if (pid != 0) { // Only consider services with a non-zero PID
            TCHAR* serviceName = service.lpServiceName;
            std::wstring wServiceName(serviceName);
            std::string sServiceName(wServiceName.begin(), wServiceName.end());

            serviceMap[pid].push_back(sServiceName);
        }
    }

    // Cleanup
    delete[] buffer;
    CloseServiceHandle(scm);

    return serviceMap;
}

// Get username associated with process
std::string TaskLister::GetUserNameFromProcess(HANDLE processHandle) {
    HANDLE tokenHandle = NULL;
    // Attempts to open the security token of the process
    if (!OpenProcessToken(processHandle, TOKEN_QUERY, &tokenHandle)) {
        return "Unavailable";
    }

    DWORD tokenInfoLength = 0;
    // Retrieve the size needed to store token information
    GetTokenInformation(tokenHandle, TokenUser, NULL, 0, &tokenInfoLength);
    // Allocates buffer large enough to hold user information
    PTOKEN_USER tokenUser = (PTOKEN_USER)new BYTE[tokenInfoLength];
    // GetTokenInformation is called again to get the information and stores it in tokenUser
    if (!GetTokenInformation(tokenHandle, TokenUser, tokenUser, tokenInfoLength, &tokenInfoLength)) {
        delete[] tokenUser;
        CloseHandle(tokenHandle);
        return "Unavailable";
    }

    WCHAR name[256];
    DWORD nameLen = sizeof(name) / sizeof(WCHAR);
    WCHAR domain[256];
    DWORD domainLen = sizeof(domain) / sizeof(WCHAR);
    SID_NAME_USE sidType;

    std::string userName = "Unavailable";
    // Converts the SID in tokenUser to readable username and converts to a string
    if (LookupAccountSid(NULL, tokenUser->User.Sid, name, &nameLen, domain, &domainLen, &sidType)) {
        std::wstring wName(name);
        userName = std::string(wName.begin(), wName.end());
    }
    
    // Cleanup
    delete[] tokenUser;
    CloseHandle(tokenHandle);

    return userName;
}

void TaskLister::listTasks(bool verbose) const {
    auto tasks = fetchTasks();
    std::cout << "\n";
    for (const auto& task : tasks) {
        printTask(task, verbose);
    }
}

void TaskLister::printTask(const TaskInfo& task, bool verbose) const {
    const int pidWidth = 10;
    const int nameWidth = 45;
    const int serviceWidth = 75;
    const int sessionNumWidth = 10;
    const int userWidth = 25;
    const int memUsageWidth = 30;
    const int cpuTimeWidth = 20;

    // Print Process ID
    std::cout << std::left << std::setw(pidWidth) << task.pid;

    // Print Process Name
    std::cout << std::left << std::setw(nameWidth) << task.name;

    // Print Services
    if (!task.services.empty()) {
        std::string allServices;
        for (const auto& service : task.services) {
            allServices += service + "; ";
        }
        std::cout << std::left << std::setw(serviceWidth) << allServices;
    }
    else {
        std::cout << std::left << std::setw(serviceWidth) << "N/A";
    }

    // Print Verbose Data
    if (verbose) {
        std::cout << std::left << std::setw(userWidth) << task.username;
        std::cout << std::left << std::setw(sessionNumWidth) << task.sessionNumber;
        std::cout << std::left << std::setw(memUsageWidth) << task.memoryUsage;
        std::cout << std::left << std::setw(cpuTimeWidth) << task.cpuTime;
    }

    std::cout << std::endl;
}

