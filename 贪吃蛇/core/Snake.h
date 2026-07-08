#pragma once

#include <deque>

struct Position
{
	int x = 0;
	int y = 0;

	bool operator==(const Position& r) const { return x == r.x && y == r.y; }
};

enum class Direction
{
	Up,
	Down,
	Left,
	Right
};

enum class SnakeSkin
{
	Red = 0,
	Green,
	Blue,
	Purple,
	COUNT
};

class Snake
{
public:
	Snake();

	void reset(const Position& start, int initialLen, Direction startDir);

	void setDirection(Direction d);
	void move();
	void grow();

	Position head() const { return body.front(); }
	int length() const { return (int)body.size(); }
	Direction direction() const { return dir; }

	bool occupies(const Position& p) const;
	bool checkSelfCollision() const;

	void draw(int cellSize) const;

	void setSkin(SnakeSkin s) { skin_ = s; }
	SnakeSkin skin() const { return skin_; }

	//  供死亡转换（变墙/变食物）使用
	const std::deque<Position>& bodyCells() const { return body; }

private:
	std::deque<Position> body;
	Direction dir = Direction::Right;
	bool pendingGrow = false;
	SnakeSkin skin_ = SnakeSkin::Red; //  默认红色
};