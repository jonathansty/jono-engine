#pragma once

namespace Graphics
{

enum class ShaderType : u32
{
	Vertex,
	Pixel,
	Compute,
	Domain,
	Geometry,
	Hull,
};

inline const char* get_target(ShaderType stage)
{
	switch (stage)
	{
		case ShaderType::Vertex:
			return "vs_5_0";
		case ShaderType::Pixel:
			return "ps_5_0";
		case ShaderType::Compute:
			return "cs_5_0";
		case ShaderType::Domain:
			return "ds_5_0";
		case ShaderType::Geometry:
			return "gs_5_0";
		case ShaderType::Hull:
			return "hs_5_0";
	}
	return nullptr;
}

}