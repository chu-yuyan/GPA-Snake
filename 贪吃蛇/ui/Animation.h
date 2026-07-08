#pragma once

#include <string>

class ScoreSystem;
class EventManager;
class Timer;
class SettlementManager;

enum class AnimDialogAction
{
	None,
	Accept,
	Skip
};

struct AnimToast
{
	bool visible = false;
	int startSec = 0;
	int durationSec = 0;
	std::wstring text;
};

class Animation
{
public:
	void playGameIntroBlocking(int screenW, int screenH) const;

	//  游戏结束退出动画（在结算前播放）
	void playGameOutroBlocking(int screenW, int screenH, bool win) const;

	void drawIceTeaOverlay(int screenW, int screenH, int nowSec, int iceTeaUntilSec) const;
	void drawVirusOverlay(int screenW, int screenH, int nowSec, int virusUntilSec) const;

	void drawMidtermWeekOverlay(int screenW, int screenH, int nowSec, int midtermUntilSec) const;
	void drawFinalWeekOverlay(int screenW, int screenH, int nowSec, int finalUntilSec) const;

	void drawEventConfirmDialog(int screenW, int screenH, int nowSeconds, const EventManager& ev) const;

	AnimDialogAction hitTestEventConfirmDialog(int screenW, int screenH, const EventManager& ev,
		int mouseX, int mouseY, bool mouseClick) const;

	void startEventResultToast(bool success, int nowSec);
	void drawEventResultToast(int screenW, int screenH, int nowSec) const;

	void showGameSummaryBlocking(int screenW, int screenH,
		const ScoreSystem& score, const Timer& timer, const EventManager& ev,
		const SettlementManager& settle) const;

private:
	struct RectI { int x = 0, y = 0, w = 0, h = 0; };
	void calcEventDialogLayout(int screenW, int screenH, RectI& dialog, RectI& okBtn, RectI& skipBtn) const;
	static const wchar_t* eventName(int e);

private:
	AnimToast eventToast_{};
};