#include <Windows.h>
#include "console.h"

htop::console::~console()
{
	//if (final) clear();
}

void htop::console::clear() const
{
	htop::console::SetColor(White, Black);
}

htop::console& htop::console::operator<<(const wchar_t* str) const
{
	DWORD dwWrited{ 0 };
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleW(hStdOut, str, lstrlenW(str), &dwWrited, NULL);
	return htop::console{  };
}

htop::console& htop::console::operator<<(const std::wstring& str) const
{
	DWORD dwWrited{ 0 };
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleW(hStdOut, str.c_str(), str.length(), &dwWrited, NULL);
	return htop::console{};
}

htop::console& htop::console::operator<<(console& (*color)()) const
{
	color();
	return htop::console{};
}

void htop::console::SetColor(ConsoleColor text, ConsoleColor background)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	if (text == None || background == None) {
		if (GetConsoleScreenBufferInfo(hStdOut, &info)) {
			if (text == None) text = htop::ConsoleColor(info.wAttributes & 0xf);
			if (background == None) background = htop::ConsoleColor(info.wAttributes >> 4);
		} else {}
	}

	SetConsoleTextAttribute(hStdOut, (WORD)((background << 4) | text));
}

htop::console& htop::endl()
{
	return htop::console() << L"\r\n";
}

htop::console& htop::start()
{
	COORD coord{ 0, 1 };
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hStdOut, coord);
	return htop::console();
}

htop::console& htop::red()
{
	htop::console::SetColor(Red, None);
	return htop::console();
}

htop::console& htop::blue()
{
	htop::console::SetColor(Blue, None);
	return htop::console();
}

htop::console& htop::green()
{
	htop::console::SetColor(Green, None);
	return htop::console();
}

htop::console& htop::mgent()
{
	htop::console::SetColor(Magenta, None);
	return htop::console();
}

htop::console& htop::lblue()
{
	htop::console::SetColor(LightBlue, None);
	return htop::console();
}

htop::console& htop::lgreen()
{
	htop::console::SetColor(LightGreen, None);
	return htop::console();
}

htop::console& htop::lmgent()
{
	htop::console::SetColor(LightMagenta, None);
	return htop::console();
}

htop::console& htop::white()
{
	htop::console::SetColor(White, None);
	return htop::console();
}

htop::console& htop::lgray()
{
	htop::console::SetColor(LightGray, None);
	return htop::console();
}

htop::console& htop::background_red()
{
	htop::console::SetColor(None, Red);
	return htop::console();
}
