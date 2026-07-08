#include "UIManager.h"
#include "Renderer.h"

#include <graphics.h>
#include <string>

static const wchar_t* EventNameLocal(GameEvent e)
{
	switch (e)
	{
	case GameEvent::ELECTIVE: return L"选修课";
	case GameEvent::SITP: return L"SITP";
	case GameEvent::PROJECT: return L"大作业";
	case GameEvent::CET4: return L"四六级";
	case GameEvent::CONTEST: return L"学科竞赛";
	default: return L"";
	}
}

static std::wstring ToFixed2(float v)
{
	std::wstring s = std::to_wstring(v);
	// 形如 "3.500000" -> "3.50"
	auto dot = s.find(L'.');
	if (dot == std::wstring::npos) return s;
	if (dot + 3 < s.size()) s.resize(dot + 3);
	return s;
}

void UIManager::drawBackgroundGrid(int windowW, int windowH, int cellSize) const
{
	// 网格颜色
	setlinecolor(RGB(235, 235, 235));
	for (int x = 0; x <= windowW; x += cellSize) line(x, 0, x, windowH);
	for (int y = 0; y <= windowH; y += cellSize) line(0, y, windowW, y);
}

void UIManager::drawHUD(const ScoreSystem& score, const Timer& timer, const EventManager& /*ev*/, int snakeLen) const
{
	const double happyPred = score.happinessPreview();

	std::wstring line1 =
		L"Len: " + std::to_wstring(snakeLen) +
		L"  GPA: " + ToFixed2(score.gpa()) +
		L"  Ability: " + std::to_wstring(score.ability()) +
		L"  Overwork: " + std::to_wstring(score.overworkDeaths()) +
		L"  Happy: " + std::to_wstring((int)happyPred) +
		L"  Time: " + std::to_wstring(timer.seconds());

	Renderer::drawPanel(6, 6, 980, 34);
	Renderer::drawText(12, 10, line1, 18);
}

void UIManager::drawLeftPanel(int panelW, int windowH,
	const ScoreSystem& score, const Timer& timer, const EventManager& ev, int snakeLen) const
{
	Renderer::drawPanel(0, 0, panelW, windowH);

	int y = 12;
	Renderer::drawText(12, y, L"大学生生存模拟器", 20); y += 28;

	Renderer::drawText(12, y, L"Len: " + std::to_wstring(snakeLen), 18); y += 22;
	Renderer::drawText(12, y, L"Time(s): " + std::to_wstring(timer.seconds()), 18); y += 22;

	Renderer::drawText(12, y, L"GPA: " + ToFixed2(score.gpa()), 18); y += 22;
	Renderer::drawText(12, y, L"能力值: " + std::to_wstring(score.ability()), 18); y += 22;
	Renderer::drawText(12, y, L"死亡: " + std::to_wstring(score.deathCount()), 18); y += 22;
	Renderer::drawText(12, y, L"卷死: " + std::to_wstring(score.overworkDeaths()), 18); y += 22;
	Renderer::drawText(12, y, L"幸福感: " + std::to_wstring((int)score.happinessPreview()), 18); y += 22;

	y += 8;
	Renderer::drawText(12, y, L"课程成绩", 18); y += 22;

	auto drawCourseLine = [&](FoodType c)
		{
			std::wstring name = ScoreSystem::courseDisplayName(c);
			std::wstring g = std::to_wstring(score.courseGrade(c));
			// "2.000000" -> "2.0"
			auto dot = g.find(L'.');
			if (dot != std::wstring::npos && dot + 2 < g.size()) g.resize(dot + 2);

			Renderer::drawText(12, y, name + L": " + g, 16);
			y += 18;
		};

	// 固定顺序
	drawCourseLine(FoodType::CALCULUS);
	drawCourseLine(FoodType::LINEAR_ALGEBRA);
	drawCourseLine(FoodType::PHYSICS);
	drawCourseLine(FoodType::ENGLISH);
	drawCourseLine(FoodType::PHYSICS_LAB);
	drawCourseLine(FoodType::PE);
	drawCourseLine(FoodType::CPP);
	drawCourseLine(FoodType::MILITARY);
	drawCourseLine(FoodType::AI);
	drawCourseLine(FoodType::RESEARCH);
	drawCourseLine(FoodType::POLITICS);

	//  选修课：只有启用后才显示
	if (ev.electiveEnabled())
		drawCourseLine(FoodType::ELECTIVE);

	// 事件状态（顺带显示进度）
	y += 8;
	if (ev.state() == EventState::ACTIVE)
	{
		Renderer::drawText(12, y, L"事件中: " + std::wstring(EventNameLocal(ev.activeEvent())), 16); y += 18;
		Renderer::drawText(12, y, L"剩余: " + std::to_wstring(ev.activeRemainingSeconds(timer.seconds())) + L"s", 16); y += 18;
		Renderer::drawText(12, y, L"进度: " + std::to_wstring(ev.activeProgress()) + L"/5", 16); y += 18;
	}
}

