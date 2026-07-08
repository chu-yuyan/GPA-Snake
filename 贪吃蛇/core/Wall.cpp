#include "Wall.h"
#include <graphics.h>
#include <algorithm>

void Wall::buildBorder(int gridW, int gridH)
{
	blocks.clear();

	for (int x = 0; x < gridW; ++x)
	{
		blocks.push_back({ x, 0 });
		blocks.push_back({ x, gridH - 1 });
	}
	for (int y = 1; y < gridH - 1; ++y)
	{
		blocks.push_back({ 0, y });
		blocks.push_back({ gridW - 1, y });
	}
}

bool Wall::contains(const Position& p) const
{
	return std::find(blocks.begin(), blocks.end(), p) != blocks.end();
}

void Wall::addBlock(const Position& p)
{
	if (contains(p)) return;
	blocks.push_back(p);
}

void Wall::addBlocks(const std::vector<Position>& ps)
{
	for (const auto& p : ps) addBlock(p);
}

void Wall::draw(int cellSize) const
{
	setlinecolor(BLACK);
	setfillcolor(RGB(80, 80, 80));

	for (const auto& b : blocks)
	{
		int x = b.x * cellSize;
		int y = b.y * cellSize;
		//  避免“多一像素”边缘
		fillrectangle(x, y, x + cellSize - 1, y + cellSize - 1);
	}
}