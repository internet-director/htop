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
		console& operator <<(const wchar_t* str) const;
		console& operator <<(const std::wstring& str) const;
		console& operator <<(console& (*color)()) const;
		static void SetColor(ConsoleColor text, ConsoleColor background);
	};

	htop::console& endl();
	htop::console& start();
	htop::console& red();
	htop::console& blue();
	htop::console& mgent();
	htop::console& lmgent();
	htop::console& green();
	htop::console& lblue();
	htop::console& lgreen();
	htop::console& white();
	htop::console& lgray();
	htop::console& background_red();

	static htop::console cout;
}