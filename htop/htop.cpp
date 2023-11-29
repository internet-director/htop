#include <Windows.h>
#include <iostream>
#include <vector>
#include <format>
#include <string>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <tlhelp32.h>
#include "utils.h"
#include "console.h"

void color_test()
{
	htop::cout << htop::red << L"a" << htop::blue << L"LOOOOH" << htop::green << L"   aAAAAAAAAAAASDLKSL:DKL:KL:k" << htop::endl;
	htop::cout << htop::red << L"a" << htop::lblue << L"LOOOOH" << htop::lgreen << L"   aAAAAAAAAAAASDLKSL:DKL:KL:k" << htop::endl;
	htop::cout << htop::red << L"a" << htop::mgent << L"LOOOOH" << htop::lmgent << L"   aAAAAAAAAAAASDLKSL:DKL:KL:k" << htop::endl;
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

enum Status {
    KILL, 
    SKIP,
    NONE
};

struct Htop {
    HANDLE hStdOut;
    std::mutex mut;
    std::atomic<int> processPosition;
    std::atomic<Status> status{ NONE };
    std::condition_variable cv;
    CONSOLE_SCREEN_BUFFER_INFO window;
    std::jthread commandChecker;
    std::jthread processChecker;
    std::vector<htop::Process> procs;
};

int getProcsPanelSize() {
    return htop::cout.height() - (htop::cout.getConsoleInfo().dwCursorPosition.Y - htop::cout.getConsoleInfo().srWindow.Top);
}
void printMemInfo() {
    auto memInfo = htop::getMemInfo();
    htop::cout << htop::blue << L"  Mem" << htop::white << L"[" << htop::lgreen;
    int sz = htop::cout.width() / 2;
    int szPanel = sz - memInfo.size();
    szPanel = min(szPanel, sz * htop::getMemLoad() / 100);
    htop::cout.fill(szPanel, L'|');
    htop::cout.fill(sz - szPanel, L' ');
    htop::cout << htop::lgray << memInfo << htop::white << L"]" << htop::endl;
}
void printCommandPanel() {
    //htop::cout.setPosition({ window.srWindow.Left, SHORT(window.srWindow.Bottom)});
    htop::cout << htop::white << htop::background_black << L"F1" << htop::black << htop::background_lblue << L"Quit  ";
    htop::cout << htop::white << htop::background_black << L"F2" << htop::black << htop::background_lblue << L"Quit  ";
    htop::cout << htop::white << htop::background_black << L"F3" << htop::black << htop::background_lblue << L"Quit  ";
    htop::cout << htop::white << htop::background_black << L"F4" << htop::black << htop::background_lblue << L"Quit  ";
    htop::cout << htop::white << htop::background_black << L"F5" << htop::black << htop::background_lblue << L"Quit  ";
    htop::cout << htop::white << htop::background_black << L"F6" << htop::black << htop::background_lblue << L"Quit  ";
    htop::cout << htop::white << htop::background_black << L"F7" << htop::black << htop::background_lblue << L"Quit  ";
    htop::cout << htop::white << htop::background_black << L"F8" << htop::black << htop::background_lblue << L"Quit  ";
    htop::cout << htop::white << htop::background_black << L"F9" << htop::black << htop::background_lblue << L"Quit  ";
    htop::cout << htop::white << htop::background_black << L"F10" << htop::black << htop::background_lblue << L"Quit";
    htop::cout.fillLine(L' ');
    htop::cout.clear();
}
void printProcesses(Htop* data) {
    htop::cout << htop::background_green << htop::black <<
        std::format(L"{:>7} {:<20}{:7}{:7}",
            L"PID",
            L"USER",
            L"MEM",
            L"NAME")
        << htop::endl;
    htop::cout.clear();

    int sz = getProcsPanelSize();

    auto pos = htop::cout.getPosition();
    int step = data->processPosition.load() * sz;

    for (int i = step; i < data->procs.size() && i < step + sz; i++) {
        htop::cout << std::format(L"{:7} {:<20}{:7}{:<50}",
            data->procs[i].base.th32ProcessID,
            (data->procs[i].username.size() < 20) ? data->procs[i].username : data->procs[i].username.substr(0, 20) + L">",
            htop::getConvertedMem(data->procs[i].allocated),
            data->procs[i].base.szExeFile) << htop::endl;
    }
}

void commsChecker(Htop* data)
{
    while (true) {
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo(data->hStdOut, &info)) {
            if (htop::cout.width(data->window.srWindow) != htop::cout.width() ||
                htop::cout.height(data->window.srWindow) != htop::cout.height()) {
                data->status.store(NONE);
                std::unique_lock lock(data->mut);
                data->window.srWindow = info.srWindow;
                data->status.store(SKIP);
                //htop::cout.cls();
                data->cv.notify_one();
            }
            //if (consoleInfo.srWindow.Right > 120) panelSize = 60;
        }

        if (GetAsyncKeyState(VK_UP) &&
            data->processPosition.load() != 0)
        {
            data->status.store(NONE);
            std::unique_lock lock(data->mut);
            data->processPosition.fetch_sub(1);
            data->status.store(SKIP);
            data->cv.notify_one();
        }
        else if (GetAsyncKeyState(VK_DOWN) &&
            getProcsPanelSize() * data->processPosition.load() < data->procs.size())
        {
            data->status.store(NONE);
            std::unique_lock lock(data->mut);
            data->processPosition.fetch_add(1);
            data->status.store(SKIP);
            data->cv.notify_one();
        }
        else if (GetAsyncKeyState(VK_F10)) {
            data->status.store(KILL);
            data->cv.notify_one();
            return;
        }

        Sleep(10);
    }
}

void procsChecker(Htop* data)
{
    while (data->status.load() != KILL) {
        data->status.store(NONE);
        std::unique_lock lock(data->mut);
        htop::getProcessInfos(data->procs);

        if (data->procs.size() <= data->processPosition.load()) {

        }

        data->status.store(SKIP);
        data->cv.notify_one();
        lock.unlock();
        Sleep(100);
    }
}

int main(int argc, const char* argv[])
{
    Htop htop_data;
    htop_data.hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    std::jthread commandChecker = std::jthread(commsChecker, &htop_data);
    std::jthread processChecker = std::jthread(procsChecker, &htop_data);
    //HHOOK thehook = SetWindowsHookExW(WH_KEYBOARD_LL, htop::keyboardHookProc<*this>, 0, 0);


    int ret{ 0 };
    SYSTEM_INFO info;
    GetSystemInfo(&info);

    while (htop_data.status.load() != KILL)
    {
        {
            std::unique_lock lock(htop_data.mut);
            Status kill;
            htop_data.cv.wait(lock, [&] {
                kill = htop_data.status.load();
                return (kill == KILL || kill == SKIP);
                });
            if (kill == KILL) {
                break;
            }

            htop::cout.setPosition({ htop_data.window.srWindow.Left, htop_data.window.srWindow.Top });
            htop::cout << htop::endl;

            {
                std::wstring cpu{};

                for (int i = 0; i < info.dwNumberOfProcessors / info.dwNumberOfProcessors; i++) {
                    htop::cout << htop::blue << L"  CPU" << htop::white << L"[" << htop::lgray << L"%" << htop::white << L"]" << htop::endl;
                }
            }

            if (htop_data.status.load() == NONE) {

                continue;
            }

            printMemInfo();
            printProcesses(&htop_data);

            if (htop_data.status.load() == NONE) {

                continue;
            }
            printCommandPanel();
        }
        htop_data.status.store(NONE);
    }

    htop::cout.clear();
    htop::cout.cls();

    return 0;
}
