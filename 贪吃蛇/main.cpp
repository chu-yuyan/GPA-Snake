#include <graphics.h>
#include "Game.h"
#include "ui/Menu.h"
#include "system/Bgm.h"

int main()
{
	initgraph(1300, 700, EX_SHOWCONSOLE);

	setbkcolor(WHITE);
	cleardevice();

	Bgm bgm;

	//  默认红色
	bgm.playLoop(L"./assets/bgms/red.wav");

	while (true)
	{
		cleardevice();

		Menu menu;
		const auto r = menu.run(1300, 700, bgm); //  传入 bgm

		if (r.quit)
			break;

		if (r.startGame)
		{
			Game game(1300, 700, 20, r.skin);
			game.run();
		}
	}

	bgm.stop();
	closegraph();
	return 0;
}