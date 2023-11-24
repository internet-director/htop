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

htop::console& htop::console::operator<<(console& (*color)(const wchar_t*)) const
{
	color(nullptr);
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

htop::console& htop::endl(const wchar_t* str)
{
	return htop::console() << L"\r\n";
}

htop::console& htop::start(const wchar_t* str)
{
	COORD coord{ 0, 0 };
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hStdOut, coord);
	return htop::console();
}


htop::console& htop::red(const wchar_t* str)
{
	htop::console::SetColor(Red, None);
	return htop::console();
}

htop::console& htop::blue(const wchar_t* str)
{
	htop::console::SetColor(Blue, None);
	return htop::console();
}

htop::console& htop::green(const wchar_t* str)
{
	htop::console::SetColor(Green, None);
	return htop::console();
}

htop::console& htop::mgent(const wchar_t* str)
{
	htop::console::SetColor(Magenta, None);
	return htop::console();
}

htop::console& htop::lblue(const wchar_t* str)
{
	htop::console::SetColor(LightBlue, None);
	return htop::console();
}

htop::console& htop::lgreen(const wchar_t* str)
{
	htop::console::SetColor(LightGreen, None);
	return htop::console();
}

htop::console& htop::lmgent(const wchar_t* str)
{
	htop::console::SetColor(LightMagenta, None);
	return htop::console();
}

htop::console& htop::white(const wchar_t* str)
{
	htop::console::SetColor(White, None);
	return htop::console();
}

htop::console& htop::lgray(const wchar_t* str)
{
	htop::console::SetColor(LightGray, None);
	return htop::console();
}

htop::console& htop::background_red(const wchar_t* str)
{
	htop::console::SetColor(None, Red);
	return htop::console();
}
