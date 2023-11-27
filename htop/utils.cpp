#include <iomanip>
#include <sstream>
#include <Windows.h>
#include <psapi.h>
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
            Process proc;
            proc.base = pe32;
            proc.handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe32.th32ProcessID);
            *proc.username = 0;

            if (proc.handle) {
                PROCESS_MEMORY_COUNTERS ProcMemCounters;
                if (GetProcessMemoryInfo(proc.handle, &ProcMemCounters, sizeof(ProcMemCounters))) {
                    proc.allocated = (size_t)ProcMemCounters.WorkingSetSize;
                }

                HANDLE ProcessTokenHandle = nullptr;
                if (OpenProcessToken(proc.handle, TOKEN_READ, &ProcessTokenHandle)) {
                    DWORD ReturnLength;

                    GetTokenInformation(ProcessTokenHandle, TokenUser, 0, 0, &ReturnLength);
                    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                        PTOKEN_USER TokenUserStruct = (PTOKEN_USER)new byte[ReturnLength];

                        if (GetTokenInformation(ProcessTokenHandle, TokenUser, TokenUserStruct, ReturnLength, &ReturnLength)) {
                            SID_NAME_USE NameUse;
                            DWORD NameLength = UNLEN;
                            TCHAR DomainName[MAX_PATH];
                            DWORD DomainLength = MAX_PATH;

                            LookupAccountSidW(0, TokenUserStruct->User.Sid, proc.username, &NameLength, DomainName, &DomainLength, &NameUse);

                            proc.username[NameLength] = 0;
                        }
                    }
                    CloseHandle(ProcessTokenHandle);
                }
                CloseHandle(proc.handle);
                proc.handle = nullptr;
            }
            result.emplace_back(std::move(proc));
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
                if (sz >= 1024) prec--, res /= 1024;
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
