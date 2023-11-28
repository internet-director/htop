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

class Window {
    HANDLE hStdOut;
    std::mutex mut;
    std::atomic<int> processPosition;
    std::atomic<Status> status{ NONE };
    std::condition_variable cv;
    CONSOLE_SCREEN_BUFFER_INFO window;
    std::jthread commandChecker;
    std::jthread processChecker;
    std::vector<htop::Process> procs;

    void commsChecker()
    {
        while (true) {
            CONSOLE_SCREEN_BUFFER_INFO info;
            if (GetConsoleScreenBufferInfo(hStdOut, &info)) {
                if (htop::cout.width(window.srWindow) != htop::cout.width() ||
                    htop::cout.height(window.srWindow) != htop::cout.height()) {
                    status.store(NONE);
                    std::unique_lock lock(mut);
                    window.srWindow = info.srWindow;
                    status.store(SKIP);
                    //htop::cout.cls();
                    cv.notify_one();
                }
                //if (consoleInfo.srWindow.Right > 120) panelSize = 60;
            }

            if (GetAsyncKeyState(VK_UP) && 
                processPosition.load() != 0) 
            {
                status.store(NONE);
                std::unique_lock lock(mut);
                processPosition.fetch_sub(1);
                status.store(SKIP);
                cv.notify_one();
            }
            else if (GetAsyncKeyState(VK_DOWN) && 
                getProcsPanelSize() * processPosition.load() < procs.size()) 
            {
                status.store(NONE);
                std::unique_lock lock(mut);
                processPosition.fetch_add(1);
                status.store(SKIP);
                cv.notify_one();
            } else if (GetAsyncKeyState(VK_F10)) {
                status.store(KILL);
                cv.notify_one();
                return;
            }

            Sleep(10);
        }
    }

    void procsChecker()
    {      
        while (status.load() != KILL) {
            status.store(NONE);
            std::unique_lock lock(mut);
            htop::getProcessInfos(procs);

            if (procs.size() <= processPosition.load()) {

            }
            
            status.store(SKIP);
            cv.notify_one();
            lock.unlock();
            Sleep(1000);
        }
    }

public:
    Window() {
        hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        commandChecker = std::jthread(&Window::commsChecker, this);
        processChecker = std::jthread(&Window::procsChecker, this);
    }
    int show() {
        int ret{ 0 };
        SYSTEM_INFO info;
        GetSystemInfo(&info);

        while (status.load() != KILL)
        {
            {
                std::unique_lock lock(mut);
                Status kill;
                cv.wait(lock, [&] {
                    kill = status.load();
                    return (kill == KILL || kill == SKIP);
                    });
                if (kill == KILL) {
                    break;
                }

                htop::cout.setPosition({ window.srWindow.Left, window.srWindow.Top });
                htop::cout << htop::endl;

                {
                    std::wstring cpu{};

                    for (int i = 0; i < info.dwNumberOfProcessors / info.dwNumberOfProcessors; i++) {
                        htop::cout << htop::blue << L"  CPU" << htop::white << L"[" << htop::lgray << L"%" << htop::white << L"]" << htop::endl;
                    }
                }

                if (status.load() == NONE) {

                    continue;
                }

                printMemInfo();
                printProcesses();

                if (status.load() == NONE) {

                    continue;
                }
                printCommandPanel();
            }
            status.store(NONE);
        }

        htop::cout.clear();
        htop::cout.cls();
        return ret;
    }

private:
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
    void printProcesses() {
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
        int step = processPosition.load() * sz;

        for (int i = step; i < procs.size() && i < step + sz; i++) {
            htop::cout << std::format(L"{:7} {:<20}{:7}{:<50}",
                procs[i].base.th32ProcessID,
                (procs[i].username.size() < 20) ? procs[i].username : procs[i].username.substr(0, 20) + L">",
                htop::getConvertedMem(procs[i].allocated),
                procs[i].base.szExeFile) << htop::endl;
        }
    }

    int getProcsPanelSize() const noexcept {
        return htop::cout.height() - (htop::cout.getConsoleInfo().dwCursorPosition.Y - htop::cout.getConsoleInfo().srWindow.Top);
    }
};

int main(int argc, const char* argv[])
{
    Window htop;
	return htop.show();
}
