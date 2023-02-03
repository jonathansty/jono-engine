#pragma once

#include <stdint.h>

enum GraphicsResourceType
{
    GRT_Texture,
    GRT_Buffer,
    GRT_ShaderResourceView,
    GRT_DepthStencilView,
    GRT_UnorderedAccessView,
    GRT_InputLayout
};

struct GraphicsResourceHandle
{
    GraphicsResourceHandle(uint64_t hash)
      : hash(hash)
    {
    }
    GraphicsResourceHandle(GraphicsResourceType grt, uint16_t gen, uint64_t id)
    {
        data.gen = gen;
        data.id = id;
        data.type = grt;
    }

    GraphicsResourceHandle()
      : hash(static_cast<uint64_t>(-1))
    {
    }

    static GraphicsResourceHandle const& Invalid()
    {
        static GraphicsResourceHandle s_Invalid = GraphicsResourceHandle();
        return s_Invalid;
    }

    bool IsValid() const { return *this != Invalid(); }

    bool operator==(GraphicsResourceHandle const& rhs) const { return this->hash == rhs.hash; }

    union
    {
        uint64_t hash;
        struct
        {
            GraphicsResourceType type : 4;
            uint16_t gen : 16;
            uint32_t id : 32;
        } data;
    };
};