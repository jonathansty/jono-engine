#include "stdafx.h"

#include "HelloWorld.h"
#include "Test3D.h"

int main(const char** argvs, const int argcs)
{
	return GameEngine::run_game(NULL, cli::parse(GetCommandLineA()), 1, new HelloWorldGame());
}
