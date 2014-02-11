#include <windows.h>
#include <sstream>
#include "..\hook\hook.h"

using namespace std;

int CALLBACK WinMain(
	_In_  HINSTANCE hInstance,
	_In_  HINSTANCE hPrevInstance,
	_In_  LPSTR lpCmdLine,
	_In_  int nCmdShow
	)
{
	auto res = !UnmapDll();
	wstringstream wss;
	wss << "vmware_scroll_stop result: " << res;
	MessageBox(NULL, wss.str().data(), L"", MB_OK | MB_ICONASTERISK);
	return res;
}