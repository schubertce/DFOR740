#ifndef TASKKILLER_H
#define TASKKILLER_H

#include <string>

class TaskKiller {
public:
    TaskKiller();

    // Function to kill a task by its PID.
    bool killTask(unsigned long pid) const;

    // Function to kill a task by its Name.
    bool killProcessByName(const std::string& processName, bool killChildProcesses) const;

    // Recursively terminate all child processes
    void terminateChildProcesses(unsigned long parentPid) const;

private:
    // Helper function to perform the actual process termination.
    bool terminateProcess(unsigned long pid) const;

};

#endif // TASKKILLER_H
