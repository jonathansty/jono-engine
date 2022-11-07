#pragma once
#ifdef ENABLE_RTTR
#include <rttr/registration>
#endif

namespace framework
{
	class Entity;

	class ENGINE_API Component
	{
		friend class World;

#ifdef ENABLE_RTTR
		RTTR_REGISTRATION_FRIEND
		RTTR_ENABLE()
#endif

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