#pragma once

#include "Framework/framework.h"
#include "DebugOverlays/OverlayManager.h"

class EntityDebugOverlay final : public DebugOverlay
{
public:
	EntityDebugOverlay(framework::World* world);
	~EntityDebugOverlay() {}

	void render_tree(framework::Entity* ent);

	void render_object(rtti::Object& obj);

	virtual void render_overlay() override;

private:
	framework::Entity* _selected;
	framework::World* _world;
};