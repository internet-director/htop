#include <Windows.h>
#include "console.h"

htop::console::~console()
{
	//if (final) clear();
}

void htop::console::clear() const
{
	htop::console::SetColor(LightGray, Black);
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

void htop::console::cls()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	SMALL_RECT scrollRect;
	COORD scrollTarget;
	CHAR_INFO fill;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// Get the number of character cells in the current buffer.
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
	{
		return;
	}

	// Scroll the rectangle of the entire buffer.
	scrollRect.Left = 0;
	scrollRect.Top = 0;
	scrollRect.Right = csbi.dwSize.X;
	scrollRect.Bottom = csbi.dwSize.Y;

	// Scroll it upwards off the top of the buffer with a magnitude of the entire height.
	scrollTarget.X = 0;
	scrollTarget.Y = (SHORT)(0 - csbi.dwSize.Y);

	// Fill with empty spaces with the buffer's default text attribute.
	fill.Char.UnicodeChar = L' ';
	fill.Attributes = csbi.wAttributes;

	// Do the scroll
	ScrollConsoleScreenBufferW(hConsole, &scrollRect, NULL, scrollTarget, &fill);

	// Move the cursor to the top left corner too.
	csbi.dwCursorPosition.X = 0;
	csbi.dwCursorPosition.Y = 0;

	SetConsoleCursorPosition(hConsole, csbi.dwCursorPosition);
}

void htop::console::write(const wchar_t* str)
{
	write(str, lstrlenW(str));
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
	htop::console::write(str);
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

htop::console& htop::background_green(console& estr)
{
	htop::console::SetColor(None, Green);
	return estr;
}
