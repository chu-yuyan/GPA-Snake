#pragma once

#include "../core/Snake.h"

struct InputState
{
	bool quit = false;
	bool dirChanged = false;
	Direction dir = Direction::Right;

	bool spaceDown = false; 
};

class Input
{
public:
	InputState poll() const;
};