#include <windows.h>
#include "hook.h"

//-------------------------------------------------------------
// shared data 
// Notice:	seen by both: the instance of "hook.dll" mapped
//			into thread of interest as well as by the instance
//			of "HookInjEx.dll" mapped into our control process
#pragma data_seg (".shared")
int		g_bSubclassed = 0;	// START button subclassed?
UINT	WM_HOOKEX = 0;
HWND	g_hWnd = 0;		// handle of START button
HHOOK	g_hHook = 0;
#pragma data_seg ()

#pragma comment(linker,"/SECTION:.shared,RWS")


//-------------------------------------------------------------
// global variables (unshared!)
//
HINSTANCE	hDll;

// New & old window procedure of the subclassed window
WNDPROC				OldProc = NULL;
LRESULT CALLBACK	NewProc(HWND, UINT, WPARAM, LPARAM);

bool artificial_scroll = false;

INPUT inp;

//-------------------------------------------------------------
// DllMain
//
BOOL APIENTRY DllMain(HINSTANCE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		hDll = hModule;
		DisableThreadLibraryCalls(hDll);

		if (WM_HOOKEX == NULL)
			WM_HOOKEX = RegisterWindowMessage(L"WM_HOOKEX_RK");

		ZeroMemory(&inp, sizeof(INPUT));
		inp.type = INPUT_MOUSE;
	}

	return TRUE;
}


//-------------------------------------------------------------
// HookProc
// Notice:
// - executed by the instance of "hook.dll" mapped into thread;
//
// When called from InjectDll:
//	  -	sublasses the start button;
//	  -	removes the hook, but the DLL stays in the remote process
//		though, because we increased its reference count via LoadLibray
//		(this way we disturb the target process as litle as possible);
//
// When called from UnmapDll:
//	  -	restores the old window procedure for the start button;
//	  - reduces the reference count of the DLL (via FreeLibrary);
//	  -	removes the hook, so the DLL is unmapped;
//
//		Also note, that the DLL isn't unmapped immediately after the
//		call to UnhookWindowsHookEx, but in the near future
//		(right after finishing with the current message).
//		Actually it's obvious why: windows can NOT unmap the 
//		DLL in the middle of processing a meesage, because the code
//		in the hook procedure is still required. 
//		That's why we can change the order LoadLibrary/FreeLibrary &
//		UnhookWindowsHookEx are called.
//
//		FreeLibrary, in contrast, unmapps the DLL imeditaley if the 
//		reference count reaches zero.
//
#define pCW ((CWPSTRUCT*)lParam)

LRESULT HookProc(
	int code,       // hook code
	WPARAM wParam,  // virtual-key code
	LPARAM lParam   // keystroke-message information
	)
{
	if (pCW->message == WM_HOOKEX) 
		if (pCW->lParam)
		{
			UnhookWindowsHookEx(g_hHook);

			if (g_bSubclassed)
				goto END;		// already subclassed?

			// Let's increase the reference count of the DLL (via LoadLibrary),
			// so it's NOT unmapped once the hook is removed;
			wchar_t lib_name[MAX_PATH];
			GetModuleFileName(hDll, lib_name, MAX_PATH);

			if (!LoadLibrary(lib_name))
				goto END;

			// Subclass START button
			OldProc = (WNDPROC) SetWindowLong(g_hWnd, GWL_WNDPROC, (LONG) NewProc);
			if (OldProc == NULL)			// failed?
				FreeLibrary(hDll);
			else {						// success -> leave "HookInjEx.dll"
				MessageBeep(MB_OK);	// mapped into thread
				g_bSubclassed = true;
			}
		}
		else
		{
			UnhookWindowsHookEx(g_hHook);

			// Failed to restore old window procedure? => Don't unmap the
			// DLL either. Why? Because then thread would call our
			// "unmapped" NewProc and  crash!!
			if (!SetWindowLong(g_hWnd, GWL_WNDPROC, (LONG) OldProc))
				goto END;

			FreeLibrary(hDll);

			MessageBeep(MB_OK);
			g_bSubclassed = false;
		}

END:
	return CallNextHookEx(g_hHook, code, wParam, lParam);
}


//-------------------------------------------------------------
// NewProc
// Notice:	- new window procedure for the window;
//	
LRESULT CALLBACK NewProc(
	HWND hwnd,      // handle to window
	UINT uMsg,      // message identifier
	WPARAM wParam,  // first message parameter
	LPARAM lParam   // second message parameter
	)
{
	DWORD f = 0;
	switch (uMsg)
	{
	case WM_MOUSEHWHEEL:
		f = MOUSEEVENTF_HWHEEL;
		break;
	case WM_MOUSEWHEEL:
		f = MOUSEEVENTF_WHEEL;
		break;
	}
	if (f && InSendMessage()) {
		inp.mi.dwFlags = f;
		inp.mi.mouseData = GET_WHEEL_DELTA_WPARAM(wParam);
		SendInput(1, &inp, sizeof(INPUT));
		return 0;
	}
	return CallWindowProc(OldProc, hwnd, uMsg, wParam, lParam);
}

////-------------------------------------------------------------
//// NewProc
//// Notice:	- new window procedure for the window;
////	
//LRESULT CALLBACK NewProc(
//	HWND hwnd,      // handle to window
//	UINT uMsg,      // message identifier
//	WPARAM wParam,  // first message parameter
//	LPARAM lParam   // second message parameter
//	)
//{
//	DWORD f = 0;
//	switch (uMsg)
//	{
//	case WM_MOUSEHWHEEL:
//		f = MOUSEEVENTF_HWHEEL;
//		break;
//	case WM_MOUSEWHEEL:
//		f = MOUSEEVENTF_WHEEL;
//		break;
//	}
//	if (f) {
//		auto ht = CallWindowProc(OldProc, hwnd, WM_NCHITTEST, wParam, lParam);
//		CallWindowProc(OldProc, hwnd, WM_SETCURSOR, (WPARAM) hwnd, MAKELPARAM(ht, uMsg));
//		return CallWindowProc(OldProc, hwnd, uMsg, wParam, lParam);
//	}
//	return CallWindowProc(OldProc, hwnd, uMsg, wParam, lParam);
//}


//-------------------------------------------------------------
// InjectDll
// Notice: 
//	- injects "hook.dll" into window's thread (via SetWindowsHookEx);
//	- subclasses window (see HookProc for more details);
//
//		Parameters: - hWnd = window handle
//
//		Return value:	1 - success;
//						0 - failure;
//
int InjectDll(HWND hWnd)
{
	g_hWnd = hWnd;

	// Hook process
	g_hHook = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)HookProc,
		hDll, GetWindowThreadProcessId(hWnd, NULL));
	if (g_hHook == NULL)
		return 0;

	// By the time SendMessage returns, 
	// the window has already been subclassed
	SendMessage(hWnd, WM_HOOKEX, 0, 1);

	return g_bSubclassed;
}


//-------------------------------------------------------------
// UnmapDll
// Notice: 
//	- restores the old window procedure for the window;
//	- unmapps the DLL from the remote process
//	  (see HookProc for more details);
//
//		Return value:	1 - success;
//						0 - failure;
//
int UnmapDll()
{
	g_hHook = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)HookProc,
		hDll, GetWindowThreadProcessId(g_hWnd, NULL));

	if (g_hHook == NULL)
		return 0;

	SendMessage(g_hWnd, WM_HOOKEX, 0, 0);

	return (g_bSubclassed == NULL);
}
