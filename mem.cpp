#include "stdafx.h"
#include "mem.h"

bool mem::Hook(void* toHook, void* funct, int len) {
	if (len < 5) {
		return false;
	}

	DWORD oldProtection;
	VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &oldProtection);
	
	memset(toHook, 0x90, len);
	
	DWORD relativeAddress = ((DWORD)funct - (DWORD)toHook) - 5;

	*(BYTE*)toHook = 0xE9;
	*(DWORD*)((DWORD)toHook + 1) = relativeAddress;
	 
	DWORD temp;
	VirtualProtect(toHook, len, oldProtection, &temp);

	return true;
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