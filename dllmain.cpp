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
#include <cmath>
#include <numbers>

typedef enum {
	SDL_GRAB_QUERY,
	SDL_GRAB_OFF,
	SDL_GRAB_ON
} SDL_GrabMode;
typedef BOOL(__cdecl* t_SDL_WM_GrabInput)(SDL_GrabMode mode);
t_SDL_WM_GrabInput	_SDL_WM_GrabInput;

typedef int(__cdecl* _printInGame)(const char* format, ...);
_printInGame printInGame = (_printInGame)(0x4090f0);

typedef ent* (__cdecl* _GetCrosshairEnt)();
_GetCrosshairEnt getCrosshairEnt = nullptr;

typedef BOOL(__stdcall* t_wglSwapBuffers)(HDC hDc);
t_wglSwapBuffers gateway_wglSwapBuffers;

const char* items[] = { "FFA", "TEAM" };
static int triggerbot_selected = 0;
const char* combo_preview_value = items[triggerbot_selected];

BOOL __stdcall hooked_wglSwapBuffers(HDC hDc) {
	uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");
	moduleBase = (uintptr_t)GetModuleHandle(NULL);

	ent* localPlayer = *(ent**)(moduleBase + 0x10F4F4);

	entList* entityList = *(entList**)(moduleBase + 0x10F4F8);

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

		ImVec2 cheatSize = { 350, 300 };

		ImGui::SetNextWindowSize(cheatSize);

		ImGui::Begin("Venom - Assault Cube Internal", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse);
		if (ImGui::BeginTabBar("main cheat")) {
			if (ImGui::BeginTabItem("Aim")) {
				ImGui::Checkbox("Aimbot", &Config::bAimbot);
				if (Config::bAimbot) {
					ImGui::Checkbox("Visible Check", &Config::bVisCheck);
					ImGui::SliderFloat("Smoothness", &Config::aimbotSmooth, 0.0f, 1.0, "%.3f");
				}
				ImGui::Checkbox("Triggerbot", &Config::bTriggerbot);
				if (Config::bTriggerbot) {
					if (ImGui::BeginCombo("Attack Settings", combo_preview_value, 0))
					{
						for (int n = 0; n < IM_ARRAYSIZE(items); n++)
						{
							const bool is_selected = (triggerbot_selected == n);
							if (ImGui::Selectable(items[n], is_selected)) {
								triggerbot_selected = n;
								combo_preview_value = items[triggerbot_selected];
							}

							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("ESP")) {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "soon :)");
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Misc"))
			{
				ImGui::Checkbox("GodMode", &Config::bHealth);
				ImGui::Checkbox("No Recoil", &Config::bRecoil);
				ImGui::Checkbox("Inf Ammo", &Config::bAmmo);
				ImGui::Checkbox("Fly Hack", &Config::bFly);
				ImGui::Checkbox("Rapid Fire", &Config::bRapidFire);
				ImGui::Checkbox("BunnyHop", &Config::bBunnyhop);
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
				ImGui::Text("CURRENT YAW: %f PITCH: %f", localPlayer->yaw, localPlayer->pitch);
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

struct traceresult_s {
	Vector3 end;
	bool collided;
};

bool IsVisible(ent*& entity) {
	uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");
	moduleBase = (uintptr_t)GetModuleHandle(NULL);
	ent* localPlayer = *(ent**)(moduleBase + 0x10F4F4);

	uintptr_t traceLine = 0x048A310;
	traceresult_s traceresult;
	traceresult.collided = false;
	Vector3 from = localPlayer->headpos;
	Vector3 to = entity->headpos;

	__asm {
		push 0; bSkipTags
		push 0; bCheckPlayers
		push localPlayer //a4
		push to.z //vec3 dst
		push to.y //vec3 dst
		push to.x //vec3 dst
		push from.z //vec3 src
		push from.y //vec3 src
		push from.x //vec3 src
		lea eax, [traceresult] //a1<eax>
		call traceLine;
		add esp, 36
	}
	return !traceresult.collided;
}

DWORD WINAPI HackThread(HMODULE hModule) {
	uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");
	moduleBase = (uintptr_t)GetModuleHandle(NULL);

	getCrosshairEnt = (_GetCrosshairEnt)(moduleBase + 0x607C0);

	bool recoilPatched = false;
	bool flyPatched = false;
	bool rapidPatched = false;
	bool onehitPatched = false;
	bool isFFA = false;

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

	printInGame("\f8Venom injected!");

	while (true) {
		ent* localPlayer = *(ent**)(moduleBase + 0x10F4F4);
		uintptr_t localPlayerPtr = *(uintptr_t*)(moduleBase + 0x10F4F4);
		entList* entityList = *(entList**)(moduleBase + 0x010F4F8);
		uintptr_t niggaList = *(uintptr_t*)(moduleBase + 0x10F4F8);
		uintptr_t currPlayers = *(int*)(0x50F500);
		uintptr_t gamemodeAddr = *(int*)(moduleBase + 0x10A044);

		if (gamemodeAddr == 7 ? isFFA = true : isFFA = false);

		if (GetAsyncKeyState(VK_END) & 1)
		{
			break;
		}

		if (!localPlayer) continue;

		if (Config::bAimbot && niggaList) {
			float closestDistance = FLT_MAX;
			ent* closestEntity = nullptr;

			static auto lastUpdateTime = std::chrono::steady_clock::now();

			for (int i = 0; i < currPlayers; i++) {
				uintptr_t entityObj = *(uintptr_t*)(niggaList + i * 0x4);
				ent* entity = reinterpret_cast<ent*>(entityObj);

				if (!entity) continue;
				if (entity->health <= 0) continue;
				if (isFFA && entity->team == localPlayer->team) continue;
				if (Config::bVisCheck && !IsVisible(entity)) continue;

				float deltaX = entity->bodypos.x - localPlayer->bodypos.x;
				float deltaY = entity->bodypos.y - localPlayer->bodypos.y;
				float deltaZ = entity->bodypos.z - localPlayer->bodypos.z;

				float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);

				if (distance < closestDistance) {
					closestDistance = distance;
					closestEntity = entity;
				}
			}

			if (closestEntity) {
				float currentYaw = localPlayer->yaw;
				float currentPitch = localPlayer->pitch;
				float abspos_x = closestEntity->bodypos.x - localPlayer->bodypos.x;
				float abspos_y = closestEntity->bodypos.y - localPlayer->bodypos.y;
				float abspos_z = closestEntity->bodypos.z - localPlayer->bodypos.z;

				float azimuth_xy = atan2f(abspos_y, abspos_x);
				float targetYaw = azimuth_xy * (180.0f / std::numbers::pi);

				float azimuth_z = atan2f(abspos_z, std::hypot(abspos_x, abspos_y));
				float targetPitch = azimuth_z * (180.0f / std::numbers::pi);

				auto currentTime = std::chrono::steady_clock::now();
				if (currentTime - lastUpdateTime >= std::chrono::milliseconds(7)) {
					float yawDiff = targetYaw + 90.0f - currentYaw;
					if (yawDiff > 180.0f) yawDiff -= 360.0f;
					if (yawDiff < -180.0f) yawDiff += 360.0f;

					float smoothingFactor = 1.1f - Config::aimbotSmooth;

					currentYaw += yawDiff * smoothingFactor;
					currentPitch += (targetPitch - currentPitch) * smoothingFactor;

					localPlayer->yaw = currentYaw;
					localPlayer->pitch = currentPitch;

					lastUpdateTime = currentTime;
				}
			}
		}

		if (Config::bBunnyhop) {
			uintptr_t isGround = localPlayerPtr + 0x68;

			if (GetAsyncKeyState(VK_SPACE) && *(int*)isGround == 256) {
				SendMessageA(hwnd, WM_KEYDOWN, VK_SPACE, 0x20);
				Sleep(1);
				SendMessageA(hwnd, WM_KEYUP, VK_SPACE, 0x20);
			}
		}

		if (Config::bRecoil && !recoilPatched) {
			mem::Patch((BYTE*)(0x462020), (BYTE*)"\xC2\x08\x00", 3);
			recoilPatched = true;
		}
		else if (!Config::bRecoil && recoilPatched) {
			mem::Patch((BYTE*)(0x462020), (BYTE*)"\x55\x8B\xEC", 3);
			recoilPatched = false;
		}

		if (Config::bFly && !flyPatched) {
			*(int*)(*(uintptr_t*)0x50F4F4 + 0x338) = 5;
			flyPatched = true;
		}
		else if (!Config::bFly && flyPatched) {
			*(int*)(*(uintptr_t*)0x50F4F4 + 0x338) = 0;
			flyPatched = false;
		}

		if (Config::bRapidFire && !rapidPatched) {
			mem::Nop((BYTE*)(moduleBase + 0x637E4), 2);
			rapidPatched = true;
		}
		else if (!Config::bRapidFire && rapidPatched) {
			mem::Patch((BYTE*)(moduleBase + 0x637E4), (BYTE*)"\x89\x0A", 2);
			rapidPatched = false;
		}

		if (Config::bOneHit) {
			*(int*)mem::FindDMAAddy(0x50F4F4, { 0x374, 0xC, 0x10C }) = 999;
			onehitPatched = true;
		}
		else if (!Config::bOneHit && onehitPatched) {
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

		if (Config::bHealth) {
			localPlayer->health = 999;
		}

		if (Config::bAmmo) {
			localPlayer->currWeapon->ammoPtr->ammoClip = 999;
		}

		if (Config::bTriggerbot) {
			ent* crosshairEnt = getCrosshairEnt();

			if (triggerbot_selected == 1) {
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
			else {
				if (crosshairEnt) {
					localPlayer->bAttack = 1;
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
	printInGame("\f8Venom uninjected!");

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
