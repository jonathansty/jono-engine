#include "PathFindingTest.pch.h"

// Different Games
#include "PathFindingGame.h"
#include "GlobalContext.h"
#include "Types/TypeManager.h"

int main(int argcs, char** argvs)
{
    unique_ptr<AbstractGame> game_ptr =  unique_ptr<AbstractGame>(GetGlobalContext()->m_TypeManager->CreateObject<PathFindingGame>(PathFindingGame::GetStaticType()));

    auto cmd = cli::parse(argvs, argcs);
    return GameEngine::Run(nullptr, cmd, 1, std::move(game_ptr));
}
