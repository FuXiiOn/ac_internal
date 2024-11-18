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

#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a, b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define DEFINE_MEMBER_N(type, name, offset) struct {unsigned char MAKE_PAD(offset); type name;}

typedef enum {
	SDL_GRAB_QUERY,
	SDL_GRAB_OFF,
	SDL_GRAB_ON
} SDL_GrabMode;
typedef BOOL(__cdecl* t_SDL_WM_GrabInput)(SDL_GrabMode mode);
t_SDL_WM_GrabInput	_SDL_WM_GrabInput;

struct Vector3 { float x, y, z; };

class ent
{
public:
	union {
		DEFINE_MEMBER_N(Vector3, headpos, 0x4);
		DEFINE_MEMBER_N(Vector3, velocity, 0x0010);
		DEFINE_MEMBER_N(int, health, 0xF8);
		DEFINE_MEMBER_N(int, armor, 0xFC);
		DEFINE_MEMBER_N(Vector3, playerpos, 0x0034);
	};
};

class entList
{
public:
	union {
		DEFINE_MEMBER_N(ent*, ent1, 0x0004);
		DEFINE_MEMBER_N(ent*, ent2, 0x0008);
		DEFINE_MEMBER_N(ent*, ent3, 0x000C);
		DEFINE_MEMBER_N(ent*, ent4, 0x0010);
		DEFINE_MEMBER_N(ent*, ent5, 0x0014);
		DEFINE_MEMBER_N(ent*, ent6, 0x0018);
		DEFINE_MEMBER_N(ent*, ent7, 0x001C);
		DEFINE_MEMBER_N(ent*, ent8, 0x0020);
		DEFINE_MEMBER_N(ent*, ent9, 0x0024);
		DEFINE_MEMBER_N(ent*, ent10, 0x0028);
		DEFINE_MEMBER_N(ent*, ent11, 0x002C);
		DEFINE_MEMBER_N(ent*, ent12, 0x0030);
		DEFINE_MEMBER_N(ent*, ent13, 0x0034);
		DEFINE_MEMBER_N(ent*, ent14, 0x0038);
		DEFINE_MEMBER_N(ent*, ent15, 0x003C);
		DEFINE_MEMBER_N(ent*, ent16, 0x0040);
		DEFINE_MEMBER_N(ent*, ent17, 0x0044);
		DEFINE_MEMBER_N(ent*, ent18, 0x0048);
		DEFINE_MEMBER_N(ent*, ent19, 0x004C);
		DEFINE_MEMBER_N(ent*, ent20, 0x0050);
		DEFINE_MEMBER_N(ent*, ent21, 0x0054);
		DEFINE_MEMBER_N(ent*, ent22, 0x0058);
		DEFINE_MEMBER_N(ent*, ent23, 0x005C);
		DEFINE_MEMBER_N(ent*, ent24, 0x0060);
		DEFINE_MEMBER_N(ent*, ent25, 0x0064);
		DEFINE_MEMBER_N(ent*, ent26, 0x0068);
		DEFINE_MEMBER_N(ent*, ent27, 0x006C);
		DEFINE_MEMBER_N(ent*, ent28, 0x0070);
		DEFINE_MEMBER_N(ent*, ent29, 0x0074);
		DEFINE_MEMBER_N(ent*, ent30, 0x0078);
		DEFINE_MEMBER_N(ent*, ent31, 0x007C);
	};
};

DWORD jmpBackAddy;
void __declspec(naked)myFunct() {
	__asm {
		mov al, 1
		mov[edi + 69], al
		jmp[jmpBackAddy]
	}
}

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

		ImGui::Begin("ghostface - Assault Cube Internal", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse);
		if (ImGui::BeginTabBar("main cheat")) {
			if (ImGui::BeginTabItem("Aimbot")) {
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
				ImGui::Checkbox("Inf Ammo", &Config::bAmmo);
				ImGui::Checkbox("No Recoil", &Config::bRecoil);
				ImGui::Checkbox("Fly Hack", &Config::bFly);
				ImGui::Checkbox("Rapid Fire", &Config::bRapidFire);
				ImGui::Checkbox("OneTap Exploit", &Config::bOneHit);
				ImGui::SameLine(); IconHelpMarker("[-] Also applies to bots");
				if (ImGui::Button("Save current coordinates")) {
					Config::savedXpos = localPlayer->playerpos.x;
					Config::savedYpos = localPlayer->playerpos.y;
					Config::savedZpos = localPlayer->playerpos.z;
				}
				if (ImGui::Button("Teleport to saved coordinates")) {
					if (Config::savedXpos != NULL && Config::savedYpos != NULL && Config::savedZpos != NULL) {
						mem::Patch((BYTE*)(&(localPlayer->playerpos.x)), (BYTE*)(&(Config::savedXpos)), 4);
						mem::Patch((BYTE*)(&(localPlayer->playerpos.y)), (BYTE*)(&(Config::savedYpos)), 4);
						mem::Patch((BYTE*)(&(localPlayer->playerpos.z)), (BYTE*)(&(Config::savedZpos)), 4);
					}
				}
				ImGui::Text("CURRENT COORDS - X: %.2f Y: %.2f Z: %.2f", localPlayer->playerpos.x, localPlayer->playerpos.y, localPlayer->playerpos.z);
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

	ent* localPlayer = *(ent**)(moduleBase + 0x10F4F4);

	entList* entityList = *(entList**)(moduleBase + 0x010F4F8);

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

	while (true) {

		if (GetAsyncKeyState(VK_END) & 1)
		{
			break;
		}

		if (Config::bRecoil)
		{
			mem::Nop((BYTE*)(moduleBase + 0x63786), 10);
		}
		else {
			mem::Patch((BYTE*)(moduleBase + 0x63786), (BYTE*)"\x50\x8D\x4C\x24\x1C\x51\x8B\xCE\xFF\xD2", 10);
		}

		DWORD hookAddress = (moduleBase + 0x5AC24);
		int hookLength = 5;
		jmpBackAddy = hookAddress + hookLength;

		if (Config::bFly) {
			mem::Hook((void*)hookAddress, myFunct, hookLength);
		}
		else {
			mem::Patch((BYTE*)(moduleBase + 0x5AC24), (BYTE*)"\x33\xC0", 2);
			mem::Patch((BYTE*)(moduleBase + 0x5AC26), (BYTE*)"\x88\x47\x69", 3);
		}

		if (Config::bRapidFire) {
			mem::Nop((BYTE*)(moduleBase + 0x637E4), 2);
		}
		else {
			mem::Patch((BYTE*)(moduleBase + 0x637E4), (BYTE*)"\x89\x0A", 2);
		}

		if (Config::bOneHit) {
			mem::Patch((BYTE*)(moduleBase + 0x29D1F), (BYTE*)"\x83\x6B\x04\x64", 4);
			mem::Patch((BYTE*)(moduleBase + 0x29D23), (BYTE*)("\x90"), 1);
		}
		else {
			mem::Patch((BYTE*)(moduleBase + 0x29D1F), (BYTE*)"\x29\x7B\x04", 3);
		}

		if (localPlayer) {
			if (Config::bHealth) {
				localPlayer->health = rand() % 899 + 100;
			}

			if (Config::bAmmo) {
				*(int*)mem::FindDMAAddy(moduleBase + 0x10F4F4, { 0x374, 0x14, 0x0 }) = 999;
			}
		}
	}

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
