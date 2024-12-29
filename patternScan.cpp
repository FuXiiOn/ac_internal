#include "patternScan.h"
#include <Windows.h>
#include <iostream>
#include <Psapi.h>

uintptr_t FindPattern(HMODULE module, const unsigned char* pattern, const char* mask)
{
	MODULEINFO info = { };
	GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(MODULEINFO));

	return FindPatternX(reinterpret_cast<uintptr_t>(module), info.SizeOfImage, pattern, mask);
}

uintptr_t FindPatternX(uintptr_t start, size_t length, const unsigned char* pattern, const char* mask)
{
	size_t pos = 0;
	int maskLength = strlen(mask) - 1;

	for (auto i = start; i < start + length; i++)
	{
		if (*reinterpret_cast<unsigned char*>(i) == pattern[pos] || mask[pos] == '?')
		{
			if (mask[pos + 1] == '\0')
			{
				return i - maskLength;
			}

			pos++;
		}
		else
		{
			pos = 0;
		}
	}

	return -1;
}