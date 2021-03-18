#include "sceneviewer.pch.h"

#include "HelloWorld.h"
#include "SceneViewer.h"

int main(const int argcs, const char** argvs)
{
	std::string scene;
	auto cmd = cli::parse(argvs, argcs);
	cli::get_string(cmd, "scene", scene);

	return GameEngine::run_game(NULL, cmd, 1, new SceneViewer(scene));
}
