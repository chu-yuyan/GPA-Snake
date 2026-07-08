#include "Snake.h"
#include <graphics.h>
#include "../ui/SnakeHeadSprite.h"

Snake::Snake()
{
	reset({ 10, 10 }, 3, Direction::Right);
}

void Snake::reset(const Position& start, int initialLen, Direction startDir)
{
	body.clear();
	dir = startDir;
	pendingGrow = false;

	for (int i = 0; i < initialLen; ++i)
	{
		body.push_back({ start.x - i, start.y });
	}
}

void Snake::setDirection(Direction d)
{
	if ((dir == Direction::Up && d == Direction::Down) ||
		(dir == Direction::Down && d == Direction::Up) ||
		(dir == Direction::Left && d == Direction::Right) ||
		(dir == Direction::Right && d == Direction::Left))
		return;

	dir = d;
}

void Snake::move()
{
	Position h = head();
	switch (dir)
	{
	case Direction::Up:    h.y -= 1; break;
	case Direction::Down:  h.y += 1; break;
	case Direction::Left:  h.x -= 1; break;
	case Direction::Right: h.x += 1; break;
	}

	body.push_front(h);

	if (pendingGrow)
	{
		pendingGrow = false;
	}
	else
	{
		body.pop_back();
	}
}

void Snake::grow()
{
	pendingGrow = true;
}

bool Snake::occupies(const Position& p) const
{
	for (const auto& c : body)
	{
		if (c == p) return true;
	}
	return false;
}

bool Snake::checkSelfCollision() const
{
	const Position h = head();
	for (size_t i = 1; i < body.size(); ++i)
	{
		if (body[i] == h)
			return true;
	}
	return false;
}

static COLORREF SkinToColor(SnakeSkin s)
{
	switch (s)
	{
	case SnakeSkin::Red:    return RGB(240, 60, 60);
	case SnakeSkin::Green:  return RGB(0, 200, 0);
	case SnakeSkin::Blue:   return RGB(30, 120, 255);
	case SnakeSkin::Purple: return RGB(160, 80, 220);
	default:                return RGB(240, 60, 60);
	}
}

// 唯一保留的 draw 函数：身体纯色 + 蛇头图片覆盖
void Snake::draw(int cellSize) const
{
	setlinecolor(BLACK);

	// 1) 绘制身体（包括头部先用纯色占位）
	setfillcolor(SkinToColor(skin_));
	for (const auto& c : body)
	{
		const int x = c.x * cellSize;
		const int y = c.y * cellSize;
		fillrectangle(x, y, x + cellSize, y + cellSize);
	}

	// 2) 绘制蛇头图片（如果有，覆盖头部区域）
	const IMAGE* headImg = SnakeHeadSprite::get(skin_, dir);
	if (headImg)
	{
		const Position& h = head();
		const int x = h.x * cellSize;
		const int y = h.y * cellSize;
		putimage(x, y, (IMAGE*)headImg);
	}
}