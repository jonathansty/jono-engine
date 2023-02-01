#pragma once

#include "GraphicsResourceHandle.h"
#include "Core/Containers.h"

using BufferDesc = D3D11_BUFFER_DESC;
using SrvDesc = D3D11_SHADER_RESOURCE_VIEW_DESC;
using UavDesc = D3D11_UNORDERED_ACCESS_VIEW_DESC;
using InputLayoutDesc = std::vector<D3D11_INPUT_ELEMENT_DESC>;

struct Viewport
{
    float x;
    float y;
    float width;
    float height;
    float minZ;
    float maxZ;
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
    
    // Context clearing
    inline void SetViewport(Viewport const& vp);
    void ClearTargets(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, float4 color, uint32_t clearFlags, float depth, uint8_t stencil);

    void* Map(GraphicsResourceHandle buffer);
    void Unmap(GraphicsResourceHandle buffer);

    void BeginFrame();
    void EndFrame();

    void Flush();

    class Dx11RenderInterface* owner;
    ID3D11DeviceContext* m_Context;

        // State tracking
    struct
    {
        ID3D11PixelShader* PS;
        ID3D11VertexShader* VS;
        ID3D11InputLayout* InputLayout;
        ID3D11RasterizerState* RS;
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

    GraphicsResourceHandle CreateBuffer(BufferDesc const& desc, void* initialData);
    GraphicsResourceHandle CreateShaderResourceView(GraphicsResourceHandle srcBuffer, SrvDesc const& desc);
    GraphicsResourceHandle CreateUnorderedAccessView(GraphicsResourceHandle srcBuffer, UavDesc const& desc);
    GraphicsResourceHandle CreateInputLayout(InputLayoutDesc inputLayoutElements, void* shaderCode, uint32_t shaderCodeLength);



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

private:

    ID3D11Buffer* GetRawBuffer(GraphicsResourceHandle h) 
    { 
        ASSERT(h.data.type == GRT_Buffer);
        return m_Buffers.Get({ h.data.id, h.data.gen }).Get();
    }

    ID3D11ShaderResourceView* GetRawSRV(GraphicsResourceHandle h)
    {
        ASSERT(h.data.type == GRT_ShaderResourceView);
        return m_SRVs.Get({ h.data.id, h.data.gen }).Get();
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

private:

    ComPtr<ID3D11Device> m_Device;
    ComPtr<ID3D11DeviceContext> m_Context;
    ComPtr<IDXGIFactory> m_Factory;

    ComPtr<IWICImagingFactory> m_WicFactory;
    ComPtr<ID3DUserDefinedAnnotation> m_UserDefinedAnnotations;

    SlotVector<ComPtr<ID3D11Buffer>> m_Buffers;
    SlotVector<ComPtr<ID3D11ShaderResourceView>> m_SRVs;
    SlotVector<ComPtr<ID3D11UnorderedAccessView>> m_UAVs;
    SlotVector<ComPtr<ID3D11InputLayout>> m_InputLayouts;

    Dx11RenderContext m_ActiveRenderContext;

};

#include "Dx11RenderInterface.inl"
