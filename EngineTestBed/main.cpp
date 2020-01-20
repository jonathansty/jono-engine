#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "AbstractGame.h"

class TestGame : public AbstractGame
{
public:
	void GameStart() override
	{
		_test = std::make_shared<Bitmap>(String("Resources/Pickups/coinBronze.png"));
		_sound = new Sound(String("Resources/Sound/Entity/Jump.wav"));
	}

	void GameEnd() override
	{
		_sound->Stop();
		delete _sound;
	}

	void GamePaint(RECT rect) override
	{
		GameEngine::Instance()->DrawSolidBackground(COLOR(0, 0, 0));
		GameEngine::Instance()->SetColor(COLOR(255, 0, 0));
		GameEngine::Instance()->DrawRect(0, 0, 100, 100);

		GameEngine::Instance()->DrawBitmap(_test.get());

		if (_sound->GetPlayState() != Sound::PlayState::Playing)
		{
			_sound->Play();
		}
	}
private:
	std::shared_ptr<Bitmap> _test;
	Sound* _sound;

};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	return RunGame(hInstance, iCmdShow, new TestGame());
}
