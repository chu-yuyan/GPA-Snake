#pragma once

#include "../core/ScoreSystem.h"
#include "../system/Timer.h"
#include "../core/EventManager.h"

class UIManager
{
public:
	void drawBackgroundGrid(int windowW, int windowH, int cellSize) const;
	void drawHUD(const ScoreSystem& score, const Timer& timer, const EventManager& ev, int snakeLen) const;

	void drawLeftPanel(int panelW, int windowH,
		const ScoreSystem& score, const Timer& timer, const EventManager& ev, int snakeLen) const;
};