#pragma once
#include "stdafx.h"
#include <windows.h>
#include <vector>

namespace mem
{
	void Patch(BYTE* dst, BYTE* src, unsigned int size);
	void Nop(BYTE* dst, unsigned int size);
	bool Hook(void* toHook, void* funct, int len);
	uintptr_t FindPattern(HMODULE module, const unsigned char* pattern, const char* mask);
	uintptr_t FindPatternX(uintptr_t start, size_t length, const unsigned char* pattern, const char* mask);
	uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets);
}