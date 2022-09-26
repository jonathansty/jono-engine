#pragma once
#include "Core/Math.h"

class RenderWorldInstance;

struct VisibilityParams
{
	Math::Frustum frustum;
};

class VisibilityManager
{
public:
	VisibilityManager() {}
	~VisibilityManager() {}

	VisibilityManager(VisibilityManager const&) = delete;
	VisibilityManager& operator=(VisibilityManager const&) = delete;

	void reset();
	void add_instance(RenderWorldInstance* inst);
	void run(VisibilityParams const&);

	std::vector<RenderWorldInstance*> const& get_visible_instances() const { return _visible_instances; }

private:

	std::vector<RenderWorldInstance*> _all_instances;
	std::vector<RenderWorldInstance*> _visible_instances;

};