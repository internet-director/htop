#include <Windows.h>
#include <iostream>
#include <vector>
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
    std::atomic<int> panelSize = 60;
    std::atomic<Status> status{ NONE };
    std::condition_variable cv;
    CONSOLE_SCREEN_BUFFER_INFO window;
    std::wstring panel;
    std::wstring emtyPanel;
    std::jthread commandChecker;
    std::jthread processChecker;
    std::vector<htop::Process> procs;

    void commsChecker()
    {
        while (true) {
            CONSOLE_SCREEN_BUFFER_INFO info;
            if (GetConsoleScreenBufferInfo(hStdOut, &info)) {
                if (info.srWindow.Right - info.srWindow.Left != width() ||
                    info.srWindow.Bottom - info.srWindow.Top != height()) {
                    status.store(NONE);
                    std::unique_lock lock(mut);
                    window.srWindow = info.srWindow;
                    // clear space
                    htop::cout.clear();
                    htop::cout.cls();
                    status.store(SKIP);
                    cv.notify_one();
                }
                //if (consoleInfo.srWindow.Right > 120) panelSize = 60;
            }

            if (GetAsyncKeyState(VK_F10)) {
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
            
            status.store(SKIP);
            cv.notify_one();
            lock.unlock();
            Sleep(1000);
        }
    }

public:
    Window() {
        panel.resize(120, L'|');
        emtyPanel.resize(110, L' ');
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
                // windowSize
                auto srWindow = window;
                int szPanel = srWindow.srWindow.Top + 1;
                htop::cout.setPosition({ srWindow.srWindow.Left, SHORT(szPanel) });
                htop::cout.clear();

                {
                    std::wstring cpu{};

                    for (int i = 0; i < info.dwNumberOfProcessors / info.dwNumberOfProcessors; i++) {
                        htop::cout << htop::blue << L"  CPU" << htop::white << L"[" << htop::lgray << L"%" << htop::white << L"]" << htop::endl;
                        szPanel++;
                    }
                }

                if (status.load() == NONE) {

                    continue;
                }

                printMemInfo();
                szPanel++;
                htop::cout << htop::background_green << htop::black << L"    PID" << L" USER    " << L" NAME";
                htop::cout.write(emtyPanel.c_str(), min(min(emtyPanel.size(), width()), USHORT(width() - 21)));
                htop::cout << htop::endl;
                //+1 bcause printCommandPanel
                printProcesses(szPanel + 1);

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
        int szPanel = panelSize - memInfo.size();
        szPanel = min(szPanel, panelSize * htop::getMemLoad() / 100);
        htop::cout.write(panel.c_str(), szPanel);
        htop::cout.write(emtyPanel.c_str(), panelSize - szPanel);
        htop::cout << htop::lgray << memInfo << htop::white << L"]" << htop::endl;
    }
    void printCommandPanel() {
        htop::cout.setPosition({ window.srWindow.Left, SHORT(window.srWindow.Bottom)});
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
        htop::cout.write(emtyPanel.c_str(), min(min(emtyPanel.size(), width()), USHORT(width() - 79)));
        htop::cout.clear();
    }
    void printProcesses(int szPanel) {
        int i = 0;
        htop::cout.clear();
        for (auto& it : procs) {
            if (++i == height() - szPanel) break;
            htop::cout << std::to_wstring(it.base.th32ProcessID) << htop::endl;
        }
    }

    SHORT width() const noexcept {
        return window.srWindow.Right - window.srWindow.Left;
    }

    SHORT height() const noexcept {
        return window.srWindow.Bottom - window.srWindow.Top;
    }
};

int main(int argc, const char* argv[])
{
    Window htop;
	return htop.show();
}
