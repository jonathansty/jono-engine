#pragma once
#include <rttr/registration>

namespace framework
{
	class Entity;

	class Component
	{
		RTTR_REGISTRATION_FRIEND
		RTTR_ENABLE()
	public:
		Component();

		virtual ~Component();

		virtual void on_attach(Entity* _ent) { _parent = _ent; }
		virtual void on_detach(Entity* _ent) { _parent = nullptr; }

		virtual void update(float dt) {}
		virtual void render() {}

		bool is_active() const { return _active; }
		Entity* get_entity() const
		{
			return _parent;
		}


	private:
		bool _active;
		Entity* _parent;


		friend class EntityDebugOverlay;

	};

}