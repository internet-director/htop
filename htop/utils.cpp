#include <iomanip>
#include <sstream>
#include <Windows.h>
#include <tlhelp32.h>
#include "utils.h"

static MEMORYSTATUSEX memInfo;

namespace htop {
    void getProcessInfos(std::vector<Process>& result) {
        result.clear();
        PROCESSENTRY32W pe32;
        HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (hProcessSnap == INVALID_HANDLE_VALUE) {
            return;
        }

        pe32.dwSize = sizeof(PROCESSENTRY32W);

        if (!Process32FirstW(hProcessSnap, &pe32)) {
            return;
        }

        while (Process32NextW(hProcessSnap, &pe32)) {
            result.push_back({ pe32 });
        }
        CloseHandle(hProcessSnap);
    }

    inline std::wstring getConvertedMem(size_t sz) {
        std::wstringstream result{};
        static const wchar_t* arr[] = { L"K", L"M", L"G" };

        for (int i = 0; i < _countof(arr); i++) {
            if (sz > 1024ull * 1024ull) sz >>= 10;
            else {
                int prec = 2;
                float res = sz;
                if (sz >= 10000) prec--, res /= 1024;
                result << std::fixed << std::setprecision(prec) << res << arr[i];
                break;
            }
        }
        return result.str();
    }

    std::wstring getMemInfo()
    {
        std::wstring result{};
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);

        if (!GlobalMemoryStatusEx(&memInfo)) {
            return result;
        }

        result = getConvertedMem(memInfo.ullTotalPhys - memInfo.ullAvailPhys) + L"\\";
        result += getConvertedMem(memInfo.ullTotalPhys);
        return result;
    }
    int getMemLoad()
    {
        return memInfo.dwMemoryLoad;
    }
}
