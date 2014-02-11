#include <windows.h>
#include <sstream>
#include "..\hook\hook.h"

using namespace std;

WNDENUMPROC playerScroll (HWND h, LPARAM p) {
		*((HWND *)p) = h;
		wchar_t c[] = L"VMWindow";
		const int c_len = sizeof(c) / sizeof(*c);
		wchar_t buf[c_len];
		GetClassName(h, buf, c_len);
		return (WNDENUMPROC)lstrcmp(buf, c);
};

WNDENUMPROC workstationScroll (HWND h, LPARAM p) {
		*((HWND *)p) = h;
		wchar_t c[] = L"VMUIView";//L"VMUIView";//L"MKSEmbedded";//L"VMware.GuestWindow";		
		const int c_len = sizeof(c) / sizeof(*c);
		wchar_t buf[c_len];
		GetClassName(h, buf, c_len);
		return (WNDENUMPROC)lstrcmp(buf, c);
};

int CALLBACK WinMain(
	_In_  HINSTANCE hInstance,
	_In_  HINSTANCE hPrevInstance,
	_In_  LPSTR lpCmdLine,
	_In_  int nCmdShow
	) {

	LPWSTR *szArgList;
    int argCount;
  
    szArgList = CommandLineToArgvW(GetCommandLine(), &argCount);

	HWND phwnd = NULL;
	HWND hwnd = NULL;

	if (szArgList != NULL && argCount == 2) {
		if(lstrcmp(szArgList[1], L"0") == 0) {
			HWND phwnd = FindWindowEx(NULL, NULL, L"VMPlayerFrame", NULL);	
			EnumChildWindows(phwnd, (WNDENUMPROC) playerScroll, (LPARAM) &hwnd);
			MessageBox(NULL, L"Enable Synaptics touchpad scroll for Vmware Player", L"", MB_OK | MB_ICONASTERISK);
		}
		else {
			HWND phwnd = FindWindowEx(NULL, NULL, L"VMUIFrame", NULL);	
			EnumChildWindows(phwnd, (WNDENUMPROC) workstationScroll, (LPARAM) &hwnd);
			MessageBox(NULL, L"Enable Synaptics touchpad scroll for Vmware Workstation", L"", MB_OK | MB_ICONASTERISK);
		}
	}
	else {
			HWND phwnd = FindWindowEx(NULL, NULL, L"VMUIFrame", NULL);	
			EnumChildWindows(phwnd, (WNDENUMPROC) workstationScroll, (LPARAM) &hwnd);
			MessageBox(NULL, L"Enable Synaptics touchpad scroll for Vmware Workstation", L"", MB_OK | MB_ICONASTERISK);
	}
 
    LocalFree(szArgList);

	auto res = hwnd ? !InjectDll(hwnd) : 2;
	wstringstream wss;
	wss << "vmware_scroll_start result: " << res;
	MessageBox(NULL, wss.str().data(), L"", MB_OK | MB_ICONASTERISK);

	return res;
}