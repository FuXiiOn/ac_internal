#pragma once
#include <iostream>
#include <Windows.h>
#include "geom.h"
#include <gl/GL.h>
#include "offsets.h"

namespace GL {
	bool WorldToScreen(Vector3 pos, Vector3& screen, float matrix[16], float wndWidth, float wndHeight);
	void SetupOrtho();
	void RestoreGL();
	void DrawOutline(float x, float y, float width, float height, float lineWidth, const GLubyte color[3]);
	void DrawFilledRect(float x, float y, float width, float height, const GLubyte color[3]);
	Vector3 centerText(float x, float y, float width, float height, float textWidth, float textHeight);
	float centerText(float x, float width, float textWidth);
	void Build(int height);
	void Print(float x, float y, const GLubyte color[3], const char* format, ...);
	void DrawESPBox(ent* e, Vector3 screenCoords);
}