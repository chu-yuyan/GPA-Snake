#pragma once

#include "../core/Snake.h"
#include <graphics.h>

// 蛇头图片集中管理：按 cellSize 缩放到 1x1 单元格
class SnakeHeadSprite
{
public:
	static void init(int cellSize);
	static const IMAGE* get(SnakeSkin s, Direction d);

private:
	static int index(SnakeSkin s, Direction d);
	static void loadOne(int idx, const wchar_t* relativePath, int targetW, int targetH);

private:
	static bool inited_;
	static IMAGE imgs_[(int)SnakeSkin::COUNT * 4];
	static bool loaded_[(int)SnakeSkin::COUNT * 4];
};
