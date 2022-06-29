#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <vector>

void patchEX(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess) {
	DWORD oldprotect;
	VirtualProtectEx(hProcess, dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	WriteProcessMemory(hProcess, dst, src, size, nullptr);
	VirtualProtectEx(hProcess, dst, size, oldprotect, &oldprotect);
}

DWORD GetProcID(const wchar_t* name)
{
	DWORD process_id = 0;
	auto hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, process_id);
	if (hSnap != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);
		if (Process32First(hSnap, &procEntry)) {
			do
			{
				if (!_wcsicmp(procEntry.szExeFile, name)) {
					process_id = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));
		}
	}
	CloseHandle(hSnap);
	return process_id;
}

uintptr_t GetModuleBaseAddress(DWORD procID, const wchar_t* name)
{
	uintptr_t modBaseAddr = 0;
	auto hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procID);
	if (hSnap != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry)) {
			do
			{
				if (!_wcsicmp(modEntry.szModule, name)) {
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

template <class T>
T ReadMem(HANDLE handle, uintptr_t dst) {
	T rpmBuffer;
	ReadProcessMemory(handle, (PVOID)dst, &rpmBuffer, sizeof(T), 0);
	return rpmBuffer;
}

template <class T>
void WriteMem(HANDLE handle, uintptr_t dst, T val) {
	WriteProcessMemory(handle, (PVOID)dst, &val, sizeof(T), 0);
}

uintptr_t FindDMAAddy(HANDLE handle, uintptr_t ptr, std::vector<unsigned int> offsets) {
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); i++) {
		addr = ReadMem<uintptr_t>(handle, addr);
		addr += offsets[i];
	}
	return addr;
}
