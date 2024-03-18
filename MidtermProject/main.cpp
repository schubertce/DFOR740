#include <iostream>
#include <iomanip>
#include "TaskLister.h"
#include "TaskKiller.h"

int main() {
    TaskLister taskLister;
    TaskKiller taskKiller;

    std::cout << "\nTask Manager\n\n";
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
