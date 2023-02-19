#pragma once

#include <stdint.h>

enum GraphicsResourceType : uint8_t 
{
    GRT_Texture,
    GRT_Buffer,
    GRT_ShaderResourceView,
    GRT_DepthStencilView,
    GRT_UnorderedAccessView,
    GRT_RenderTargetView,
    GRT_InputLayout,
    GRT_SamplerState,
    GRT_RasterizerState,
    GRT_BlendState,
    GRT_DepthStencilState,
    GRT_Max 
};
static_assert(GraphicsResourceType::GRT_Max < 16);

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

    operator bool() const
    {
        return IsValid();
    }

    bool operator==(GraphicsResourceHandle const& rhs) const { return this->hash == rhs.hash; }

    GraphicsResourceType GetType() const { return data.type; }

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

struct SwapchainHandle : public GraphicsResourceHandle
{
    SwapchainHandle(uint64_t hash)
      : hash(hash)
    {
    }
    SwapchainHandle(uint16_t gen, uint64_t id)
    {
        data.gen = gen;
        data.id = id;
    }

    SwapchainHandle()
      : hash(static_cast<uint64_t>(-1))
    {
    }

    static SwapchainHandle const& Invalid()
    {
        static SwapchainHandle s_Invalid = SwapchainHandle();
        return s_Invalid;
    }

    bool IsValid() const { return *this != Invalid(); }

    operator bool() const
    {
        return IsValid();
    }

    bool operator==(SwapchainHandle const& rhs) const { return this->hash == rhs.hash; }

    union
    {
        uint64_t hash;
        struct
        {
            uint32_t gen : 32;
            uint32_t id : 32;
        } data;
    };
};



