#include <vector>
#include <string>
#include <lmcons.h>
#include <Windows.h>

namespace htop {
    struct Window;

    struct Process
    {
        PROCESSENTRY32W base;
        std::wstring username;
        HANDLE handle{ nullptr };

        size_t allocated{ 0 };
    };

    void getProcessInfos(std::vector<Process>& result);
    std::wstring getConvertedMem(size_t sz);
    std::wstring getMemInfo();
    int getMemLoad();
}