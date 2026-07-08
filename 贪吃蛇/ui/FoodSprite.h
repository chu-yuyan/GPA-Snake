#pragma once

#include "../core/Food.h"
#include <graphics.h>

//  食物图片集中管理：加载时缩放到目标尺寸，draw时直接 putimage
class FoodSprite
{
public:
	//  需要 cellSize：把图片缩放到 Food::Size * cellSize
	static void init(int cellSize);

	static const IMAGE* get(FoodType t);

private:
	static void loadOne(FoodType t, const wchar_t* relativePath, int targetW, int targetH);

private:
	static bool inited_;
	static IMAGE imgs_[(int)FoodType::COUNT];
	static bool loaded_[(int)FoodType::COUNT];
};