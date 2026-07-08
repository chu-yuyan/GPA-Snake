#include "Renderer.h"
#include <graphics.h>

#include <unordered_map>
#include <windows.h>

void Renderer::drawText(int x, int y, const std::wstring& text, int size)
{
	setbkmode(TRANSPARENT);
	settextcolor(WHITE);
	settextstyle(size, 0, L"Consolas");
	outtextxy(x, y, text.c_str());
}

void Renderer::drawPanel(int x, int y, int w, int h)
{
	setlinecolor(RGB(160, 160, 160));
	setfillcolor(RGB(20, 20, 20));
	fillrectangle(x, y, x + w, y + h); 
	rectangle(x, y, x + w, y + h);
}

bool Renderer::drawImageStretch(const std::wstring& filePath, int x, int y, int w, int h)
{
	static std::unordered_map<std::wstring, IMAGE> cache;

	auto it = cache.find(filePath);
	if (it == cache.end())
	{
		IMAGE img;
		loadimage(&img, filePath.c_str());

		if (img.getwidth() <= 0 || img.getheight() <= 0)
		{
			std::wstring msg = L"[Renderer] loadimage failed: " + filePath + L"\n";
			OutputDebugStringW(msg.c_str());
			return false;
		}

		it = cache.emplace(filePath, img).first;
	}

	// EasyX 支持的 7 参版本：dstX,dstY,dstW,dstH,srcImg,srcX,srcY
	putimage(x, y, w, h, &it->second, 0, 0);
	return true;
}

void Renderer::drawTextStyled(int x, int y,
	const std::wstring& text,
	int size,
	COLORREF color,
	bool bold,
	const wchar_t* fontName)
{
	setbkmode(TRANSPARENT);
	settextcolor(color);

	LOGFONTW lf{};
	lf.lfHeight = size;
	lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
	wcsncpy_s(lf.lfFaceName, fontName, _TRUNCATE);
	settextstyle(&lf);
	outtextxy(x, y, text.c_str());
}

void Renderer::drawTextInRect(
	int x, int y, int w, int h,
	const std::wstring& text,
	int size,
	COLORREF color,
	bool bold,
	UINT format,
	const wchar_t* fontName)
{
	setbkmode(TRANSPARENT);
	settextcolor(color);

	LOGFONTW lf{};
	lf.lfHeight = size;
	lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
	wcsncpy_s(lf.lfFaceName, fontName, _TRUNCATE);
	settextstyle(&lf);

	RECT rc{ x, y, x + w, y + h };
	drawtext(text.c_str(), &rc, format);
}