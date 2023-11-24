#include <tlhelp32.h>
#include "utils.h"

namespace htop {
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
}
