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
	DWORD OFFSET_CHARACTER = 0x4911;
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
	uintptr_t character_addr = FindDMAAddy(hProcess, modBase + 0x59AB8, { 0x0,0x1C4,0x420,0x30,0x84,0x470,0x58 });
	std::cout << std::dec;
	bool auto_shoot = false;
	bool GOD_MOD = false;
	bool need_restart = false;
	bool patched = false;
	int curFPS = ReadMem<int>(hProcess, fps_addr);

	 int character_id = 12;
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
			if (GetAsyncKeyState(0x57))//"W" 
			{
				if (patched) {
					patchEX((BYTE*)(modBase + OFFSET_CHARACTER), (BYTE*)"\x8B\x51\x08\x89\x56\x08", 6, hProcess);
					patched = false;
					}
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
		if (GetAsyncKeyState(0x44)) //"D"
		{
			if (!patched){
				patchEX((BYTE*)(modBase + OFFSET_CHARACTER), (BYTE*)"\x90\x90\x90\x90\x90\x90", 6, hProcess);
			patched = true;
		}
			std::cout << "\nEnter character number: ";
			 int temp_char_id;
			std::cin >> temp_char_id;
			if (std::cin.fail()) {
				std::cin.clear();
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			}
			else
			{
				if (temp_char_id >= 1 && temp_char_id <= 36) {
					need_restart = true;
					character_id = temp_char_id*4+8;
					std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
					Sleep(100);
				}
			}
		}
		if (GetAsyncKeyState(0x45)) //"E"
		{
			 character_addr = FindDMAAddy(hProcess, modBase + 0x59AB8, { 0x0,0x1C4,0x420,0x30,0x84,0x470,0x58 });
			 Sleep(100);
		}
		if (patched) WriteMem<int>(hProcess, character_addr, character_id);

		if (GetAsyncKeyState(VK_END))
		{
			patchEX((BYTE*)(modBase + OFFSET_GODMOD), (BYTE*)"\x39\xDD\x89\x88\x1C\x01\x00\x00", hookLength, hProcess);
			patchEX((BYTE*)(modBase + OFFSET_CHARACTER), (BYTE*)"\x8B\x51\x08\x89\x56\x08", 6, hProcess);
			break;
		}
		Sleep(10);
	}
	return 0;
}