#pragma once

#include "Framework/framework.h"
#include "debug_overlays/OverlayManager.h"

namespace framework {

class EntityDebugOverlay final : public DebugOverlay
{
public:
	EntityDebugOverlay(framework::World* world);
	~EntityDebugOverlay() {}

	void render_tree(framework::EntityHandle ent);

	void render_object(rttr::instance& obj);

	virtual void render_overlay() override;

private:
	framework::EntityHandle _selected;
	framework::World* _world;
};

}