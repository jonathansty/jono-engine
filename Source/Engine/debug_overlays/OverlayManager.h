#pragma once
#include "singleton.h"

struct IOverlay
{
	virtual ~IOverlay() = default;

	// Render function called when building ImGui UI
	virtual void render_overlay() = 0;

	// Render function called when rendering the ImGui viewport
	virtual void render_viewport() {}
};

class DebugOverlay : public IOverlay
{
public:
	DebugOverlay(bool isOpen, std::string name) 
		: _isOpen(isOpen)
		, _name(name)
	{
	}
	virtual ~DebugOverlay() = default;

	const char* get_name() const
	{
		return _name.c_str();
	}

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
	OverlayManager(bool isOpen = false) : DebugOverlay(isOpen, "Overlays") {}
	~OverlayManager();


	void render_overlay() override;
	void render_viewport() override;

	void register_overlay(DebugOverlay* overlay);
	void unregister_overlay(DebugOverlay* overlay);

	DebugOverlay* get_overlay(std::string const& name);

	std::vector<DebugOverlay*> get_overlays() const;

private:
	std::unordered_map<std::string, DebugOverlay*> _overlays;

};
