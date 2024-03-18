# DFOR740 - Midterm Project

Tasklist\Taskkill Project

For this midterm, I was assigned the project of creating a program that would mimic the functionality of both tasklist and taskkill using C++. The code was written using Microsoft Visual Studio and is broken into three cpp files and two header files, which are included here along with the compiled executable.

Simply clone this repository and open the MidtermProject.sln file at the root in Microsoft Visual Studio to edit the code and build the executable. The current build is located here: \x64\Debug\MidtermProject.exe

Launch the executable from an elevated command prompt, and you will be presented with a menu.

    Task Manager

    1. List Tasks
    2. List Tasks (Verbose)
    3. Kill Task by PID
    4. Kill Task by Name

    Enter your choice:

The first two options will list all currently running processes on the system, with varying details depending on whether the Verbose option is selected.

The second set of options will forcefully terminate a selected process with an additional option to terminate child processes. Once you choose options 2 or 3 and then enter either the Process ID or Name of the process, you will presented with the option to:

    Also kill child processes? (Y/N):

If yes is selected, the program will kill all child processes and any processes that those children have spawned.

The program immediately exits once the tasks are listed or killed.

