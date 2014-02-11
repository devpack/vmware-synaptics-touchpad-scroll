#pragma once
#ifdef HOOK_EXPORTS
#define DLL_ENTRY __declspec(dllexport)
#else 
#define DLL_ENTRY __declspec(dllexport)
#endif

#include <windows.h>

extern int DLL_ENTRY g_bSubclassed;
DLL_ENTRY int InjectDll(HWND hWnd);
DLL_ENTRY int UnmapDll();