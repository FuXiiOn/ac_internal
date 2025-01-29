#include "stdafx.h"
#include <iostream>
#include <gl/GL.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui_impl_win32.h"
#include "mem.h"
#include <string>
#include <thread>
#include "config.h";
#include "geom.h"
#include <algorithm>
#include "offsets.h"
#include <cmath>
#include <numbers>
#include "Psapi.h"
#include "fstream"
#include "glDraw.h"
#pragma comment(lib, "opengl32.lib")
#define M_PI 3.14159265358979323846

typedef enum {
	SDL_GRAB_QUERY,
	SDL_GRAB_OFF,
	SDL_GRAB_ON
} SDL_GrabMode;
typedef BOOL(__cdecl* t_SDL_WM_GrabInput)(SDL_GrabMode mode);
t_SDL_WM_GrabInput _SDL_WM_GrabInput;

typedef int(__cdecl* _printInGame)(const char* format, ...);
_printInGame printInGame = (_printInGame)(0x4090f0);

typedef ent* (__cdecl* _GetCrosshairEnt)();
_GetCrosshairEnt getCrosshairEnt = nullptr;

typedef BOOL(__stdcall* t_wglSwapBuffers)(HDC hDc);
t_wglSwapBuffers gateway_wglSwapBuffers;

namespace rgb {
	const GLubyte red[3] = { 255, 0, 0 };
	const GLubyte green[3] = { 0, 255, 0 };
	const GLubyte gray[3] = { 55, 55, 55 };
	const GLubyte graylight[3] = { 192, 192, 192 };
	const GLubyte black[3] = { 0, 0, 0 };
}

const char* items[] = { "FFA", "TEAM" };
static int triggerbot_selected = 0;
const char* combo_preview_value = items[triggerbot_selected];

char windowTitle[] = "AssaultCube";
HWND gameHWND = FindWindowA(NULL, windowTitle);
int viewport[4];
ent* closestSilent = nullptr;
BYTE originalSilentBytes[10];

BOOL __stdcall hooked_wglSwapBuffers(HDC hDc) {
	uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");
	moduleBase = (uintptr_t)GetModuleHandle(NULL);

	ent* localPlayer = *(ent**)(moduleBase + 0x10F4F4);

	entList* entityList = *(entList**)(moduleBase + 0x10F4F8);

	GL::SetupOrtho();

	GL::Build(15);

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
					ImGui::Checkbox("Silent Aim", &Config::bSilent);
					ImGui::Checkbox("Visible Check", &Config::bVisCheck);
					ImGui::Checkbox("FOV", &Config::bAimFov);
					if (Config::bAimFov) {
						ImGui::SliderFloat("FOV Radius", &Config::fovRadius, 10, 500, "%.3f");
					}
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
				ImGui::Checkbox("ESP", &Config::bEsp);
				if (Config::bEsp) {
					ImGui::Checkbox("Health bar", &Config::bHealthBar);
					ImGui::Checkbox("SnapLines", &Config::bSnapLines);
				}
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
		if (GetForegroundWindow() == gameHWND) {
			_SDL_WM_GrabInput(SDL_GrabMode(1));
		}
		else {
			t_SDL_WM_GrabInput(SDL_GrabMode(2));
		}
	}

	if (Config::bEsp) {
		int* currPlayers = (int*)(0x50F500);
		uintptr_t entityList = *(uintptr_t*)(moduleBase + 0x10F4F8);
		float* viewMatrix = (float*)0x501AE8;

		const float PLAYER_HEIGHT = 5.25f;
		const float EYE_HEIGHT = 4.5f;

		for (unsigned int i = 0; i < *currPlayers; i++) {
			uintptr_t entityObj = *(uintptr_t*)(entityList + i * 0x4);
			ent* entity = reinterpret_cast<ent*>(entityObj);

			if (!entity) continue;
			if (entity->health < 0) continue;

			glGetIntegerv(GL_VIEWPORT, viewport);
			Vector3 newHead = entity->headpos;
			newHead.z = entity->headpos.z - EYE_HEIGHT + PLAYER_HEIGHT / 2;

			Vector3 screenCoords;

			if (GL::WorldToScreen(newHead, screenCoords, viewMatrix, viewport[2], viewport[3])) {
				GL::DrawESPBox(entity, screenCoords);
			}
		}
	}

	if (Config::bSilent) {
		if (closestSilent) {
			Vector3 screenCoords;
			float* viewMatrix = (float*)0x501AE8;

			bool isVisible = GL::WorldToScreen(closestSilent->headpos, screenCoords, viewMatrix, viewport[2], viewport[3]);

			if (!isVisible) {
				Vector3 camSpace;
				camSpace.x = closestSilent->headpos.x * viewMatrix[0] + closestSilent->headpos.y * viewMatrix[4] + closestSilent->headpos.z * viewMatrix[8] + viewMatrix[12];
				camSpace.y = closestSilent->headpos.x * viewMatrix[1] + closestSilent->headpos.y * viewMatrix[5] + closestSilent->headpos.z * viewMatrix[9] + viewMatrix[13];
				camSpace.z = closestSilent->headpos.x * viewMatrix[2] + closestSilent->headpos.y * viewMatrix[6] + closestSilent->headpos.z * viewMatrix[10] + viewMatrix[14];

				// Normalize direction
				float length = sqrt(camSpace.x * camSpace.x + camSpace.y * camSpace.y );
				camSpace.x /= length;
				camSpace.y /= length;

				// Scale it to the screen edge
				float edgeDistance = viewport[2] * 0.75f;  // Move it out of view
				screenCoords.x = (viewport[2] / 2) + camSpace.x * edgeDistance;
				screenCoords.y = (viewport[3] / 2) - camSpace.y * edgeDistance;
			}

				glGetIntegerv(GL_VIEWPORT, viewport);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
				glBegin(GL_LINES);
				glVertex2f(viewport[2] / 2, viewport[3] / 2);
				glVertex2f(screenCoords.x, screenCoords.y);
				glEnd();
				glDisable(GL_BLEND);
			
		}
	}

	if (Config::bAimFov) {
		int wndWidth, wndHeight;

		RECT rect;
		GetClientRect(gameHWND, &rect);

		wndWidth = rect.right - rect.left;
		wndHeight = rect.bottom - rect.top;

		int centerWidth = wndWidth / 2;
		int centerHeight = wndHeight / 2;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBegin(GL_LINE_LOOP);
		glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
		for (int i = 0; i <= 50; i++) {
			float angle = 2.0f * M_PI * i / 50;
			float x = Config::fovRadius * cos(angle);
			float y = Config::fovRadius * sin(angle);
			glVertex2f(x + centerWidth, y + centerHeight);
		}
		glEnd();
		glDisable(GL_BLEND);
	}

	GL::RestoreGL();

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
	ent* localPlayer = *(ent**)(moduleBase + 0x10F4F4);

	uintptr_t traceLine = 0x048A310;
	traceresult_s traceresult;
	traceresult.collided = false;
	Vector3 from = localPlayer->headpos;
	Vector3 to = entity->headpos;

	__try {
		__asm {
			push 0; bSkipTags //a6 (unnecessary argument)
			push 0; bCheckPlayers //a5 (unnecessary argument)
			push localPlayer //a4
			push to.z //vec3 dst
			push to.y //vec3 dst
			push to.x //vec3 dst
			push from.z //vec3 src
			push from.y //vec3 src
			push from.x //vec3 src
			lea eax, [traceresult] //a1<eax>
			call traceLine
			add esp, 36 //clean stack
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
	return !traceresult.collided;
}

uintptr_t oSilentAddr = 0;
uintptr_t silentContinue = 0;

void __declspec(naked) hookedSilent() {
	static uintptr_t old_eax = 0;

	__asm {
		cmp Config::bSilent,0
		je exit_hook

		mov old_eax, eax
		mov eax, [closestSilent]
		cmp eax, 0
		je exit_hook
		add eax, 4
		mov ecx, eax
		mov eax, [old_eax]
		jmp silentContinue

	exit_hook:
		jmp oSilentAddr
	}
}

DWORD WINAPI HackThread(HMODULE hModule) {
	uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");
	uintptr_t recoilAddress = mem::FindPattern((HMODULE)moduleBase, (const unsigned char*)"\x55\x8b\xec\x83\xe4\x00\x83\xec\x00\x53\x56\x8b\xf1\x8b\x46", "xxxxx?xx?xxxxxx");
	uintptr_t silentAimAddr = (moduleBase + 0x637B6);
	memcpy(originalSilentBytes, (BYTE*)silentAimAddr, 5);
	oSilentAddr = silentAimAddr + 5;

	getCrosshairEnt = (_GetCrosshairEnt)(moduleBase + 0x607C0);

	bool recoilPatched = false;
	bool flyPatched = false;
	bool rapidPatched = false;
	bool onehitPatched = false;
	bool isFFA = false;

	oWndProc = (WNDPROC)SetWindowLongPtr(gameHWND, GWL_WNDPROC, (LONG_PTR)WndProc);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(gameHWND);
	ImGui_ImplOpenGL2_Init();

	uintptr_t wglSwapBuffersAddr = (uintptr_t)GetProcAddress(GetModuleHandle(L"opengl32.dll"), "wglSwapBuffers");
	BYTE originalSwapBuffersBytes[10];
	memcpy(originalSwapBuffersBytes, (BYTE*)wglSwapBuffersAddr, 5);
	oSilentAddr = (uintptr_t)mem::TrampHook((BYTE*)silentAimAddr, (BYTE*)hookedSilent, 5);
	silentContinue = oSilentAddr + 4;

	_SDL_WM_GrabInput = (t_SDL_WM_GrabInput)GetProcAddress(GetModuleHandle(L"SDL.dll"), "SDL_WM_GrabInput");
	gateway_wglSwapBuffers = (t_wglSwapBuffers)wglSwapBuffersAddr;

	gateway_wglSwapBuffers = (t_wglSwapBuffers)mem::TrampHook((BYTE*)gateway_wglSwapBuffers, (BYTE*)hooked_wglSwapBuffers, 5);

	printInGame("\f8Venom injected!");

	while (true) {
		ent* localPlayer = *(ent**)(moduleBase + 0x10F4F4);
		uintptr_t localPlayerPtr = *(uintptr_t*)(moduleBase + 0x10F4F4);
		entList* entityList = *(entList**)(moduleBase + 0x010F4F8);
		uintptr_t entList2 = *(uintptr_t*)(moduleBase + 0x10F4F8);
		uintptr_t currPlayers = *(int*)(0x50F500);
		uintptr_t gamemodeAddr = *(int*)(moduleBase + 0x10A044);
		ent* crosshairEnt = getCrosshairEnt();
		RECT rect;
		GetClientRect(gameHWND, &rect);
		int screenWidth = rect.right - rect.left;
		int screenHeight = rect.bottom - rect.top;

		if (gamemodeAddr == 7 ? isFFA = true : isFFA = false);

		if (GetAsyncKeyState(VK_END) & 1)
		{
			break;
		}

		if (!localPlayer) continue;

		if (Config::bAimbot && entList2) {
			float closestDistance = FLT_MAX;
			ent* closestEntity = nullptr;
			ent* bestSilent = nullptr;
			float bestFov = FLT_MAX;
			float bestYaw = 0.0f, bestPitch = 0.0f;
			float radiusDegrees = 2.0f * atan(Config::fovRadius / (screenWidth / 2.0f)) * (180.0f / M_PI);
			float currentYaw, currentPitch;

			static auto lastUpdateTime = std::chrono::steady_clock::now();

			for (int i = 0; i < currPlayers; i++) {
				uintptr_t entityObj = *(uintptr_t*)(entList2 + i * 0x4);
				ent* entity = reinterpret_cast<ent*>(entityObj);

				if (!entity) continue;
				if (entity->health < 1) continue;
				if (isFFA && entity->team == localPlayer->team) continue;
				if (Config::bVisCheck && !IsVisible(entity)) continue;

				float abspos_x = entity->bodypos.x - localPlayer->bodypos.x;
				float abspos_y = entity->bodypos.y - localPlayer->bodypos.y;
				float abspos_z = entity->bodypos.z - localPlayer->bodypos.z;

				float distance = sqrt(abspos_x * abspos_x + abspos_y * abspos_y + abspos_z * abspos_z);

				float azimuth_xy = atan2f(abspos_y, abspos_x);
				float newYaw = azimuth_xy * (180.0f / M_PI);

				float azimuth_z = atan2f(abspos_z, hypot(abspos_x, abspos_y));
				float newPitch = azimuth_z * (180.0f / M_PI);

				currentYaw = localPlayer->yaw;
				currentPitch = localPlayer->pitch;

				float yawDiff = newYaw + 90.0f - currentYaw;
				float pitchDiff = newPitch - currentPitch;

				if (yawDiff > 180.0f) yawDiff -= 360.0f;
				if (yawDiff < -180.0f) yawDiff += 360.0f;

				float fov = sqrt(yawDiff * yawDiff + pitchDiff * pitchDiff);

				if (Config::bAimFov) {
					if (fov < radiusDegrees && fov < bestFov) {
						closestEntity = entity;
						bestSilent = entity;
						bestFov = fov;
						bestYaw = newYaw + 90.0f;
						bestPitch = newPitch;
					}
				}
				else {
					if (distance < closestDistance) {
						closestEntity = entity;
						bestSilent = entity;
						closestDistance = distance;
						bestYaw = newYaw + 90.0f;
						bestPitch = newPitch;
					}
				}
			}

			if(bestSilent) {
				closestSilent = bestSilent;
			}
			else {
				closestSilent = nullptr;
			}

			if (closestEntity) {
				if (!Config::bSilent) {
					auto currentTime = std::chrono::steady_clock::now();
					if (currentTime - lastUpdateTime >= std::chrono::milliseconds(7)) {
						float yawDiff = bestYaw - currentYaw;
						float pitchDiff = bestPitch - currentPitch;

						if (yawDiff > 180.0f) yawDiff -= 360.0f;
						if (yawDiff < -180.0f) yawDiff += 360.0f;

						float smoothingFactor = std::clamp((1.0f - Config::aimbotSmooth), 0.05f, 1.0f);

						currentYaw += yawDiff * smoothingFactor;
						currentPitch += pitchDiff * smoothingFactor;

						localPlayer->yaw = currentYaw;
						localPlayer->pitch = currentPitch;

						lastUpdateTime = currentTime;
					}
				}
			}
		}

		if (Config::bBunnyhop) {
			uintptr_t isGround = localPlayerPtr + 0x68;
			uintptr_t isShifting = localPlayerPtr + 0x6C;
			uintptr_t doJump = localPlayerPtr + 0x6B;
			static bool jumpPatched = false;

			if (GetAsyncKeyState(VK_SPACE) && *(int*)isGround == 256) {
				*(int*)doJump = 1;
				jumpPatched = true;
			}

			if (jumpPatched && *(int*)isGround == 256 && *(int*)doJump == 0) {
				*(int*)doJump = 256;
				jumpPatched = false;
			}
		}

		if (Config::bRecoil && !recoilPatched) {
			mem::Patch((BYTE*)recoilAddress, (BYTE*)"\xC2\x08\x00", 3);
			recoilPatched = true;
		}
		else if (!Config::bRecoil && recoilPatched) {
			mem::Patch((BYTE*)recoilAddress, (BYTE*)"\x55\x8B\xEC", 3);
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

	mem::Patch((BYTE*)silentAimAddr, (BYTE*)originalSilentBytes, 5);
	mem::Patch((BYTE*)wglSwapBuffersAddr, (BYTE*)originalSwapBuffersBytes, 5);

	Sleep(1000);

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
