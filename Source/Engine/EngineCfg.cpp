#include "engine.pch.h"
#include "EngineCfg.h"

#include "CommandLine.h"
#include "Types/IniStream.h"


REGISTER_TYPE("/Types/Core/EngineCfg", EngineCfg);

void EngineCfg::Serialize(IniStream& data, EngineCfg* cfg)
{
	EngineCfg* output = (EngineCfg*)(cfg);

	output->d2d_use = data.GetPropertyValue<bool>("UseD2D");
	output->d2d_use_aa = data.GetPropertyValue<bool>("UseD2DAA");
	output->d3d_use = data.GetPropertyValue<bool>("UseD3D");
	output->d3d_msaa_mode = (MSAAMode)data.GetPropertyValue<int>("MSAAMode");
	output->max_frame_time = data.GetPropertyValue<float>("MaxFrameTime");
}
