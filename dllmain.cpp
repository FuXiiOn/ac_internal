#include "stdafx.h"
#include <iostream>
#include <gl/GL.h>
#include "glText.h"
#include "glDraw.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui_impl_win32.h"
#include "mem.h"
#include <string>
#include <thread>
#include "hooks.h"
#include "config.h";
#include "detours.h"
#include "geom.h"
#include <algorithm>
#include "offsets.h"

typedef enum {
	SDL_GRAB_QUERY,
	SDL_GRAB_OFF,
	SDL_GRAB_ON
} SDL_GrabMode;
typedef BOOL(__cdecl* t_SDL_WM_GrabInput)(SDL_GrabMode mode);
t_SDL_WM_GrabInput	_SDL_WM_GrabInput;

typedef int(__cdecl* _printIngame)(const char* format, ...);
_printIngame printIngame = (_printIngame)(0x4090f0);

typedef ent* (__cdecl* _GetCrosshairEnt)();
_GetCrosshairEnt getCrosshairEnt = nullptr;

typedef BOOL(__stdcall* t_wglSwapBuffers)(HDC hDc);
t_wglSwapBuffers gateway_wglSwapBuffers;

BOOL __stdcall hooked_wglSwapBuffers(HDC hDc) {
	uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");
	moduleBase = (uintptr_t)GetModuleHandle(NULL);

	ent* localPlayer = *(ent**)(moduleBase + 0x10F4F4);

	entList* entityList = *(entList**)(moduleBase + 0x010F4F8);

	if (GetAsyncKeyState(VK_END) & 1)
	{
		ImGui_ImplOpenGL2_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (GetAsyncKeyState(VK_INSERT) & 1) {
		Config::bMenu = !Config::bMenu;
	}

	if (Config::bMenu) {
		_SDL_WM_GrabInput(SDL_GrabMode(2));

		ImVec2 cheatSize = {350, 300};

		ImGui::SetNextWindowSize(cheatSize);

		ImGui::Begin("Venom - Assault Cube Internal", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse);
		if (ImGui::BeginTabBar("main cheat")) {
			if (ImGui::BeginTabItem("Aimbot")) {
				ImGui::Checkbox("Triggerbot", &Config::bTriggerbot);
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "soon :)");
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("ESP")) {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "soon :)");
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Exploits"))
			{
				ImGui::Checkbox("GodMode", &Config::bHealth);
				ImGui::Checkbox("No Recoil", &Config::bRecoil);
				ImGui::Checkbox("Inf Ammo", &Config::bAmmo);
				ImGui::Checkbox("Fly Hack", &Config::bFly);
				ImGui::Checkbox("Rapid Fire", &Config::bRapidFire);
				ImGui::Checkbox("OneTap Exploit", &Config::bOneHit);
				if (ImGui::Button("Save current coordinates")) {
					Config::savedXpos = localPlayer->bodypos.x;
					Config::savedYpos = localPlayer->bodypos.y;
					Config::savedZpos = localPlayer->bodypos.z;
				}
				if (ImGui::Button("Teleport to saved coordinates")) {
					if (Config::savedXpos != NULL && Config::savedYpos != NULL) {
						mem::Patch((BYTE*)(&(localPlayer->bodypos.x)), (BYTE*)(&(Config::savedXpos)), 4);
						mem::Patch((BYTE*)(&(localPlayer->bodypos.y)), (BYTE*)(&(Config::savedYpos)), 4);
						mem::Patch((BYTE*)(&(localPlayer->bodypos.z)), (BYTE*)(&(Config::savedZpos)), 4);
					}
				}
				ImGui::Text("CURRENT COORDS - X: %.2f Y: %.2f Z: %.2f", localPlayer->bodypos.x, localPlayer->bodypos.y, localPlayer->bodypos.z);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
	}
	else {
		_SDL_WM_GrabInput(SDL_GrabMode(1));
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	return gateway_wglSwapBuffers(hDc);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

WNDPROC	oWndProc;
LRESULT __stdcall WndProc(const HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (!Config::bMenu) {
		return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
	}
	else {
		ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
		return true;
	}
}

DWORD WINAPI HackThread(HMODULE hModule) {
	uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");
	moduleBase = (uintptr_t)GetModuleHandle(NULL);

	getCrosshairEnt = (_GetCrosshairEnt)(moduleBase + 0x607C0);

	ent* localPlayer = *(ent**)(moduleBase + 0x10F4F4);

	entList* entityList = *(entList**)(moduleBase + 0x010F4F8);

	bool recoilPatched = false;
	bool flyPatched = false;
	bool rapidPatched = false;
	bool onehitPatched = false;

	char windowTitle[] = "AssaultCube";
	HWND hwnd = FindWindowA(NULL, windowTitle);

	if (!hwnd)
	{
		char message[256];
		sprintf_s(message, 256,
			"Venom was not able to retrieve the window handle for \"%s\". Ensure you injected the DLL into the right process.\nIf this error persists, please contact the developer on Github.",
			windowTitle);
		MessageBoxA(NULL, message, "Unable to retrieve the window handle", MB_OK | MB_ICONERROR);
		FreeLibraryAndExitThread(hModule, EXIT_FAILURE);
	}

	oWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)WndProc);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplOpenGL2_Init();

	_SDL_WM_GrabInput = (t_SDL_WM_GrabInput)DetourFindFunction("SDL.dll", "SDL_WM_GrabInput");
	gateway_wglSwapBuffers = (t_wglSwapBuffers)DetourFindFunction("opengl32.dll", "wglSwapBuffers");
	if (gateway_wglSwapBuffers == NULL)
	{
		MessageBoxA(NULL, "Venom was not able to hook the function \"wglSwapBuffers\".\nPlease contact the developer on Github.",
			"Unable to hook \"wglSwapBuffers\"", MB_OK | MB_ICONERROR);
		FreeLibraryAndExitThread(hModule, EXIT_FAILURE);
	}
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)gateway_wglSwapBuffers, hooked_wglSwapBuffers);
	DetourTransactionCommit();

	printIngame("\f8Venom injected!");

	while (true) {

		if (GetAsyncKeyState(VK_END) & 1)
		{
			break;
		}

		if (Config::bRecoil && !recoilPatched) {
			mem::Patch((BYTE*)(0x462020), (BYTE*)"\xC2\x08\x00", 3);
			recoilPatched = true;
		}
		else if(!Config::bRecoil && recoilPatched) {
			mem::Patch((BYTE*)(0x462020), (BYTE*)"\x55\x8B\xEC", 3);
			recoilPatched = false;
		}

		if (Config::bFly && !flyPatched) {
			*(int*)mem::FindDMAAddy((0x50F4F4), { 0x338 }) = 5;
			flyPatched = true;
		}
		else if(!Config::bFly && flyPatched) {
			*(int*)mem::FindDMAAddy((0x50F4F4), { 0x338 }) = 0;
			flyPatched = false;
		}
	
		if (Config::bRapidFire && !rapidPatched) {
			mem::Nop((BYTE*)(moduleBase + 0x637E4), 2);
			rapidPatched = true;
		}
		else if(!Config::bRapidFire && rapidPatched) {
			mem::Patch((BYTE*)(moduleBase + 0x637E4), (BYTE*)"\x89\x0A", 2);
			rapidPatched = false;
		}

		if (Config::bOneHit) {
			*(int*)mem::FindDMAAddy(0x50F4F4, { 0x374, 0xC, 0x10C }) = 999;
			onehitPatched = true;
		}
		else if(!Config::bOneHit && onehitPatched) {
			char weaponIdentifier[5];
			uintptr_t identifierAddr = mem::FindDMAAddy(0x50F4F4, { 0x374, 0xC, 0x0 });

			memcpy(weaponIdentifier, (char*)identifierAddr, 4);
			weaponIdentifier[4] = '\0';

			if (strcmp(weaponIdentifier, "assa") == 0) {
				*(int*)mem::FindDMAAddy(0x50F4F4, { 0x374, 0xC, 0x10C }) = 22;
			}
			else if (strcmp(weaponIdentifier, "pist") == 0) {
				*(int*)mem::FindDMAAddy(0x50F4F4, { 0x374, 0xC, 0x10C }) = 18;
			}
		}

		if (localPlayer) {
			if (Config::bHealth) {
				localPlayer->health = rand() % 899 + 100;
			}

			if (Config::bAmmo) {
				localPlayer->currWeapon->ammoPtr->ammoClip = 999;
			}

			if (Config::bTriggerbot) {
				ent* crosshairEnt = getCrosshairEnt();

				if (crosshairEnt) {
					if (localPlayer->team != crosshairEnt->team) {
						localPlayer->bAttack = 1;
					}
				}
				else {
					localPlayer->bAttack = 0;
				}
				if (GetAsyncKeyState(VK_LBUTTON)) {
					localPlayer->bAttack = 1;
				}
			}
		}
	}

		printIngame("\f8Venom uninjected!");

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)gateway_wglSwapBuffers, hooked_wglSwapBuffers);
		DetourTransactionCommit();

		ImGui_ImplOpenGL2_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, NULL);
		if (hThread)
			CloseHandle(hThread);
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
