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

enum Status {
    KILL,
    SKIP,
    NONE
};

class Window {
    HANDLE hStdOut;
    std::mutex mut;
    std::pair<int, int> procPos;
    std::atomic<Status> status{ NONE };
    std::condition_variable cv;
    CONSOLE_SCREEN_BUFFER_INFO window;
    std::jthread commandChecker;
    std::jthread processChecker;
    std::vector<htop::Process> procs;

    void commsChecker()
    {
        bool pressed = false;
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

            if (GetAsyncKeyState(VK_UP))
            {
                if (procPos.second == 0)
                {
                    if (procPos.first == 0) goto skip;
                    procPos.first--;
                    procPos.second = getProcsPanelSize();
                }
                status.store(NONE);
                std::unique_lock lock(mut);
                procPos.second--;
                status.store(SKIP);
                cv.notify_one();
            }
            else if (GetAsyncKeyState(VK_DOWN))
            {
                size_t panelSz = getProcsPanelSize();
                if (procPos.first * panelSz + procPos.second >= procs.size())
                {
                    procPos.first = procs.size() - panelSz;
                    procPos.second = procs.size() % panelSz;
                    goto skip;
                }
                if (procPos.second == panelSz)
                {
                    procPos.second = -1;
                    procPos.first++;
                }

                status.store(NONE);
                std::unique_lock lock(mut);
                procPos.second++;
                status.store(SKIP);
                cv.notify_one();
            }
            else if (GetAsyncKeyState(VK_F10)) {
                status.store(KILL);
                cv.notify_one();
                return;
            }

        skip:;
            Sleep(300);
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
        int cnt = 0;
        auto pos = htop::cout.getPosition();

        for (int i = procPos.first * sz; i < procs.size() && i < (procPos.first + 1) * sz; i++, cnt++) {
            bool flag = false;

            if (cnt == procPos.second)
            {
                htop::cout << htop::background_lblue << htop::black;
                flag = true;
            }

            htop::cout << std::format(L"{:7} {:<20}{:7}{:<50}",
                procs[i].base.th32ProcessID,
                (procs[i].username.size() < 20) ? procs[i].username : procs[i].username.substr(0, 20) + L">",
                htop::getConvertedMem(procs[i].allocated),
                procs[i].base.szExeFile);

            if (flag) htop::cout.clear();
            htop::cout << htop::endl;
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
