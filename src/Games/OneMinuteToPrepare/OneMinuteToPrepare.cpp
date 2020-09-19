#include "OneMinuteToPrepare.h"

#include <Engine.hpp>

#include "Components/player_movement_component.h"

using namespace framework;

//TODO: Fix GameStart so that we already now the viewport size when invoking that
class Game final : public AbstractGame
{
private:
	double _timer;
	std::shared_ptr<framework::World> _world;

	uint64_t _frames;
	bool _has_init;

public:
	virtual void GameInitialize(GameSettings& gameSettings) 
	{
		gameSettings.m_WindowTitle = String("Project Athena");
	}								

	virtual void GameStart(void) 
	{
		logging::logf("Project Athena: Started\n");
		game_engine::instance()->EnablePhysicsDebugRendering(true);

		_world = std::make_shared<World>();

		// Create the player
		{
			auto entity = _world->create_entity();
			entity->set_name("Player");
			entity->create_component<player_movement_component>();
			auto comp = entity->create_component<rigid_body_2d>(BodyType::KINEMATIC);
			std::shared_ptr<PhysicsActor> actor = comp->get_actor();
			actor->SetGravityScale(0.0);
			actor->SetFixedRotation(true);
			actor->AddCircleShape(10.0);
		}

		// Create balls
		{
			for (int i = 0; i < 5; ++i)
			{
				auto entity = _world->create_entity();
				entity->set_name("Ball");
				rigid_body_2d* comp = entity->create_component<rigid_body_2d>(BodyType::DYNAMIC);
				std::shared_ptr<PhysicsActor> actor = comp->get_actor();
				actor->AddCircleShape(25.0);
				actor->SetPosition({ 100.0 + (double)i * 55.0, 50.0 });
				actor->SetActive(true);
			}
		}

		// Create a floor
		{
			auto entity = _world->create_entity();
			entity->set_name("Floor");
			rigid_body_2d* comp = entity->create_component<rigid_body_2d>(BodyType::KINEMATIC);
			std::shared_ptr<PhysicsActor> actor = comp->get_actor();

			ImVec2 size = game_engine::instance()->get_viewport_size();
			actor->AddBoxShape(size.x, 5.0);
			actor->SetPosition({ size.x / 2.0, size.y - 5.0 });
		}
	}

	virtual void GameEnd(void) 
	{
		logging::logf("Project Athena: Stopped\n");
	}																

	virtual void GamePaint(RECT rect) 
	{
		game_engine* ctx = game_engine::instance();
		ctx->DrawSolidBackground(COLOR{ 128,0,64 });

		ctx->set_color(COLOR{ 255,255,255 });
		ctx->DrawString(String("Hello... World..."), 25, 25);


		ctx->SetWorldMatrix(MATRIX3X2::CreateRotationMatrix(XMConvertToRadians(_timer)) * MATRIX3X2::CreateTranslationMatrix(100, 100));
		ctx->DrawString(String("Rotating Text!"), 0, 0);

		ctx->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
		_world->render();
	}

	virtual void GameTick(double deltaTime) 
	{

		_timer += deltaTime * 2.5f;
		_world->update(deltaTime);
	}													


	virtual void DebugUI() 
	{

	}
};

int main()
{
	return game_engine::run_game(0, 1, new Game());
}