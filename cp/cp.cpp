#include <Windows.h>

#define CMP(X, Y, Z) !lstrcmpW(X, L##Y) || !lstrcmpW(X, L##Z)
#define alloc(SZ) HeapAlloc(GetProcessHeap(), NULL, SZ)
#define free(M) HeapFree(GetProcessHeap(), NULL, M)

struct _args {
	BOOL NoClobber;
	BOOL Force;
} Args;

typedef struct _tree {
	_tree* next;
	int value;
} Tree, *PTree;

void FreeTree(PTree tree)
{
	PTree cur = tree;
	while (cur != NULL)
	{
		cur = tree->next;
		free(tree);
	}
}

int parse(LPWSTR* argv, int argc, PTree cur)
{
	RtlSecureZeroMemory(&Args, sizeof Args);
	RtlSecureZeroMemory(cur, sizeof Tree);

	for (int i = 1; i < argc; i++)
	{
		if (CMP(argv[i], "-n", "--no-clobber")) Args.NoClobber = TRUE;
		else if (CMP(argv[i], "-f", "--force")) Args.Force = TRUE;
		else 
		{
			cur->value = i;
			cur->next = (PTree)alloc(sizeof Tree);

			if (cur->next == NULL) 
				return 3;

			cur = cur->next;
			RtlSecureZeroMemory(cur, sizeof Tree);
		}
	}

	if (Args.NoClobber && Args.Force) return 2;

	return 0;
}

#ifdef _DEBUG
int main()
#else
int entry()
#endif
{
	int err = 0;
	int argc = 0;
	int code = 0;
	Tree tree;

	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	if (argc < 3)
	{
		code = 1;
		goto end;
	}

	if (parse(argv, argc, &tree))
	{
		code = 1;
		goto end;
	}

	if (!CopyFileW(argv[1], argv[2], Args.NoClobber)) err = GetLastError();

	switch (err) {
	case ERROR_ACCESS_DENIED:
		if (Args.Force && DeleteFileW(argv[2]))
		{
			if (!CopyFileW(argv[1], argv[2], Args.NoClobber)) code = 2;
		}
	default:
		code = 2;
	}

end:;
	FreeTree(tree.next);
	return code;
}
