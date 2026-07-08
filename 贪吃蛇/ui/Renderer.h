#pragma once

#include <string>
#include <graphics.h>

class Renderer
{
public:
	static void drawText(int x, int y, const std::wstring& text, int size = 20);
	static void drawPanel(int x, int y, int w, int h);
	static bool drawImageStretch(const std::wstring& filePath, int x, int y, int w, int h);

	static void drawTextStyled(
		int x, int y,
		const std::wstring& text,
		int size,
		COLORREF color,
		bool bold = false,
		const wchar_t* fontName = L"Consolas");

	static void drawTextInRect(
		int x, int y, int w, int h,
		const std::wstring& text,
		int size,
		COLORREF color,
		bool bold = false,
		UINT format = DT_WORDBREAK | DT_LEFT | DT_TOP,
		const wchar_t* fontName = L"Î¢ÈíÑÅºÚ");
};