#pragma once
#include "singleton.h"

// #TODO: Make common render context interface to avoid having to do this
struct Dx11RenderContext;
using RenderContext = Dx11RenderContext;

struct ENGINE_API IOverlay
{
	virtual ~IOverlay() = default;

	// Render function called when building ImGui UI
	virtual void RenderOverlay() = 0;

	// Render function called when rendering the ImGui viewport
	virtual void RenderViewport() {}

	virtual void Render3D(RenderContext& ctx) {}
};

class ENGINE_API DebugOverlay : public IOverlay
{
public:
	DebugOverlay(bool isOpen, std::string name);
	virtual ~DebugOverlay() = default;

	const char* get_name() const;

	bool get_visible() const { return _isOpen; }
	void set_visible(bool visible);

protected:
	bool _isOpen;
	std::string _name;

private:
	friend class OverlayManager;
};

class OverlayManager : public DebugOverlay
{
public:
	OverlayManager(bool isOpen = false)
			: DebugOverlay(isOpen, "Overlays") {}
	~OverlayManager();

	void RenderOverlay() override;
	void RenderViewport() override;
	void Render3D(RenderContext& ctx) override;

	void register_overlay(DebugOverlay* overlay);
	void unregister_overlay(DebugOverlay* overlay);

	DebugOverlay* get_overlay(std::string const& name);

	std::vector<DebugOverlay*> get_overlays() const;

private:
	std::unordered_map<std::string, DebugOverlay*> _overlays;
};
