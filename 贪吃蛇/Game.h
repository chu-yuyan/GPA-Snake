#pragma once

#include <vector>

#include "core/Snake.h"
#include "core/Food.h"
#include "core/Wall.h"
#include "core/EventManager.h"
#include "core/ScoreSystem.h"

#include "ui/UIManager.h"
#include "ui/Animation.h"
#include "ui/SettlementManager.h"
#include "system/Input.h"
#include "system/Timer.h"
#include "system/SaveManager.h"

class Game
{
public:
	Game(int windowW = 1300, int windowH = 700, int cellSize = 20, SnakeSkin skin = SnakeSkin::Green);
	void run();

private:
	void input();
	void update();
	void render();

	void resetRound();
	void spawnFoods(int count);
	void spawnEventFoodsOnly(int count, FoodType eventFoodType);
	FoodType mapEventToFood(GameEvent e) const;
	bool isCellBlocked(const Position& p) const;

	int calcSleepMs() const;

private:
	int windowW;
	int windowH;
	int cellSize;

	int uiPanelW = 300;
	int playOffsetX = 0;
	int playW = 0;
	int playH = 0;

	int gridW;
	int gridH;

	bool running = true;

	Input inputSys;
	Timer timer;
	SaveManager save;

	Snake snake;
	std::vector<Food> foods;
	Wall wall;
	EventManager eventMgr;
	ScoreSystem score;

	UIManager ui;
	SettlementManager settle_;

	int eatCount = 0;
	//  游戏总时长（秒）。调试时可临时改小（例如 30），调完再改回 180。
	static constexpr int TotalTimeSeconds = 30;

	//  期中/期末周触发时刻（按时长比例缩放），避免缩短总时长后事件不触发。
	bool midtermTriggered_ = false;
	bool finalTriggered_ = false;
	static constexpr double MidtermAtRatio = 90.0 / 180.0;
	static constexpr double FinalAtRatio = 160.0 / 180.0;
	static constexpr int MidtermDurationSeconds = 10;
	static constexpr int FinalDurationSeconds = 20;

	int dbgMouseX = -1;
	int dbgMouseY = -1;
	bool dbgMouseClick = false;

	int deathFoodDebt = 0;

	InputState lastInput_{};

	int iceTeaUntilSec_ = 0;
	int virusUntilSec_ = 0;

	int midtermUntilSec_ = 0; // 期中周
	int finalUntilSec_ = 0;   // 期末周

	Animation anim_;

	EventState prevEventState_ = EventState::IDLE;
};