#pragma once
#include <Windows.h>
#include <iostream>

uintptr_t FindPattern(HMODULE module, const unsigned char* pattern, const char* mask);
uintptr_t FindPatternX(uintptr_t start, size_t length, const unsigned char* pattern, const char* mask);