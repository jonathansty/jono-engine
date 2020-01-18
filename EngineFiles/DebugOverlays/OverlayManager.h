#pragma once
#include "singleton.h"

interface IOverlay
{
	virtual void render_overlay() = 0;
};

class DebugOverlay : public IOverlay
{
public:
	DebugOverlay(bool isOpen, std::string name) 
		: _isOpen(isOpen)
		, _name(name)
	{
	}

	const char* get_name() const
	{
		return _name.c_str();
	}

	bool get_visible() const { return _isOpen; }
	void set_visible(bool visible) { _isOpen = visible; }

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
	~OverlayManager() {}


	void render_overlay() override;

	void register_overlay(DebugOverlay* overlay);
	void unregister_overlay(DebugOverlay* overlay);

private:
	std::unordered_map<std::string, DebugOverlay*> _overlays;

};
