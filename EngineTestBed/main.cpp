#include "stdafx.h"

#include "HelloWorld.h"
#include "Test3D.h"

int main()
{
	return game_engine::run_game(NULL, 1, new Hello3D());
}
