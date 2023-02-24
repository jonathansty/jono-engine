#pragma once

enum class VertexLayoutFlags 
{
    Position  = 1 << 0,
    Normal    = 1 << 1,

    // Optional tangents
    Tangent0  = 1 << 2,
    Tangent1  = 1 << 3,
    Tangent2  = 1 << 4,
    Tangent3  = 1 << 5,

    // Optional UVs
    UV0  = 1 << 6,
    UV1  = 1 << 7,
    UV2  = 1 << 8,
    UV3  = 1 << 9,

    // Optional colours
    Colour0  = 1 << 10,
    Colour1  = 1 << 11,
    Colour2  = 1 << 12,
    Colour3  = 1 << 13
};
ENUM_BITFLAGS(VertexLayoutFlags);

inline bool VertexLayoutCompatable(VertexLayoutFlags meshLayout, VertexLayoutFlags shaderLayout)
{
    // #TODO: Check correctness
    return (meshLayout & shaderLayout) == shaderLayout;
}