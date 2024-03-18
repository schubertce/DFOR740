#ifndef TASKLISTER_H
#define TASKLISTER_H

#include <Windows.h>
#include <vector>
#include <string>
#include <map>


// Struct to store information about a single task.
struct TaskInfo {
    unsigned long pid; // Process ID
    std::string name;  // Process Name
    std::vector<std::string> services; // Associated Services
    std::string username; // User Name
    std::string sessionName; // Session Name
    unsigned long sessionNumber; // Session Number
    SIZE_T memoryUsage;  // Memory Usage
    std::string status; // Status
    std::string cpuTime; // CPU Time
    std::string windowTitle; // Window Title
};

class TaskLister {
public:
    TaskLister();

    // Function to list all tasks.
    void listTasks(bool verbose) const;

    // Function to fetch services associated with PIDs
    static std::map<unsigned long, std::vector<std::string>> fetchServices();

    // Function to fetch username
    static std::string GetUserNameFromProcess(HANDLE processHandle);

    // Function to fetch Window Titles
    void GetAllWindowTitles(std::map<DWORD, std::string>& windowTitles);

private:
    // Function to fetch tasks from the system.
    std::vector<TaskInfo> fetchTasks() const;

    // Helper function to print task details.
    void printTask(const TaskInfo& task, bool verbose) const;
};

#endif // TASKLISTER_H

