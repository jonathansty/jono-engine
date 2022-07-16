#include "PathFindingTest.pch.h"

// Different Games
#include "PathFindingGame.h"

int main(const int argcs, const char** argvs)
{
	unique_ptr<AbstractGame> game_ptr = make_unique<PathFindingGame>();
	return GameEngine::run_game(NULL, {}, 1, std::move(game_ptr));
}
