#include "sceneviewer.pch.h"

// Different Games
#include "GameTest2D.h"
#include "SceneViewer.h"

#include "Graphics/ShaderCompiler.h"

int main(const int argcs, const char** argvs)
{
	// Parses the command line for use with the game engine
	cli::CommandLine cmd = cli::parse(argvs, argcs);

	// Allows booting different game classes compiled into the executable
	unique_ptr<AbstractGame> game_ptr;

	if (cli::has_arg(cmd, "game")) {
		std::string game;
		cli::get_string(cmd, "game", game);

		#if FEATURE_D2D
		if (game.compare("2DTest")) {
			game_ptr = make_unique<GameTest2D>();
		} 
		#endif
	}

	// If no game was specified fallback to the sceneviewer.
	if(game_ptr == nullptr) {
		std::string scene;
		cli::get_string(cmd, "scene", scene);

		game_ptr = make_unique<SceneViewer>(scene);
	}

	return GameEngine::run_game(NULL, cmd, 1, std::move(game_ptr));
}
