#include "Animation.h"
#include "Renderer.h"

#include "../core/ScoreSystem.h"
#include "../core/EventManager.h"
#include "../system/Timer.h"
#include "SettlementManager.h"

#include <graphics.h>

// 禁用 windows.h 里的 min/max 宏，否则会把 std::min/std::max 搞崩
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <string>
#include <algorithm>

static bool InRectLocal(int x, int y, int w, int h, int mx, int my)
{
	return mx >= x && mx <= x + w && my >= y && my <= y + h;
}

// ====== 轻量 UI 绘制辅助（放在 cpp 内部，不污染头文件） ======
static int ClampI(int v, int lo, int hi) { return (v < lo) ? lo : (v > hi ? hi : v); }

static COLORREF LerpColor(COLORREF a, COLORREF b, float t)
{
	t = (t < 0.0f) ? 0.0f : (t > 1.0f ? 1.0f : t);
	const int ar = GetRValue(a), ag = GetGValue(a), ab = GetBValue(a);
	const int br = GetRValue(b), bg = GetGValue(b), bb = GetBValue(b);
	const int rr = (int)(ar + (br - ar) * t);
	const int rg = (int)(ag + (bg - ag) * t);
	const int rb = (int)(ab + (bb - ab) * t);
	return RGB(ClampI(rr, 0, 255), ClampI(rg, 0, 255), ClampI(rb, 0, 255));
}

// 0..1..0 的平滑呼吸（periodMs 一个周期）
static float Pulse01(int periodMs, int offsetMs = 0)
{
	const int ms = (int)GetTickCount() + offsetMs;
	const float x = (float)(ms % periodMs) / (float)periodMs; // 0..1
	// 三角波 -> 0..1..0
	const float tri = (x < 0.5f) ? (x * 2.0f) : (2.0f - x * 2.0f);
	// 稍微平滑一下
	return tri * tri * (3.0f - 2.0f * tri); // smoothstep
}

static void DrawPanelEx(int x, int y, int w, int h, COLORREF fill, COLORREF border)
{
	setfillcolor(fill);
	setlinecolor(border);
	fillrectangle(x, y, x + w, y + h);
	rectangle(x, y, x + w, y + h);
}

static void DrawShadowPanel(int x, int y, int w, int h)
{
	// 伪阴影：右下偏移一层深色
	DrawPanelEx(x + 3, y + 3, w, h, RGB(10, 10, 10), RGB(10, 10, 10));
	DrawPanelEx(x, y, w, h, RGB(26, 26, 28), RGB(170, 170, 170));
}

static void DrawCapsuleBar(int x, int y, int w, int h, float progress01, COLORREF accent, const std::wstring& text)
{
	// 背景
	DrawPanelEx(x, y, w, h, RGB(26, 26, 28), RGB(160, 160, 160));

	// 进度条
	const int pad = 6;
	const int barH = 8;
	const int barY = y + h - pad - barH;
	const int barW = w - pad * 2;
	const int filled = (int)(barW * (std::max)(0.0f, (std::min)(1.0f, progress01)));

	DrawPanelEx(x + pad, barY, barW, barH, RGB(18, 18, 18), RGB(70, 70, 70));
	if (filled > 0)
	{
		DrawPanelEx(x + pad, barY, filled, barH, LerpColor(accent, RGB(255, 255, 255), 0.12f), accent);
	}

	// 文本
	Renderer::drawTextStyled(x + 12, y + 10, text, 18, RGB(235, 235, 235), false, L"Consolas");
}

void Animation::calcEventDialogLayout(int screenW, int screenH, RectI& dialog, RectI& okBtn, RectI& skipBtn) const
{
	dialog.w = 560;
	dialog.h = 240;
	dialog.x = (screenW - dialog.w) / 2;
	dialog.y = (screenH - dialog.h) / 2;

	const int btnY = dialog.y + dialog.h - 56;
	const int btnH = 34;
	const int btnW = 180;

	okBtn = { dialog.x + dialog.w / 2 - btnW - 18, btnY, btnW, btnH };
	skipBtn = { dialog.x + dialog.w / 2 + 18, btnY, btnW, btnH };
}

const wchar_t* Animation::eventName(int e)
{
	switch ((GameEvent)e)
	{
	case GameEvent::ELECTIVE: return L"选修课";
	case GameEvent::SITP: return L"SITP";
	case GameEvent::PROJECT: return L"大作业";
	case GameEvent::CET4: return L"四六级";
	case GameEvent::CONTEST: return L"学科竞赛";
	default: return L"";
	}
}

AnimDialogAction Animation::hitTestEventConfirmDialog(int screenW, int screenH, const EventManager& ev,
	int mouseX, int mouseY, bool mouseClick) const
{
	if (!mouseClick) return AnimDialogAction::None;
	if (ev.state() != EventState::PENDING) return AnimDialogAction::None;

	RectI dlg, okBtn, skipBtn;
	calcEventDialogLayout(screenW, screenH, dlg, okBtn, skipBtn);

	if (InRectLocal(okBtn.x, okBtn.y, okBtn.w, okBtn.h, mouseX, mouseY)) return AnimDialogAction::Accept;
	if (InRectLocal(skipBtn.x, skipBtn.y, skipBtn.w, skipBtn.h, mouseX, mouseY)) return AnimDialogAction::Skip;
	return AnimDialogAction::None;
}

void Animation::drawEventConfirmDialog(int screenW, int screenH, int /*nowSeconds*/, const EventManager& ev) const
{
	if (ev.state() != EventState::PENDING) return;


	RectI dlg, okBtn, skipBtn;
	calcEventDialogLayout(screenW, screenH, dlg, okBtn, skipBtn);

	// 主弹窗（带伪阴影）
	DrawShadowPanel(dlg.x, dlg.y, dlg.w, dlg.h);

	// 标题条
	const int titleH = 52;
	DrawPanelEx(dlg.x, dlg.y, dlg.w, titleH, RGB(20, 20, 22), RGB(170, 170, 170));
	Renderer::drawTextStyled(dlg.x + 18, dlg.y + 14, L"事件确认", 22, RGB(245, 245, 245), true, L"Consolas");

	std::wstring title = L"是否参加：";
	title += eventName((int)ev.pendingEvent());
	Renderer::drawTextStyled(dlg.x + 18, dlg.y + 70, title, 20, RGB(230, 230, 230), false, L"Consolas");

	// 内容说明（更紧凑，颜色区分）
	if (ev.pendingEvent() == GameEvent::ELECTIVE)
	{
		Renderer::drawTextStyled(dlg.x + 18, dlg.y + 104, L"确定：解锁【选修课】并加入平时掉落/绩点计算", 18, RGB(210, 235, 255), false, L"Consolas");
		Renderer::drawTextStyled(dlg.x + 18, dlg.y + 130, L"跳过：本局不出现选修课，也不计入绩点", 18, RGB(235, 215, 215), false, L"Consolas");
	}
	else
	{
		Renderer::drawTextStyled(dlg.x + 18, dlg.y + 104, L"确定：暂停结束后，10秒内只刷新该事件食物", 18, RGB(210, 235, 255), false, L"Consolas");
		Renderer::drawTextStyled(dlg.x + 18, dlg.y + 130, L"跳过：继续正常课程掉落（不会出现该事件食物）", 18, RGB(235, 215, 215), false, L"Consolas");
	}

	// 按钮悬停高亮：用 WinAPI 获取鼠标（EasyX 没有 mousex/mousey）
	POINT pt{};
	GetCursorPos(&pt);

	const HWND hwnd = GetHWnd();
	ScreenToClient(hwnd, &pt);

	const int mx = (int)pt.x;
	const int my = (int)pt.y;

	const bool hoverOk = InRectLocal(okBtn.x, okBtn.y, okBtn.w, okBtn.h, mx, my);
	const bool hoverSkip = InRectLocal(skipBtn.x, skipBtn.y, skipBtn.w, skipBtn.h, mx, my);

	const COLORREF okFill = hoverOk ? RGB(36, 60, 36) : RGB(24, 34, 24);
	const COLORREF okBorder = hoverOk ? RGB(140, 220, 140) : RGB(120, 170, 120);
	DrawPanelEx(okBtn.x, okBtn.y, okBtn.w, okBtn.h, okFill, okBorder);
	Renderer::drawTextStyled(okBtn.x + 66, okBtn.y + 7, L"确定", 20, RGB(235, 255, 235), true, L"Consolas");

	const COLORREF skFill = hoverSkip ? RGB(60, 36, 36) : RGB(34, 24, 24);
	const COLORREF skBorder = hoverSkip ? RGB(230, 160, 160) : RGB(180, 120, 120);
	DrawPanelEx(skipBtn.x, skipBtn.y, skipBtn.w, skipBtn.h, skFill, skBorder);
	Renderer::drawTextStyled(skipBtn.x + 66, skipBtn.y + 7, L"跳过", 20, RGB(255, 235, 235), true, L"Consolas");
}

void Animation::playGameIntroBlocking(int screenW, int screenH) const
{
	// 更“像动画”的开场：约 1.2s 淡入 + 标题呼吸亮度
	const DWORD start = GetTickCount();
	const DWORD dur = 1200;

	while (true)
	{
		const DWORD now = GetTickCount();
		const float t = (dur == 0) ? 1.0f : (std::min)(1.0f, (now - start) / (float)dur);

		cleardevice();

		// 背景渐亮
		const COLORREF bg = LerpColor(RGB(0, 0, 0), RGB(22, 22, 24), t);
		DrawPanelEx(0, 0, screenW, screenH, bg, bg);

		// 标题呼吸
		const float pulse = 0.55f + 0.45f * Pulse01(1200);
		const COLORREF titleColor = LerpColor(RGB(180, 180, 190), RGB(255, 255, 255), pulse);

		Renderer::drawTextStyled(screenW / 2 - 120, screenH / 2 - 40, L"GPA Snake", 40, titleColor, true, L"Consolas");

		// 提示闪烁（随淡入）
		const float hintPulse = Pulse01(900, 130);
		const COLORREF hintColor = LerpColor(RGB(120, 120, 120), RGB(220, 220, 220), hintPulse * t);
		Renderer::drawTextStyled(screenW / 2 - 200, screenH / 2 + 20, L"按方向键移动，空格加速", 20, hintColor, false, L"Consolas");

		FlushBatchDraw();

		if (t >= 1.0f) break;
		Sleep(16);
	}

	Sleep(250);
}

void Animation::drawIceTeaOverlay(int screenW, int screenH, int nowSec, int iceTeaUntilSec) const
{
	if (nowSec >= iceTeaUntilSec) return;

	const int left = iceTeaUntilSec - nowSec;

	const int w = 300, h = 52;
	const int x = screenW - w - 12;
	const int y = 12;

	const float p = (std::min)(1.0f, left / 10.0f);
	DrawCapsuleBar(x, y, w, h, p, RGB(80, 170, 255), L"冰红茶加速: " + std::to_wstring(left) + L"s");
}

void Animation::drawVirusOverlay(int screenW, int screenH, int nowSec, int virusUntilSec) const
{
	if (nowSec >= virusUntilSec) return;

	const int left = virusUntilSec - nowSec;

	const int w = 300, h = 52;
	const int x = screenW - w - 12;
	const int y = 70;

	const float p = (std::min)(1.0f, left / 10.0f);
	DrawCapsuleBar(x, y, w, h, p, RGB(255, 120, 120), L"病毒减速: " + std::to_wstring(left) + L"s");
}

void Animation::drawMidtermWeekOverlay(int screenW, int screenH, int nowSec, int midtermUntilSec) const
{
	if (nowSec >= midtermUntilSec) return;

	const int left = midtermUntilSec - nowSec;

	const int w = 300, h = 52;
	const int x = screenW - w - 12;
	const int y = 128;

	const float p = (std::min)(1.0f, left / 10.0f);
	DrawCapsuleBar(x, y, w, h, p, RGB(255, 200, 80), L"期中周(掉落x2): " + std::to_wstring(left) + L"s");
}

void Animation::drawFinalWeekOverlay(int screenW, int screenH, int nowSec, int finalUntilSec) const
{
	if (nowSec >= finalUntilSec) return;

	const int left = finalUntilSec - nowSec;

	const int w = 300, h = 52;
	const int x = screenW - w - 12;
	const int y = 186;

	const float p = (std::min)(1.0f, left / 10.0f);
	DrawCapsuleBar(x, y, w, h, p, RGB(180, 140, 255), L"期末周(掉落x2): " + std::to_wstring(left) + L"s");
}

void Animation::startEventResultToast(bool success, int nowSec)
{
	eventToast_.visible = true;
	eventToast_.startSec = nowSec;
	eventToast_.durationSec = 2;
	eventToast_.text = success ? L"事件成功：能力值 +3" : L"事件失败：未完成挑战";
}

void Animation::drawEventResultToast(int screenW, int screenH, int nowSec) const
{
	if (!eventToast_.visible) return;
	if (nowSec < eventToast_.startSec) return;

	const int elapsed = nowSec - eventToast_.startSec;
	if (elapsed >= eventToast_.durationSec)
		return;

	const float total = (float)eventToast_.durationSec;
	const float e = (std::max)(0.0f, (std::min)(total, (float)elapsed));

	float a = 1.0f;
	if (e < 0.25f) a = e / 0.25f;
	else if (e > total - 0.25f) a = (total - e) / 0.25f;

	const int w = 560, h = 56;
	const int x = (screenW - w) / 2;

	const int baseY = 14;
	const int y = baseY - (int)((1.0f - a) * 18);

	const COLORREF fill = LerpColor(RGB(10, 10, 10), RGB(28, 28, 30), a);
	const COLORREF border = LerpColor(RGB(10, 10, 10), RGB(180, 180, 180), a);
	DrawPanelEx(x, y, w, h, fill, border);

	const COLORREF textColor = LerpColor(RGB(120, 120, 120), RGB(245, 245, 245), a);
	Renderer::drawTextStyled(x + 16, y + 16, eventToast_.text, 20, textColor, true, L"Consolas");
}

void Animation::showGameSummaryBlocking(int screenW, int screenH,
	const ScoreSystem& score, const Timer& timer, const EventManager& ev,
	const SettlementManager& settle) const
{
	(void)score; (void)timer; (void)ev;

	while (true)
	{
		cleardevice();

		// 背景轻微呼吸，让结算更“活”
		const float p = 0.2f + 0.8f * Pulse01(1600);
		const COLORREF bg = LerpColor(RGB(16, 16, 18), RGB(24, 24, 28), p);
		DrawPanelEx(0, 0, screenW, screenH, bg, bg);

		settle.draw(screenW, screenH);

		Renderer::drawTextStyled(12, screenH - 30, L"ESC / 鼠标左键：返回", 18, RGB(200, 200, 200), false, L"Consolas");

		FlushBatchDraw();

		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) return;
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) return;

		Sleep(10);
	}
}

void Animation::playGameOutroBlocking(int screenW, int screenH, bool win) const
{
	// 黑屏淡出 + 文案渐显
	const DWORD start = GetTickCount();
	const DWORD dur = 900;

	const std::wstring title =  L"学期结束";
	const std::wstring hint = L"正在生成结算...";

	while (true)
	{
		const DWORD now = GetTickCount();
		const float t = (dur == 0) ? 1.0f : (std::min)(1.0f, (now - start) / (float)dur);

		cleardevice();

		// 从较亮背景 -> 深黑（模拟“退出”）
		const COLORREF bg = LerpColor(RGB(28, 28, 30), RGB(0, 0, 0), t);
		DrawPanelEx(0, 0, screenW, screenH, bg, bg);

		// 文字颜色从暗到亮（渐显）
		const COLORREF titleCol = LerpColor(RGB(60, 60, 60), RGB(245, 245, 245), t);
		const COLORREF hintCol = LerpColor(RGB(40, 40, 40), RGB(200, 200, 200), t);

		Renderer::drawTextStyled(screenW / 2 - 170, screenH / 2 - 26, title, 30, titleCol, true, L"微软雅黑");
		Renderer::drawTextStyled(screenW / 2 - 110, screenH / 2 + 22, hint, 20, hintCol, false, L"微软雅黑");

		FlushBatchDraw();

		if (t >= 1.0f) break;
		Sleep(16);
	}

	Sleep(160);
}