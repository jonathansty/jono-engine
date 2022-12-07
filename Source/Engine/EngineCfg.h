#pragma once

enum class MSAAMode
{
	Off,
	MSAA_2x,
	MSAA_4x,
};

struct GraphicsSettings
{
	CLASS_BASE(GraphicsSettings)

	u32 m_Foo;
};

struct EngineCfg
{
	CLASS_BASE(EngineCfg);

	// Allow 2D rendering
	bool m_UseD2D = false;
	bool m_UseD2DAA = false;

	// Allow 3D rendering
	bool m_UseD3D = true;
	MSAAMode m_D3DMSAA = MSAAMode::Off;

	f64 m_MaxFrametime;

	GraphicsSettings m_GraphicsSettings;

	std::vector<u32> m_Values;

};

