#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <limits>
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
	DWORD OFFSET_GODMOD = 0x48195;
	DWORD OFFSET_CHARACTER = 0x4914;
	int hookLength = 8;
	DWORD procID = GetProcID(L"I Wanna Be The Boshy.exe");
	uintptr_t modBase = GetModuleBaseAddress(procID, L"I Wanna Be The Boshy.exe");
	HANDLE hProcess = 0;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procID);

	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0;
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;
	ip.ki.wVk = 0x58;
	EnumWindows(EnumWindowsProcMy, procID);
	uintptr_t fps_addr = FindDMAAddy(hProcess, modBase + 0x59A94, { 0x78 });
	uintptr_t character_addr = FindDMAAddy(hProcess, modBase + 0x59A9C, { 0x98,0x500,0x234,0x8,0x8,0x878,0x58 });
	std::cout << std::dec;
	bool auto_shoot = false;
	bool GOD_MOD = false;
	bool need_restart = false;
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
			if (need_restart) {
				need_restart = false;
				ip.ki.wVk = 0x52;
				ip.ki.dwFlags = 0;
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(10);
				ip.ki.dwFlags = KEYEVENTF_KEYUP;
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(10);
				ip.ki.wVk = 0x58;
				
			}
			if (GetAsyncKeyState(0x43))//"C" 
			{
				auto_shoot = !auto_shoot;
				display(auto_shoot, GOD_MOD, curFPS);
				Sleep(100);
			}
			if (GetAsyncKeyState(0x41))//"A"
			{
				GOD_MOD = !GOD_MOD;
				display(auto_shoot, GOD_MOD, curFPS);
				if (GOD_MOD) { //0xE9
					patchEX((BYTE*)(modBase + OFFSET_GODMOD), (BYTE*)"\x39\xE4\x89\x88\x1C\x01\x00\x00", hookLength, hProcess);
				}
				else {
					patchEX((BYTE*)(modBase + OFFSET_GODMOD), (BYTE*)"\x39\xDD\x89\x88\x1C\x01\x00\x00", hookLength, hProcess);
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
			WriteMem<int>(hProcess, fps_addr, curFPS);
		}
		if (GetAsyncKeyState(0x53)) //"S"
		{
			patchEX((BYTE*)(modBase + OFFSET_CHARACTER), (BYTE*)"\x90\x90\x90", 3, hProcess);
			std::cout << "\nEnter character number(If you want to be able to change character ingame, type \"b\"): ";
			unsigned character_id;
			std::cin >> character_id;
			if (std::cin.fail()) {
				std::cin.clear();
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				patchEX((BYTE*)(modBase + OFFSET_CHARACTER), (BYTE*)"\x89\x56\x08", 3, hProcess);
			}
			else {
				if (character_id >= 1 && character_id <= 36) {
					need_restart = true;
					WriteMem<int>(hProcess, character_addr, character_id * 4 + 8);
					Sleep(100);
				}
				
			}
			Sleep(100);
		}
		if (GetAsyncKeyState(VK_END))
		{
			patchEX((BYTE*)(modBase + OFFSET_GODMOD), (BYTE*)"\x39\xDD\x89\x88\x1C\x01\x00\x00", hookLength, hProcess);
			patchEX((BYTE*)(modBase + OFFSET_CHARACTER), (BYTE*)"\x89\x56\x08", 3, hProcess);
			break;
		}
	}
	return 0;
}