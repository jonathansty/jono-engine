#pragma once

enum class MSAAMode
{
	Off,
	MSAA_2x,
	MSAA_4x,
};

struct EngineSettings
{
	// Allow 2D rendering
	bool d2d_use = false;
	bool d2d_use_aa = false;

	// Allow 3D rendering
	bool d3d_use = true;
	MSAAMode d3d_msaa_mode = MSAAMode::Off;

	f64 max_frame_time = 0.0;
};
