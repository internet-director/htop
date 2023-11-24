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
		bool final{ false };
	public:
		console(bool funal = false): final{final} {}
		~console();
		void clear() const;
		console& operator <<(const wchar_t* str) const;
		console& operator <<(const std::wstring& str) const;
		console& operator <<(console& (*color)(const wchar_t*)) const;
		static void SetColor(ConsoleColor text, ConsoleColor background);
	};

	htop::console& endl(const wchar_t* str);
	htop::console& start(const wchar_t* str);
	htop::console& red(const wchar_t* str);
	htop::console& blue(const wchar_t* str);
	htop::console& mgent(const wchar_t* str);
	htop::console& lmgent(const wchar_t* str);
	htop::console& green(const wchar_t* str);
	htop::console& lblue(const wchar_t* str);
	htop::console& lgreen(const wchar_t* str);
	htop::console& white(const wchar_t* str);
	htop::console& lgray(const wchar_t* str);
	htop::console& background_red(const wchar_t* str);


	static htop::console cout(true);
}