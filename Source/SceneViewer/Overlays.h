#pragma once

#include "Framework/framework.h"
#include "Debug/OverlayManager.h"

namespace framework {

class EntityDebugOverlay final : public DebugOverlay
{
public:
	EntityDebugOverlay(framework::World* world);
	~EntityDebugOverlay() {}

	void render_tree(framework::EntityHandle ent);

	#ifdef ENABLE_RTTR
	void render_object(rttr::instance& obj);
	#endif

	virtual void RenderOverlay() override;

	virtual void RenderViewport() override;

private:
	framework::EntityHandle _selected;
	framework::World* _world;
};

}