#pragma once

#include "Snake.h"
#include <array>

enum class FoodType
{
	// 课程（计入GPA）
	CALCULUS,        // 数分 6
	LINEAR_ALGEBRA,  // 线代 5
	PHYSICS,         // 物理 3
	ENGLISH,         // 英语 2
	PHYSICS_LAB,     // 物理实验 1
	PE,              // 体育 1
	CPP,             // 高程 2
	MILITARY,        // 军事理论 2
	AI,              // 人工智能 2
	RESEARCH,        // 科研前沿 2
	POLITICS,        // 形策 0.5

	// 道具
	ICE_TEA,         // 冰红茶（加速buff）
	VIRUS,           // 病毒（减速debuff）

	// 事件食物（仅当事件开始选择“是”才进入掉落池；是否计入GPA后续由ScoreSystem决定）
	CET4,            // 四六级
	SITP,            // SITP
	PROJECT,         // 大作业
	ELECTIVE,        // 选修课
	CONTEST,         // 竞赛

	COUNT
};

class Food
{
public:
	Position pos{ 5, 5 };                 //  左上角格子
	FoodType type = FoodType::CALCULUS;

	int spawnTick = 0;
	int ttlTicks = 80;

	static constexpr int Size = 2;        //  2x2，占4格

	bool expired(int nowTick) const { return nowTick - spawnTick >= ttlTicks; }

	std::array<Position, 4> cells() const
	{
		return { Position{pos.x, pos.y},
				 Position{pos.x + 1, pos.y},
				 Position{pos.x, pos.y + 1},
				 Position{pos.x + 1, pos.y + 1} };
	}

	bool contains(const Position& p) const
	{
		for (const auto& c : cells())
		{
			if (c == p) return true;
		}
		return false;
	}

	void draw(int cellSize) const;
};