#pragma once

#include "game-engine/board.h"

class BoardInitializer
{
public:
	virtual ~BoardInitializer() {}

	virtual void InitializeBoard(int rand_seed, GameEngine::Board & board) = 0;
};