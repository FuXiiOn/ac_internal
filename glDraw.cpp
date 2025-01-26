#include "glDraw.h"
#include <gl/GL.h>
#include "geom.h"
#include "offsets.h"
#include "config.h"

unsigned int base;
HDC hdc = nullptr;
extern int viewport[4];

namespace rgb {
	const GLubyte red[3] = { 255, 0, 0 };
	const GLubyte green[3] = { 0, 255, 0 };
	const GLubyte gray[3] = { 55, 55, 55 };
	const GLubyte graylight[3] = { 192, 192, 192 };
	const GLubyte black[3] = { 0, 0, 0 };
}

bool GL::WorldToScreen(Vector3 pos, Vector3& screen, float matrix[16], float wndWidth, float wndHeight) {
	Vector4 clipCoords;
	clipCoords.x = pos.x * matrix[0] + pos.y * matrix[4] + pos.z * matrix[8] + matrix[12];
	clipCoords.y = pos.x * matrix[1] + pos.y * matrix[5] + pos.z * matrix[9] + matrix[13];
	clipCoords.z = pos.x * matrix[2] + pos.y * matrix[6] + pos.z * matrix[10] + matrix[14];
	clipCoords.w = pos.x * matrix[3] + pos.y * matrix[7] + pos.z * matrix[11] + matrix[15];

	if (clipCoords.w < 0.1f) return false;

	Vector3 NDC;
	NDC.x = clipCoords.x / clipCoords.w;
	NDC.y = clipCoords.y / clipCoords.w;
	NDC.z = clipCoords.z / clipCoords.w;

	screen.x = (wndWidth / 2 * NDC.x) + (NDC.x + wndWidth / 2);
	screen.y = -(wndHeight / 2 * NDC.y) + (NDC.y + wndHeight / 2);
	return true;
}

void GL::SetupOrtho() {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glViewport(0, 0, viewport[2], viewport[3]);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, viewport[2], viewport[3], 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
}

void GL::RestoreGL() {
	glPopAttrib();
	glPopMatrix();
}

void GL::DrawOutline(float x, float y, float width, float height, float lineWidth, const GLubyte color[3]) {
	glLineWidth(lineWidth);
	glBegin(GL_LINE_STRIP);
	glColor3ub(color[0], color[1], color[2]);
	glVertex2f(x - 0.5f, y - 0.5f);
	glVertex2f(x + width + 0.5f, y - 0.5f);
	glVertex2f(x + width + 0.5f, y + height + 0.5f);
	glVertex2f(x - 0.5f, y + height + 0.5f);
	glVertex2f(x - 0.5f, y - 0.5f);
	glEnd();
}

void GL::DrawFilledRect(float x, float y, float width, float height, const GLubyte color[3]) {
	glColor3ub(color[0], color[1], color[2]);
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + width, y);
	glVertex2f(x + width, y + height);
	glVertex2f(x, y + height);
	glEnd();
}

Vector3 GL::centerText(float x, float y, float width, float height, float textWidth, float textHeight) {
	Vector3 text;
	text.x = x + (width - textWidth) / 2;
	text.y = y + textHeight;
	return text;
}

float GL::centerText(float x, float width, float textWidth) {
	return x + (width - textWidth) / 2;
}

void GL::Build(int height) {
	hdc = wglGetCurrentDC();
	base = glGenLists(96);
	HFONT hFont = CreateFontA(-height, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "Consolas");
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
	wglUseFontBitmaps(hdc, 32, 96, base);
	SelectObject(hdc, hFont);
	DeleteObject(hFont);
}

void GL::Print(float x, float y, const GLubyte color[3], const char* format, ...) {
	glColor3ub(color[0], color[1], color[2]);
	glRasterPos2f(x, y);

	char text[100];
	va_list args;
	va_start(args, format);
	vsprintf_s(text, 100, format, args);
	va_end(args);

	glPushAttrib(GL_LIST_BIT);
	glListBase(base - 32);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}

void GL::DrawSnapLine(float startX, float startY, float entityX, float entityY) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
	glBegin(GL_LINES);
	glVertex2f(startX, startY);
	glVertex2f(entityX, entityY);
	glEnd();
	glDisable(GL_BLEND);
}

void GL::DrawESPBox(ent* e, Vector3 screenCoords) {
	uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");
	ent* localPlayer = *(ent**)(moduleBase + 0x10F4F4);
	const GLubyte* color = nullptr;
	uintptr_t gamemodeAddr = *(int*)(moduleBase + 0x10A044);
	bool isFFA = false;
	if (gamemodeAddr == 7 ? isFFA = true : isFFA = false);
	const char* window = "AssaultCube";
	HWND hwnd = FindWindowA(0, window);
	RECT rect;
	GetClientRect(hwnd, &rect);
	int VIRTUAL_SCREEN_WIDTH = rect.right - rect.left;

	if (isFFA && e->team == localPlayer->team) {
		color = rgb::green;
	}
	else {
		color = rgb::red;
	}

	const int GAME_UNIT_MAGIC = 400;

	const float PLAYER_HEIGHT = 5.25f;
	const float PLAYER_WIDTH = 2.0f;
	const float EYE_HEIGHT = 4.5f;
	const float PLAYER_ASPECT_RATIO = PLAYER_HEIGHT / PLAYER_WIDTH;

	const int ESP_FONT_HEIGHT = 15;
	const int ESP_FONT_WIDTH = 9;

	float distance = localPlayer->bodypos.getDistance(e->bodypos);

	float scale = (GAME_UNIT_MAGIC / distance) * (viewport[2] / VIRTUAL_SCREEN_WIDTH);
	float x = screenCoords.x - scale;
	float y = screenCoords.y - scale * PLAYER_ASPECT_RATIO;
	float width = scale * 2;
	float height = scale * PLAYER_ASPECT_RATIO * 2;

	GL::DrawOutline(x, y, width, height, 2.0f, color);

	if (Config::bHealthBar) {
		GL::DrawHealthBar(x, y, width, height, e->health, 100);
	}

	if (Config::bSnapLines) {
		int screenCenterX = viewport[2] / 2;
		int screenCenterY = viewport[3];
		GL::DrawSnapLine(screenCenterX, screenCenterY, x + width / 2, y + height);
	}

	float textX = GL::centerText(x, width, strlen(e->name) * ESP_FONT_WIDTH);
	float textY = y - ESP_FONT_WIDTH / 2;
	GL::Print(textX, textY, color, "%s", e->name);
}

void GL::DrawHealthBar(float x, float y, float width, float height, int currentHealth, int maxHealth) {
	float healthPercent = (float)currentHealth / maxHealth;
	
	float barWidth = 5.0f;
	float filledHeight = height * healthPercent;

	GL::DrawFilledRect(x + width + 3, y, barWidth, height, rgb::gray);

	GL::DrawFilledRect(x + width + 3, y + (height - filledHeight), barWidth, filledHeight, rgb::green);
}