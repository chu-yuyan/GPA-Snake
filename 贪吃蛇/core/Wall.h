#pragma once

#include <vector>
#include "Snake.h"

class Wall
{
public:
	void buildBorder(int gridW, int gridH);
	bool contains(const Position& p) const;
	void draw(int cellSize) const;

	//  新增：追加墙块（用于撞墙后蛇身变墙）
	void addBlocks(const std::vector<Position>& ps);
	void addBlock(const Position& p);

private:
	std::vector<Position> blocks; // 包含边界墙 + 动态墙
};