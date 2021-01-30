#include "testbed.stdafx.h"

#include "HelloWorld.h"
#include "Test3D.h"

int main(const int argcs, const char** argvs)
{
	return GameEngine::run_game(NULL, cli::parse(argvs, argcs), 1, new HelloWorldGame());
	//return GameEngine::run_game(NULL, cli::parse(argvs, argcs), 1, new Hello3D());
}
