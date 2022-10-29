#include "PathFindingTest.pch.h"

// Different Games
#include "PathFindingGame.h"

int main(int argcs, char** argvs)
{
	unique_ptr<AbstractGame> game_ptr = make_unique<PathFindingGame>();

	auto cmd = cli::parse(argvs, argcs);
	return GameEngine::Run(NULL, cmd, 1, std::move(game_ptr));
}
