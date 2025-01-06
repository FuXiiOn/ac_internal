#include "stdafx.h"
#include "mem.h"
#include <Windows.h>
#include <Psapi.h>

bool mem::Hook(void* toHook, void* funct, int len) {
	if (len < 5) return false;

	DWORD oldProtection;
	VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &oldProtection);
	
	DWORD relativeAddress = ((DWORD)funct - (DWORD)toHook) - 5;

	*(BYTE*)toHook = 0xE9;
	*(DWORD*)((DWORD)toHook + 1) = relativeAddress;
	 
	VirtualProtect(toHook, len, oldProtection, &oldProtection);

	return true;
}

BYTE* mem::TrampHook(BYTE* src, BYTE* dst, size_t length) {
	if (length < 5) return 0;

	BYTE* gateway = (BYTE*)VirtualAlloc(0, length, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	memcpy_s(gateway, length, src, length);

	uintptr_t gatewayRelativeAddress = src - gateway - 5;

	*(gateway + length) = 0xE9;

	*(uintptr_t*)((uintptr_t)gateway + length + 1) = gatewayRelativeAddress;

	mem::Hook(src, dst, length);

	return gateway;
}

void mem::Patch(BYTE* dst, BYTE* src, unsigned int size)
{
	DWORD oldprotect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);

	memcpy(dst, src, size);
	VirtualProtect(dst, size, oldprotect, &oldprotect);
}


void mem::Nop(BYTE* dst, unsigned int size)
{
	DWORD oldprotect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	memset(dst, 0x90, size);
	VirtualProtect(dst, size, oldprotect, &oldprotect);
}

uintptr_t mem::FindPattern(HMODULE module, const unsigned char* pattern, const char* mask)
{
	MODULEINFO info = { };
	GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(MODULEINFO));

	return FindPatternX(reinterpret_cast<uintptr_t>(module), info.SizeOfImage, pattern, mask);
}

uintptr_t mem::FindPatternX(uintptr_t start, size_t length, const unsigned char* pattern, const char* mask) {
	size_t pos = 0;
	int maskLength = strlen(mask) - 1;

	for (auto i = start; i < start + length; i++) {
		if (*reinterpret_cast<unsigned char*>(i) == pattern[pos] || mask[pos] == '?') {
			if (mask[pos + 1] == '\0') {
				return i - maskLength;
			}

			pos++;
		}
		else {
			pos = 0;
		}
	}

	return -1;
}

uintptr_t mem::FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		addr = *(uintptr_t*)addr;
		addr += offsets[i];
	}
	return addr;
}