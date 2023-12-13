#include <Windows.h>

BOOL TryCloseFile(LPWSTR filename)
{
    return FALSE;
}

#ifdef _DEBUG
int main()
#else
int entry()
#endif
{
    int argc = 0;
    int code = 0;
    int dir = FALSE;
    int undo = TRUE;
    int start = 1;

    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argc < 2)
    {
        // TODO: log error
        code = -1;
        goto end;
    }
    
    if (!lstrcmpW(argv[1], L"-rf"))
    {
        if (argc < 3)
        {
            code = -1;
            goto end;
        }
        start++;
        dir = TRUE;
    }

    for (int i = start; i < argc; i++)
    {
        DWORD FileAttributes = GetFileAttributesW(argv[i]);

        if ((FileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !dir) {
            // TODO: this is directory, needed -rf flag
            continue;
        }

        SHFILEOPSTRUCTW op;
        memset(&op, 0, sizeof op);

        op.wFunc = FO_DELETE;
        op.fFlags = FOF_NOERRORUI | (undo ? FOF_ALLOWUNDO : 0);
        op.pFrom = argv[i];

        if (SHFileOperationW(&op))
        {
            int err = GetLastError();
            // TODO: print log
        }

    }

end:
	return code;
}