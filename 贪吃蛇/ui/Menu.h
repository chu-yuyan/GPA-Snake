#pragma once

#include "../core/Snake.h"

class Bgm;

class Menu
{
public:
	struct Result
	{
		bool startGame = false;
		bool quit = false;
		SnakeSkin skin = SnakeSkin::Red;
	};

	//  传入 bgm，用于在切换皮肤时即时切歌
	Result run(int screenW, int screenH, Bgm& bgm) const;
};