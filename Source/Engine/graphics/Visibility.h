#pragma once
#include "Core/Math.h"

class RenderWorldInstance;

enum VisibilityFrustum
{
	VisiblityFrustum_Main,
	VisiblityFrustum_CSM0,
	VisiblityFrustum_CSM1,
	VisiblityFrustum_CSM2,
	VisiblityFrustum_CSM3,
	VisibilityFrustum_Count
};

struct VisibilityParams
{
	std::array<Math::Frustum, VisibilityFrustum_Count> frustum;
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

	std::vector<RenderWorldInstance*> const& get_visible_instances(VisibilityFrustum frustum = VisiblityFrustum_Main) const { return _visible_instances[frustum]; }

private:

	// All instances in the world
	std::vector<RenderWorldInstance*> _all_instances;

	// Visible instances in the main opaque pass
	std::array<std::vector<RenderWorldInstance*>, VisibilityFrustum_Count> _visible_instances;

};