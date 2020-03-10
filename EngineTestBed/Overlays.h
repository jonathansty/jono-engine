#pragma once

#include "Framework/framework.h"
#include "DebugOverlays/OverlayManager.h"

class EntityDebugOverlay final : public DebugOverlay
{
public:
	EntityDebugOverlay(framework::World* world);
	~EntityDebugOverlay() {}

	void RenderTree(framework::Entity* ent);

	void RenderObject(rtti::Object& obj);

	virtual void render_overlay() override;

private:
	framework::Entity* _selected;
	framework::World* _world;
};