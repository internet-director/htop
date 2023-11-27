#include <string>

namespace htop {
	enum ConsoleColor
	{
		Black = 0,
		Blue = 1,
		Green = 2,
		Cyan = 3,
		Red = 4,
		Magenta = 5,
		Brown = 6,
		LightGray = 7,
		DarkGray = 8,
		LightBlue = 9,
		LightGreen = 10,
		LightCyan = 11,
		LightRed = 12,
		LightMagenta = 13,
		Yellow = 14,
		White = 15,
		None = 16
	};

	class console {
	public:
		console() = default;
		~console();
		void clear() const;
		static void SetColor(ConsoleColor text, ConsoleColor background);
		static void write(const wchar_t* str);
		static void write(const wchar_t* str, size_t sz);
		static void fill(COORD pos, size_t len, wchar_t fill);
		static void fill(size_t len, wchar_t fill);
		static void fillLine(wchar_t fill);
		static void setPosition(COORD pos);
		static void addYPosition(int len);
		static COORD getPosition();
		static CONSOLE_SCREEN_BUFFER_INFO getConsoleInfo();
		static void cls();

		static SHORT width() {
			auto window = getConsoleInfo();
			return width(window.srWindow);
		}

		static SHORT height() {
			auto window = getConsoleInfo();
			return height(window.srWindow);
		}

		static SHORT width(SMALL_RECT srWindow) {
			return srWindow.Right - srWindow.Left;
		}

		static SHORT height(SMALL_RECT srWindow) {
			return srWindow.Bottom - srWindow.Top;
		}
	};

	console& operator <<(console& estr, const wchar_t* str);
	console& operator <<(console& estr, const std::wstring& str);
	console& operator <<(console& estr, console& (*color)(console&));

	htop::console& endl(console& estr);
	htop::console& start(console& estr);
	htop::console& red(console& estr);
	htop::console& blue(console& estr);
	htop::console& mgent(console& estr);
	htop::console& lmgent(console& estr);
	htop::console& green(console& estr);
	htop::console& lblue(console& estr);
	htop::console& lgreen(console& estr);
	htop::console& white(console& estr);
	htop::console& black(console& estr);
	htop::console& lgray(console& estr);
	htop::console& background_red(console& estr);
	htop::console& background_lblue(console& estr);
	htop::console& background_black(console& estr);
	htop::console& background_green(console& estr);

	static htop::console cout;
}
