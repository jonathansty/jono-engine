#pragma once
enum class ShaderStage : u32
{
    Vertex,
    Pixel,
    Compute,
    Domain,
    Geometry,
    Hull,
};

inline const char* get_target(ShaderStage stage)
{
    switch (stage)
    {
        case ShaderStage::Vertex:
            return "vs_5_0";
        case ShaderStage::Pixel:
            return "ps_5_0";
        case ShaderStage::Compute:
            return "cs_5_0";
        case ShaderStage::Domain:
            return "ds_5_0";
        case ShaderStage::Geometry:
            return "gs_5_0";
        case ShaderStage::Hull:
            return "hs_5_0";
    }
    return nullptr;
}
