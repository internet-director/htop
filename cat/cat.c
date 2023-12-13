#include <Windows.h>

HANDLE hout = NULL;

BOOL writeConsole(const PWCHAR buffer, DWORD len)
{
    if (len == 0) return FALSE;
    return WriteConsoleA(hout, buffer, len, &len, NULL);
}

#define log(x) writeConsole(x, lstrlenA(x))

#ifdef _DEBUG
int main()
#else
int entry()
#endif
{
    int argc = 0;
    int code = 0;

    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argc < 2)
    {
        // TODO: log error
        code = -1;
        goto end;
    }

    hout = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hout == NULL)
    {
        // TODO: log error
        code = -2;
        goto end;
    }

    for (int i = 1; i < argc; i++)
    {
        HANDLE file = CreateFileW(argv[i], GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, NULL);

        if (file != INVALID_HANDLE_VALUE)
        {
            BYTE buffer[2048];
            DWORD len;
            SetFilePointer(file, 0, NULL, FILE_BEGIN);

            while (ReadFile(file, buffer, sizeof buffer, &len, NULL) && writeConsole(buffer, len));

            CloseHandle(file);
        }
        else log("Cant open file!");

        if (i + 1 != argc) log("\r\n");
    }

end:
    return code;
}
