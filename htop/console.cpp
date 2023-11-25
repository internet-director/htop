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

void htop::console::write(const wchar_t* str, size_t sz)
{
	DWORD dwWrited{ 0 };
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleW(hStdOut, str, sz, &dwWrited, NULL);
}

void htop::console::setPosition(COORD pos)
{
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hStdOut, pos);
}

htop::console& htop::operator<<(htop::console& estr, const wchar_t* str)
{
	htop::console::write(str, lstrlenW(str));
	return estr;
}

htop::console& htop::operator<<(htop::console& estr, const std::wstring& str)
{
	htop::console::write(str.c_str(), str.length());
	return estr;
}

htop::console& htop::operator<<(htop::console& estr, htop::console& (*color)(console&))
{
	return color(estr);
}

htop::console& htop::endl(console& estr)
{
	estr << L"\r\n";
	return estr;
}

htop::console& htop::start(console& estr)
{
	htop::console::setPosition({ 0, 1 });
	return estr;
}

htop::console& htop::red(console& estr)
{
	htop::console::SetColor(Red, None);
	return estr;
}

htop::console& htop::blue(console& estr)
{
	htop::console::SetColor(Blue, None);
	return estr;
}

htop::console& htop::green(console& estr)
{
	htop::console::SetColor(Green, None);
	return estr;
}

htop::console& htop::mgent(console& estr)
{
	htop::console::SetColor(Magenta, None);
	return estr;
}

htop::console& htop::lblue(console& estr)
{
	htop::console::SetColor(LightBlue, None);
	return estr;
}

htop::console& htop::lgreen(console& estr)
{
	htop::console::SetColor(LightGreen, None);
	return estr;
}

htop::console& htop::lmgent(console& estr)
{
	htop::console::SetColor(LightMagenta, None);
	return estr;
}

htop::console& htop::white(console& estr)
{
	htop::console::SetColor(White, None);
	return estr;
}

htop::console& htop::black(console& estr)
{
	htop::console::SetColor(Black, None);
	return estr;
}

htop::console& htop::lgray(console& estr)
{
	htop::console::SetColor(DarkGray, None);
	return estr;
}

htop::console& htop::background_red(console& estr)
{
	htop::console::SetColor(None, Red);
	return estr;
}

htop::console& htop::background_lblue(console& estr)
{
	htop::console::SetColor(None, LightBlue);
	return estr;
}

htop::console& htop::background_black(console& estr)
{
	htop::console::SetColor(None, Black);
	return estr;
}
