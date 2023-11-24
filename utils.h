#include <vector>
#include <tlhelp32.h>
#include <Windows.h>

namespace htop {

    struct Process
    {
        PROCESSENTRY32W base;
    };

    std::vector<Process> getProcessInfos();
}