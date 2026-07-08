#include "Food.h"
#include <graphics.h>

#include "../ui/FoodSprite.h" //  新增：食物图片管理

static COLORREF FoodColor(FoodType t)
{
	// （保留你原来的颜色表不动...）
	switch (t)
	{
	case FoodType::CALCULUS: return RGB(60, 160, 255);
	case FoodType::LINEAR_ALGEBRA: return RGB(80, 180, 255);
	case FoodType::PHYSICS: return RGB(120, 200, 255);
	case FoodType::ENGLISH: return RGB(255, 220, 120);
	case FoodType::PHYSICS_LAB: return RGB(255, 200, 160);
	case FoodType::PE: return RGB(120, 255, 160);
	case FoodType::CPP: return RGB(160, 255, 120);
	case FoodType::MILITARY: return RGB(200, 200, 200);
	case FoodType::AI: return RGB(180, 120, 255);
	case FoodType::RESEARCH: return RGB(255, 160, 220);
	case FoodType::POLITICS: return RGB(255, 180, 120);

	case FoodType::ICE_TEA: return RGB(205, 133, 63);
	case FoodType::VIRUS: return RGB(255, 80, 80);

	case FoodType::CET4: return RGB(255, 255, 160);
	case FoodType::SITP: return RGB(160, 255, 255);
	case FoodType::PROJECT: return RGB(255, 160, 160);
	case FoodType::ELECTIVE: return RGB(200, 160, 255);
	case FoodType::CONTEST: return RGB(160, 255, 160);

	default: return WHITE;
	}
}

void Food::draw(int cellSize) const
{
	const int px = pos.x * cellSize;
	const int py = pos.y * cellSize;

	const int w = cellSize * Size;
	const int h = cellSize * Size;

	//  优先画图片（你把图片放到 `assets/foods/*.png` 后这里会自动生效）
	// ⚠️ 注意：当前使用 putimage(x,y,img) 直接绘制原始尺寸。
	// 后续如果你希望严格适配 2x2 的 w/h，需要再实现“按比例缩放 + 居中”的绘制。
	if (const IMAGE* img = FoodSprite::get(type))
	{
		// 【图片绘制位置】这里就是你将来替换成“缩放绘制到 w/h”的位置
		putimage(px, py, (IMAGE*)img);
		return;
	}

	// fallback：无图片时仍显示色块，便于调试
	setlinecolor(BLACK);
	setfillcolor(FoodColor(type));
	fillrectangle(px + 2, py + 2, px + w - 2, py + h - 2);
}