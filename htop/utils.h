#include <vector>
#include <string>
#include <lmcons.h>
#include <Windows.h>

namespace htop {
    struct Process
    {
        PROCESSENTRY32W base;
        WCHAR username[UNLEN];
        HANDLE handle;

        size_t allocated;
    };

    void getProcessInfos(std::vector<Process>& result);
    std::wstring getMemInfo();
    int getMemLoad();
}