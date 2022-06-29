#include <windows.h>
#include <iostream>
#include "HandleGetter.h"
#include "Display.h"

HWND hwndBoshy = NULL;
BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
{
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	if (lpdwProcessId == lParam)
	{
		hwndBoshy = hwnd;
		return FALSE;
	}
	return TRUE;
}

int main()
{
	DWORD OFFSET = 0x48195;
	DWORD procID = GetProcID(L"I Wanna Be The Boshy.exe");
	uintptr_t modBase = GetModuleBaseAddress(procID, L"I Wanna Be The Boshy.exe");
	HANDLE hProcess = 0;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procID);
	std::cout << std::hex << "ModuleBase = " << modBase + OFFSET << "\n" << "procID = " << procID << "\n";
	int hookLength = 8;

	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0;
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;
	ip.ki.wVk = 0x58;
	EnumWindows(EnumWindowsProcMy, procID);
	uintptr_t fps_addr = FindDMAAddy(hProcess, modBase + 0x59A94, { 0x78 });
	std::cout << std::dec;
	bool auto_shoot = false;
	bool GOD_MOD = false;
	int curFPS = ReadMem<int>(hProcess, fps_addr);

	display(auto_shoot, GOD_MOD, curFPS);
	while (true) {
		if (GetForegroundWindow() == hwndBoshy) {
			if (auto_shoot) {
				ip.ki.dwFlags = 0;
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(10);
				ip.ki.dwFlags = KEYEVENTF_KEYUP; 
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(10);
			}
			if (GetAsyncKeyState(0x43)) {
				auto_shoot = !auto_shoot;
				display(auto_shoot, GOD_MOD, curFPS);
				Sleep(100);
			}
			if (GetAsyncKeyState(0x41)) {
				GOD_MOD = !GOD_MOD;
				display(auto_shoot, GOD_MOD, curFPS);
				if (GOD_MOD) { //0xE9
					patchEX((BYTE*)(modBase + OFFSET), (BYTE*)"\x39\xE4\x89\x88\x1C\x01\x00\x00", hookLength, hProcess);
				}
				else {
					patchEX((BYTE*)(modBase + OFFSET), (BYTE*)"\x39\xDD\x89\x88\x1C\x01\x00\x00", hookLength, hProcess);
				}
				Sleep(100);
			}
			if (GetAsyncKeyState(VK_OEM_COMMA)) {
				if ((curFPS - 5) >= 10) {
					curFPS -= 5;
					display(auto_shoot, GOD_MOD, curFPS);
					Sleep(200);
				}
			}
			if (GetAsyncKeyState(VK_OEM_PERIOD)) {
				if ((curFPS + 5) < 130) {
					curFPS += 5;
					display(auto_shoot, GOD_MOD, curFPS);
					Sleep(200);
				}
			}
			if (GetAsyncKeyState(VK_END))
			{
				patchEX((BYTE*)(modBase + OFFSET), (BYTE*)"\x39\xDD\x89\x88\x1C\x01\x00\x00", hookLength, hProcess);
				break;
			}
			WriteMem<int>(hProcess, fps_addr, curFPS);
		}
	}
	return 0;
}