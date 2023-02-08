#pragma once

#include "Core/Array.h"
#include "Core/Containers.h"

#include "GraphicsResourceHandle.h"
#include "ShaderStage.h"

using BufferDesc          = D3D11_BUFFER_DESC;
using SrvDesc             = D3D11_SHADER_RESOURCE_VIEW_DESC;
using UavDesc             = D3D11_UNORDERED_ACCESS_VIEW_DESC;
using DsvDesc             = D3D11_DEPTH_STENCIL_VIEW_DESC;
using InputLayoutDesc     = std::vector<D3D11_INPUT_ELEMENT_DESC>;
using SamplerStateDesc    = D3D11_SAMPLER_DESC;
using RasterizerStateDesc = D3D11_RASTERIZER_DESC;
using BlendStateDesc      = D3D11_BLEND_DESC;
using DepthStencilDesc    = D3D11_DEPTH_STENCIL_DESC;
using Texture1DDesc       = D3D11_TEXTURE1D_DESC;
using Texture2DDesc       = D3D11_TEXTURE2D_DESC;
using Texture3DDesc       = D3D11_TEXTURE3D_DESC;


struct Viewport
{
    Viewport() = default;

    Viewport(float x, float y, float width, float height)
        : x(x), y(y), width(width), height(height), minZ(0.0f), maxZ(1.0f)
    {
    
    }
    float x;
    float y;
    float width;
    float height;
    float minZ;
    float maxZ;
};

struct ComputeItem
{
    Array<GraphicsResourceHandle> srvs;
    Array<GraphicsResourceHandle> uavs;
    Array<GraphicsResourceHandle> cbs;

    ID3D11ComputeShader* shader = nullptr;

    uint32_t dispatchX = 1;
    uint32_t dispatchY = 1;
    uint32_t dispatchZ = 1;
};


namespace Graphics
{
class Renderer;
};

struct Dx11RenderContext 
{
    inline void IASetIndexBuffer(GraphicsResourceHandle const& buffer, DXGI_FORMAT format, uint32_t offset);
    inline void IASetInputLayout(GraphicsResourceHandle const& inputLayout);
    inline void IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);

    inline void RSSetState(GraphicsResourceHandle rs);
    inline void RSSetViewports(Span<Viewport> vps);
    inline void SetViewport(Viewport const& vp);

    inline void PSSetShader(ID3D11PixelShader* ps);
    inline void VSSetShader(ID3D11VertexShader* vs);

    inline void OMSetDepthStencilState(GraphicsResourceHandle dss, uint32_t stencilRef = 0);
    inline void OMSetBlendState(GraphicsResourceHandle bs, std::array<float, 4> blendFactor = {}, uint32_t sampleMask = 0x0);

    void ClearTargets(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, float4 color, uint32_t clearFlags, float depth, uint8_t stencil);

    void SetShaderResources(ShaderStage stage, uint32_t startSlot, Span<GraphicsResourceHandle> srvs);
    void SetSamplers(ShaderStage stage, uint32_t startSlot, Span<GraphicsResourceHandle> samplers);

    void* Map(GraphicsResourceHandle buffer);
    void  Unmap(GraphicsResourceHandle buffer);

    void BeginFrame();
    void EndFrame();
    void Flush();

    // Compute Shader interface
    inline void ExecuteComputeItem(ComputeItem const& item);
    void ExecuteComputeItems(Span<ComputeItem> const& items);


    // Render interface owner
    class Dx11RenderInterface* owner;

    // D3D11 implementation details
    ID3D11DeviceContext* m_Context;

    // State tracking
    struct
    {
        ID3D11PixelShader* PS;
        ID3D11VertexShader* VS;
        GraphicsResourceHandle InputLayout;
        GraphicsResourceHandle RS;
    } m_PrevRenderState;
};

class Dx11RenderInterface
{
    // #TODO: Remove all friend classes
    friend class Graphics::Renderer;
    friend class Material;
    friend class MaterialInstance;
    friend class GraphicsThread;
    friend struct Dx11RenderContext;

public:
    void Init();
    void Shutdown();

    GraphicsResourceHandle CreateBuffer(BufferDesc const& desc, void* initialData, std::string_view debugName = "");
    GraphicsResourceHandle CreateShaderResourceView(GraphicsResourceHandle srcBuffer, SrvDesc const& desc, std::string_view debugName = "");
    GraphicsResourceHandle CreateUnorderedAccessView(GraphicsResourceHandle srcBuffer, UavDesc const& desc, std::string_view debugName = "");
    GraphicsResourceHandle CreateDepthStencilView(GraphicsResourceHandle srcBuffer, DsvDesc const& desc, std::string_view debugName = "");
    GraphicsResourceHandle CreateInputLayout(InputLayoutDesc inputLayoutElements, void* shaderCode, uint32_t shaderCodeLength);
    GraphicsResourceHandle CreateSamplerState(SamplerStateDesc samplerStateDesc, std::string_view name = "");
    GraphicsResourceHandle CreateRasterizerState(RasterizerStateDesc desc, std::string_view name = "");
    GraphicsResourceHandle CreateBlendState(BlendStateDesc desc, std::string_view name = "");
    GraphicsResourceHandle CreateDepthStencilState(DepthStencilDesc desc, std::string_view name = "");

    GraphicsResourceHandle CreateTexture(Texture1DDesc const& desc, void* initialData, std::string_view name = "");
    GraphicsResourceHandle CreateTexture(Texture2DDesc const& desc, void* initialData, std::string_view name = "");
    GraphicsResourceHandle CreateTexture(Texture3DDesc const& desc, void* initialData, std::string_view name = "");

    void ReleaseResource(GraphicsResourceHandle h);


    Dx11RenderContext& BeginContext() 
    {
        m_ActiveRenderContext.owner = this;
        m_ActiveRenderContext.m_Context = m_Context.Get();
        return m_ActiveRenderContext;
    }

    void FlushContext(Dx11RenderContext& ctx)
    {
        //m_Context->Flush();
        ctx.Flush();
    }

    template<typename T> T* GetRawResourceAs(GraphicsResourceHandle h)
    {
        SlotHandle slotHandle = {
            h.data.id, h.data.gen
        };
        switch (h.data.type)
        {
            case GRT_Texture:
            case GRT_Buffer:
                return reinterpret_cast<T*>(m_Resources.Get(slotHandle).Get());
            case GRT_ShaderResourceView:
                return reinterpret_cast<T*>(m_SRVs.Get(slotHandle).Get());
            case GRT_DepthStencilView:
                return reinterpret_cast<T*> (m_DSVs.Get(slotHandle).Get());
            case GRT_UnorderedAccessView:
                return reinterpret_cast<T*>(m_UAVs.Get(slotHandle).Get());
            case GRT_InputLayout:
                return reinterpret_cast<T*>(m_InputLayouts.Get(slotHandle).Get());
            case GRT_SamplerState:
                return reinterpret_cast<T*>(m_SamplerStates.Get(slotHandle).Get());
            case GRT_RasterizerState:
                return reinterpret_cast<T*>(m_RasterizerStates.Get(slotHandle).Get());
            case GRT_BlendState:
                return reinterpret_cast<T*>(m_BlendStates.Get(slotHandle).Get());
            case GRT_DepthStencilState:
                return reinterpret_cast<T*>(m_DepthStencilStates.Get(slotHandle).Get());
            default:
                ASSERT("Unsupported type.");
                break;
        }
        return nullptr;
    }

private:

    ID3D11Resource* GetRawResource(GraphicsResourceHandle h)
    {
        switch(h.data.type)
        {
            case GRT_Texture:
                return m_Resources.Get({ h.data.id, h.data.gen }).Get();
            case GRT_Buffer:
                return m_Resources.Get({ h.data.id, h.data.gen }).Get();
            case GRT_ShaderResourceView:
            case GRT_DepthStencilView:
            case GRT_UnorderedAccessView:
            case GRT_InputLayout:
            default:
                ASSERT("Unsupported type.");
                break;
        }
        return nullptr;
    }
    ID3D11Buffer* GetRawBuffer(GraphicsResourceHandle h) 
    { 
        ASSERT(h.data.type == GRT_Buffer);
        return static_cast<ID3D11Buffer*>(m_Resources.Get({ h.data.id, h.data.gen }).Get());
    }

    ID3D11ShaderResourceView* GetRawSRV(GraphicsResourceHandle h)
    {
        ASSERT(h.data.type == GRT_ShaderResourceView);
        return m_SRVs.Get({ h.data.id, h.data.gen }).Get();
    }

    ID3D11SamplerState* GetRawSampler(GraphicsResourceHandle h)
    {
        ASSERT(h.data.type == GRT_SamplerState);
        return m_SamplerStates.Get({ h.data.id, h.data.gen }).Get();
    }

    ID3D11UnorderedAccessView* GetRawUAV(GraphicsResourceHandle h)
    {
        ASSERT(h.data.type == GRT_UnorderedAccessView);
        return m_UAVs.Get({ h.data.id, h.data.gen }).Get();
    }

    ID3D11InputLayout* GetRawInputLayout(GraphicsResourceHandle h)
    {
        ASSERT(h.data.type == GRT_InputLayout);
        return m_InputLayouts.Get({ h.data.id, h.data.gen }).Get();
    }

    ID3D11Texture2D* GetRawTexture2D(GraphicsResourceHandle h)
    {
        ASSERT(h.data.type == GRT_Texture);
        return static_cast<ID3D11Texture2D*>(m_Resources.Get({ h.data.id, h.data.gen }).Get());
    }

private:

    ComPtr<ID3D11Device> m_Device;
    ComPtr<ID3D11DeviceContext> m_Context;
    ComPtr<IDXGIFactory> m_Factory;

    ComPtr<IWICImagingFactory> m_WicFactory;
    ComPtr<ID3DUserDefinedAnnotation> m_UserDefinedAnnotations;

    SlotVector<ComPtr<ID3D11Resource>> m_Resources;
    SlotVector<ComPtr<ID3D11ShaderResourceView>> m_SRVs;
    SlotVector<ComPtr<ID3D11UnorderedAccessView>> m_UAVs;
    SlotVector<ComPtr<ID3D11DepthStencilView>> m_DSVs;
    SlotVector<ComPtr<ID3D11InputLayout>> m_InputLayouts;
    SlotVector<ComPtr<ID3D11SamplerState>> m_SamplerStates;
    SlotVector<ComPtr<ID3D11RasterizerState>> m_RasterizerStates;
    SlotVector<ComPtr<ID3D11BlendState>> m_BlendStates;
    SlotVector<ComPtr<ID3D11DepthStencilState>> m_DepthStencilStates;

    Dx11RenderContext m_ActiveRenderContext;

};

#include "Dx11RenderInterface.inl"
