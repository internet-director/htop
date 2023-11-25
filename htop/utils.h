#include <vector>
#include <string>
#include <Windows.h>

namespace htop {
    struct Process
    {
        PROCESSENTRY32W base;
    };

    void getProcessInfos(std::vector<Process>& result);
    std::wstring getMemInfo();
    int getMemLoad();
}