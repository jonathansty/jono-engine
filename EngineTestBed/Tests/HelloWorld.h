#pragma once

#include "AbstractGame.h"

class HelloWorldGame : public AbstractGame
{
public:
	void GameInitialize(GameSettings& gameSettings)
	{
		gameSettings.EnableConsole(true);
	}
	void GameStart() override;

	void GameEnd() override;

	void GamePaint(RECT rect) override;

	void DebugUI() override;

private:
	std::shared_ptr<Bitmap> _test;
};

struct SimpleData
{
	int a;
	int b;
	float c;
	std::string name;
};

struct OtherData : public SimpleData
{
	int b = 0;
};


