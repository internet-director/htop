#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <tlhelp32.h>
#include "console.h"

void color_test()
{
	htop::cout << htop::red << L"a" << htop::blue << L"LOOOOH" << htop::green << L"   aAAAAAAAAAAASDLKSL:DKL:KL:k" << htop::endl;
	htop::cout << htop::red << L"a" << htop::lblue << L"LOOOOH" << htop::lgreen << L"   aAAAAAAAAAAASDLKSL:DKL:KL:k" << htop::endl;
	htop::cout << htop::red << L"a" << htop::mgent << L"LOOOOH" << htop::lmgent << L"   aAAAAAAAAAAASDLKSL:DKL:KL:k" << htop::endl;
}

struct Process
{
    PROCESSENTRY32W base;
};

std::vector<Process> getProcessInfos() {
    std::vector<Process> result{};

    PROCESSENTRY32W pe32;
    auto hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return {};
    }

    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(hProcessSnap, &pe32)) {
        return {};
    }

    while (Process32NextW(hProcessSnap, &pe32)) {
        result.push_back({ pe32 });
    }
    CloseHandle(hProcessSnap);
    return result;
}

void calculateCpusLoad()
{
    FILETIME idleTime, kernelTime, userTime;
    ULARGE_INTEGER idleStart, idleEnd, kernelStart, kernelEnd, userStart, userEnd;

    // Get initial system times
    GetSystemTimes(&idleTime, &kernelTime, &userTime);
    idleStart.LowPart = idleTime.dwLowDateTime;
    idleStart.HighPart = idleTime.dwHighDateTime;
    kernelStart.LowPart = kernelTime.dwLowDateTime;
    kernelStart.HighPart = kernelTime.dwHighDateTime;
    userStart.LowPart = userTime.dwLowDateTime;
    userStart.HighPart = userTime.dwHighDateTime;

    while (true) {
        // Wait for 1 second
        Sleep(500);

        // Get current system times
        GetSystemTimes(&idleTime, &kernelTime, &userTime);
        idleEnd.LowPart = idleTime.dwLowDateTime;
        idleEnd.HighPart = idleTime.dwHighDateTime;
        kernelEnd.LowPart = kernelTime.dwLowDateTime;
        kernelEnd.HighPart = kernelTime.dwHighDateTime;
        userEnd.LowPart = userTime.dwLowDateTime;
        userEnd.HighPart = userTime.dwHighDateTime;

        // Calculate CPU usage percentage
        ULONGLONG idleDelta = idleEnd.QuadPart - idleStart.QuadPart;
        ULONGLONG kernelDelta = kernelEnd.QuadPart - kernelStart.QuadPart;
        ULONGLONG userDelta = userEnd.QuadPart - userStart.QuadPart;
        ULONGLONG totalDelta = kernelDelta + userDelta;
        double usagePercent = 100.0 * (1.0 - static_cast<double>(idleDelta) / static_cast<double>(totalDelta));

        std::cout << "CPU Usage: " << usagePercent << "%" << std::endl;

        // Update start times for the next iteration
        idleStart = idleEnd;
        kernelStart = kernelEnd;
        userStart = userEnd;
    }
}

int main(int argc, const char* argv[])
{
    //calculateCpusLoad();
    SYSTEM_INFO info;
    MEMORYSTATUSEX memInfo;

    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    while (true)
    {
        {
            std::wstring cpu{};
            GetSystemInfo(&info);

            for (int i = 0; i < info.dwNumberOfProcessors; i++) {
                htop::cout << htop::lblue << L"  CPU" << htop::white << L"[" << htop::lgray << L"%" << htop::white << L"]" << htop::endl;
            }
        }
        
        {
            GlobalMemoryStatusEx(&memInfo);
             memInfo.ullTotalPageFile;
            memInfo.ullTotalPhys;

            auto total = std::to_wstring(memInfo.ullTotalPhys);
            auto allocated = std::to_wstring(memInfo.ullTotalPhys - memInfo.ullAvailPhys);

            htop::cout << htop::lblue << L"  Mem" << htop::white << L"[" << htop::lgray << allocated << L"\\" << total << L"M" << htop::white << L"]" << htop::endl;
        }

        htop::cout << htop::start;
        Sleep(500);
    }
    for (auto& it : getProcessInfos()) {
        htop::cout << it.base.szExeFile << htop::endl;
    }
    color_test();
	return 0;
}