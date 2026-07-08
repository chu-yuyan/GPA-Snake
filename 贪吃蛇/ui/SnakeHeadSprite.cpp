#include "SnakeHeadSprite.h"

bool SnakeHeadSprite::inited_ = false;
IMAGE SnakeHeadSprite::imgs_[(int)SnakeSkin::COUNT * 4]{};
bool SnakeHeadSprite::loaded_[(int)SnakeSkin::COUNT * 4]{};

int SnakeHeadSprite::index(SnakeSkin s, Direction d)
{
	return ((int)s) * 4 + (int)d;
}

void SnakeHeadSprite::loadOne(int idx, const wchar_t* relativePath, int targetW, int targetH)
{
	loaded_[idx] = false;

	IMAGE raw;
	loadimage(&raw, relativePath);

	const int rw = raw.getwidth();
	const int rh = raw.getheight();
	if (rw <= 0 || rh <= 0) return;

	imgs_[idx].Resize(targetW, targetH);
	SetWorkingImage(&imgs_[idx]);
	cleardevice();
	putimage(0, 0, &raw);
	SetWorkingImage(nullptr);

	loaded_[idx] = (imgs_[idx].getwidth() > 0 && imgs_[idx].getheight() > 0);
}

void SnakeHeadSprite::init(int cellSize)
{
	if (inited_) return;
	inited_ = true;

	for (int i = 0; i < (int)SnakeSkin::COUNT * 4; ++i)
		loaded_[i] = false;

	const int target = cellSize; // 1x1

	
	// ·½Ïò£ºup/down/left/right
	loadOne(index(SnakeSkin::Red, Direction::Up), L"./assets/heads/red_up.png", target, target);
	loadOne(index(SnakeSkin::Red, Direction::Down), L"./assets/heads/red_down.png", target, target);
	loadOne(index(SnakeSkin::Red, Direction::Left), L"./assets/heads/red_left.png", target, target);
	loadOne(index(SnakeSkin::Red, Direction::Right), L"./assets/heads/red_right.png", target, target);

	loadOne(index(SnakeSkin::Green, Direction::Up), L"./assets/heads/green_up.png", target, target);
	loadOne(index(SnakeSkin::Green, Direction::Down), L"./assets/heads/green_down.png", target, target);
	loadOne(index(SnakeSkin::Green, Direction::Left), L"./assets/heads/green_left.png", target, target);
	loadOne(index(SnakeSkin::Green, Direction::Right), L"./assets/heads/green_right.png", target, target);

	loadOne(index(SnakeSkin::Blue, Direction::Up), L"./assets/heads/blue_up.png", target, target);
	loadOne(index(SnakeSkin::Blue, Direction::Down), L"./assets/heads/blue_down.png", target, target);
	loadOne(index(SnakeSkin::Blue, Direction::Left), L"./assets/heads/blue_left.png", target, target);
	loadOne(index(SnakeSkin::Blue, Direction::Right), L"./assets/heads/blue_right.png", target, target);

	loadOne(index(SnakeSkin::Purple, Direction::Up), L"./assets/heads/purple_up.png", target, target);
	loadOne(index(SnakeSkin::Purple, Direction::Down), L"./assets/heads/purple_down.png", target, target);
	loadOne(index(SnakeSkin::Purple, Direction::Left), L"./assets/heads/purple_left.png", target, target);
	loadOne(index(SnakeSkin::Purple, Direction::Right), L"./assets/heads/purple_right.png", target, target);
}

const IMAGE* SnakeHeadSprite::get(SnakeSkin s, Direction d)
{
	const int idx = index(s, d);
	if (idx < 0 || idx >= (int)SnakeSkin::COUNT * 4) return nullptr;
	return loaded_[idx] ? &imgs_[idx] : nullptr;
}
