#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
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

class Window {
    HANDLE hStdOut;
    std::atomic<int> panelSize = 60;
    std::atomic<bool> executed{ true };
    std::atomic<CONSOLE_SCREEN_BUFFER_INFO> consoleInfo;
    std::wstring panel;
    std::wstring emtyPanel;
    std::jthread keysChecker;

    void checker()
    {
        while (true) {
            CONSOLE_SCREEN_BUFFER_INFO info;
            if (GetConsoleScreenBufferInfo(hStdOut,&info)) {
                //if (consoleInfo.srWindow.Right > 120) panelSize = 60;
            }
            if (GetAsyncKeyState(VK_F10)) {
                executed.store(false);
                return;
            }
            consoleInfo.store(info);
            Sleep(50);
        }
    }

public:
    Window() {
        panel.resize(120, L'|');
        emtyPanel.resize(110, L' ');
        hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        keysChecker = std::jthread(&Window::checker, this);
    }
    int show() {
        int ret{ 0 };
        SYSTEM_INFO info;
        GetSystemInfo(&info);

        while (executed.load())
        {
            htop::cout.setPosition({ 0, 1 });
            {
                std::wstring cpu{};

                for (int i = 0; i < info.dwNumberOfProcessors; i++) {
                    htop::cout << htop::blue << L"  CPU" << htop::white << L"[" << htop::lgray << L"%" << htop::white << L"]" << htop::endl;
                }
            }

            // mem info
            {
                auto memInfo = htop::getMemInfo();
                htop::cout << htop::blue << L"  Mem" << htop::white << L"[" << htop::lgreen;
                int szPanel = panelSize - memInfo.size();
                szPanel = min(szPanel, panelSize * htop::getMemLoad() / 100);
                htop::cout.write(panel.c_str(), szPanel);
                htop::cout.write(emtyPanel.c_str(), panelSize - szPanel);
                htop::cout << htop::lgray << memInfo << htop::white << L"]" << htop::endl;
            }

            // info panel
            {
                auto cInfo = consoleInfo.load();
                SHORT p = cInfo.srWindow.Bottom - 1;
                htop::cout.setPosition({ 0, p });
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
                htop::cout.write(emtyPanel.c_str(), min(emtyPanel.length(), (cInfo.srWindow.Right - 100) * 2));
                htop::cout.clear();
            }

            Sleep(500);
        }

        return ret;
    }
};

int main(int argc, const char* argv[])
{
    Window htop;
	return htop.show();
}