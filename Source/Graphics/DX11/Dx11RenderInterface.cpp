#include "Graphics.pch.h"
#include "RenderInterface.h"


RenderInterface* GetRI()
{
    static Dx11RenderInterface* ri = nullptr;
    if(ri == nullptr)
    {
        ri = new Dx11RenderInterface();
    }
    return ri;
}

namespace Helpers
{
inline void SetDebugObjectName(ID3D11DeviceChild* obj, std::string_view const& name)
{
    ENSURE_HR(obj->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(name.size()), name.data()));
}
inline void SetDebugObjectName(IDXGIObject* obj, std::string_view const& name)
{
    ENSURE_HR(obj->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(name.size()), name.data()));
}
} // namespace Helpers

void Dx11RenderInterface::Init()
{
    MEMORY_TAG(MemoryCategory::Graphics);

    // Reserve some initial sizes
    m_Resources.Grow(1024);
    m_SRVs.Grow(2048);
    m_UAVs.Grow(512);
    m_DSVs.Grow(512);
    m_InputLayouts.Grow(128);
    m_SamplerStates.Grow(64);
    m_RasterizerStates.Grow(64);
    m_BlendStates.Grow(64);
    m_DepthStencilStates.Grow(64);

    // Create Direct3D 11 factory
    {
        ENSURE_HR(CreateDXGIFactory(IID_PPV_ARGS(&m_Factory)));
        Helpers::SetDebugObjectName(m_Factory.Get(), "DXGI Factory");

        // Define the ordering of feature levels that Direct3D attempts to create.
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
        };
        D3D_FEATURE_LEVEL featureLevel;

        uint32_t creation_flag = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

        // #todo
        bool debug_layer = cli::has_arg("-enable-d3d-debug");
        if (debug_layer)
        {
            creation_flag |= D3D11_CREATE_DEVICE_DEBUG;
            debug_layer = true;
        }

        ENSURE_HR(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creation_flag, featureLevels, UINT(std::size(featureLevels)), D3D11_SDK_VERSION, &m_Device, &featureLevel, &m_Context));

        if (debug_layer)
        {
            bool do_breaks = cli::has_arg("-d3d-break");
            ComPtr<ID3D11InfoQueue> info_queue;
            m_Device->QueryInterface(IID_PPV_ARGS(&info_queue));
            if (info_queue)
            {
                if (do_breaks)
                {
                    info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
                    info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
                }

                // D3D11_INFO_QUEUE_FILTER f{};
                // f.DenyList.NumSeverities = 1;
                // D3D11_MESSAGE_SEVERITY severities[1] = {
                //	D3D11_MESSAGE_SEVERITY_WARNING
                // };
                // f.DenyList.pSeverityList = severities;
                // info_queue->AddStorageFilterEntries(&f);
            }
        }
    }


    HRESULT hr;
    if (!m_WicFactory)
    {
        hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_WicFactory));
        if (FAILED(hr))
        {
            FAILMSG("Create WIC Factory Failed");
        }
    }

    ENSURE_HR(m_Context->QueryInterface(IID_PPV_ARGS(&m_UserDefinedAnnotations)));
}

void Dx11RenderInterface::Shutdown()
{
}

GraphicsResourceHandle Dx11RenderInterface::CreateBuffer(BufferDesc const& desc, SubresourceData const* initialData, std::string_view debugName)
{
    ComPtr<ID3D11Buffer> buffer;
    if(initialData && initialData->pSysMem)
    {
		D3D11_SUBRESOURCE_DATA subresourceData = *initialData;
		ENSURE_HR(m_Device->CreateBuffer(&desc, &subresourceData, buffer.GetAddressOf()));
    }
    else
    {
		ENSURE_HR(m_Device->CreateBuffer(&desc, nullptr, buffer.GetAddressOf()));
    }

    Helpers::SetDebugObjectName(buffer.Get(), debugName);

    auto slotH = m_Resources.Push(buffer);
    GraphicsResourceHandle handle = GraphicsResourceHandle(GRT_Buffer, slotH.gen, slotH.id);
    return handle;
}

GraphicsResourceHandle Dx11RenderInterface::CreateShaderResourceView(GraphicsResourceHandle srcBuffer, SrvDesc const& desc, std::string_view debugName)
{
    ComPtr<ID3D11ShaderResourceView> srv;

    ID3D11Resource* b = GetRawResource(srcBuffer);
    ENSURE_HR(m_Device->CreateShaderResourceView(b, &desc, srv.GetAddressOf()));

    Helpers::SetDebugObjectName(srv.Get(), debugName);

    auto h = m_SRVs.Push(srv);

    GraphicsResourceHandle handle = GraphicsResourceHandle(GRT_ShaderResourceView, h.gen, h.id);
    return handle;

}

GraphicsResourceHandle Dx11RenderInterface::CreateUnorderedAccessView(GraphicsResourceHandle srcBuffer, UavDesc const& desc, std::string_view debugName)
{
    ComPtr<ID3D11UnorderedAccessView> uav;
    ID3D11Resource* b = GetRawResource(srcBuffer);
    ENSURE_HR(m_Device->CreateUnorderedAccessView(b, &desc, &uav));

    Helpers::SetDebugObjectName(uav.Get(), debugName);

    auto h = m_UAVs.Push(uav);
    GraphicsResourceHandle handle = GraphicsResourceHandle(GRT_UnorderedAccessView, h.gen, h.id);
    return handle;
}

GraphicsResourceHandle Dx11RenderInterface::CreateDepthStencilView(GraphicsResourceHandle srcBuffer, DsvDesc const& desc, std::string_view debugName /*= ""*/)
{
    ComPtr<ID3D11DepthStencilView> ds;
    ENSURE_HR(m_Device->CreateDepthStencilView(GetRawResource(srcBuffer), &desc, ds.ReleaseAndGetAddressOf()));

    Helpers::SetDebugObjectName(ds.Get(), debugName);

    auto h = m_DSVs.Push(ds);
    GraphicsResourceHandle handle = GraphicsResourceHandle(GRT_UnorderedAccessView, h.gen, h.id);
    return handle;
}

GraphicsResourceHandle Dx11RenderInterface::CreateInputLayout(InputLayoutDesc inputLayoutElements, void* shaderCode, uint32_t shaderCodeLength)
{
    ComPtr<ID3D11InputLayout> inputLayout;
    ENSURE_HR(m_Device->CreateInputLayout(inputLayoutElements.data(), (UINT)inputLayoutElements.size(), shaderCode, (UINT)shaderCodeLength, &inputLayout));
    auto h = m_InputLayouts.Push(inputLayout);
    GraphicsResourceHandle handle = GraphicsResourceHandle(GRT_InputLayout, h.gen, h.id);
    return handle;
}

GraphicsResourceHandle Dx11RenderInterface::CreateSamplerState(SamplerStateDesc samplerStateDesc, std::string_view name /*= ""*/)
{
    ComPtr<ID3D11SamplerState> ss;
    ENSURE_HR(m_Device->CreateSamplerState(&samplerStateDesc, ss.ReleaseAndGetAddressOf()));
    Helpers::SetDebugObjectName(ss.Get(), name);

    auto h = m_SamplerStates.Push(ss);
    GraphicsResourceHandle handle = GraphicsResourceHandle(GRT_SamplerState, h.gen, h.id);
    return handle;
}

GraphicsResourceHandle Dx11RenderInterface::CreateRasterizerState(RasterizerStateDesc desc, std::string_view name /*= ""*/)
{
    ComPtr<ID3D11RasterizerState> ss;
    ENSURE_HR(m_Device->CreateRasterizerState(&desc, ss.ReleaseAndGetAddressOf()));
    Helpers::SetDebugObjectName(ss.Get(), name);

    auto h = m_RasterizerStates.Push(ss);
    GraphicsResourceHandle handle = GraphicsResourceHandle(GRT_RasterizerState, h.gen, h.id);
    return handle;
}

GraphicsResourceHandle Dx11RenderInterface::CreateBlendState(BlendStateDesc desc, std::string_view name /*= ""*/)
{
    ComPtr<ID3D11BlendState> ss;
    ENSURE_HR(m_Device->CreateBlendState(&desc, ss.ReleaseAndGetAddressOf()));
    Helpers::SetDebugObjectName(ss.Get(), name);

    auto h = m_BlendStates.Push(ss);
    GraphicsResourceHandle handle = GraphicsResourceHandle(GRT_BlendState, h.gen, h.id);
    return handle;
}

GraphicsResourceHandle Dx11RenderInterface::CreateDepthStencilState(DepthStencilDesc desc, std::string_view name /*= ""*/)
{
    ComPtr<ID3D11DepthStencilState> ss;
    ENSURE_HR(m_Device->CreateDepthStencilState(&desc, ss.ReleaseAndGetAddressOf()));
    Helpers::SetDebugObjectName(ss.Get(), name);

    auto h = m_DepthStencilStates.Push(ss);
    GraphicsResourceHandle handle = GraphicsResourceHandle(GRT_DepthStencilState, h.gen, h.id);
    return handle;
}

GraphicsResourceHandle Dx11RenderInterface::CreateTexture(Texture1DDesc const& desc, void* initialData, std::string_view name)
{
    ComPtr<ID3D11Texture1D> res;
    if(initialData != nullptr)
    {
        D3D11_SUBRESOURCE_DATA data{};
        data.pSysMem = initialData;
        ENSURE_HR(m_Device->CreateTexture1D(&desc, &data, &res));
    }
    else
    {
        ENSURE_HR(m_Device->CreateTexture1D(&desc, nullptr, &res));
    }

    ComPtr<ID3D11Resource> genRes;
    res.As<ID3D11Resource>(&genRes);
    Helpers::SetDebugObjectName(genRes.Get(), name);
    auto handle = m_Resources.Push(genRes);
    return GraphicsResourceHandle(GRT_Texture, handle.gen, handle.id);
}

GraphicsResourceHandle Dx11RenderInterface::CreateTexture(Texture2DDesc const& desc, void* initialData, std::string_view name)
{
    ComPtr<ID3D11Texture2D> res;
    if (initialData != nullptr)
    {
        D3D11_SUBRESOURCE_DATA data{};
        data.pSysMem = initialData;
        data.SysMemPitch = sizeof(u32) * desc.Width;
        ENSURE_HR(m_Device->CreateTexture2D(&desc, &data, &res));
    }
    else
    {
        ENSURE_HR(m_Device->CreateTexture2D(&desc, nullptr, &res));
    }

    ComPtr<ID3D11Resource> genRes;
    res.As<ID3D11Resource>(&genRes);

    Helpers::SetDebugObjectName(genRes.Get(), name);
    auto handle = m_Resources.Push(genRes);
    return GraphicsResourceHandle(GRT_Texture, handle.gen, handle.id);
}

GraphicsResourceHandle Dx11RenderInterface::CreateTexture(Texture3DDesc const& desc, void* initialData, std::string_view name)
{
    ComPtr<ID3D11Texture3D> res;
    if (initialData != nullptr)
    {
        D3D11_SUBRESOURCE_DATA data{};
        data.pSysMem = initialData;
        ENSURE_HR(m_Device->CreateTexture3D(&desc, &data, &res));
    }
    else
    {
        ENSURE_HR(m_Device->CreateTexture3D(&desc, nullptr, &res));
    }

    ComPtr<ID3D11Resource> genRes;
    res.As<ID3D11Resource>(&genRes);
    Helpers::SetDebugObjectName(genRes.Get(), name);
    auto handle = m_Resources.Push(genRes);
    return GraphicsResourceHandle(GRT_Texture, handle.gen, handle.id);
}

void Dx11RenderInterface::ReleaseResource(GraphicsResourceHandle& h)
{
    switch (h.data.type)
    {
        case GRT_ShaderResourceView:
            m_SRVs.Erase(SlotHandle{ h.data.id, h.data.gen });
            break;
        case GRT_UnorderedAccessView:
            m_UAVs.Erase(SlotHandle{ h.data.id, h.data.gen });
            break;
        case GRT_InputLayout:
            m_InputLayouts.Erase(SlotHandle{ h.data.id, h.data.gen });
            break;
        case GRT_Texture:
        case GRT_Buffer:
            m_Resources.Erase(SlotHandle{ h.data.id, h.data.gen });
            break;
        default:
            ASSERT("Unsupported type!");
            break;
    }
    h = GraphicsResourceHandle::Invalid();
}

void* Dx11RenderContext::Map(GraphicsResourceHandle buffer)
{
    D3D11_MAPPED_SUBRESOURCE resource{};

    ID3D11Buffer* b = owner->GetRawBuffer(buffer);
    ENSURE_HR(m_Context->Map(b, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));
    return resource.pData;
}

void Dx11RenderContext::Unmap(GraphicsResourceHandle buffer)
{
    ID3D11Buffer* b = owner->GetRawBuffer(buffer);
    m_Context->Unmap(b, 0);
}


void Dx11RenderContext::ClearTargets(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, float4 color, uint32_t clearFlags, float depth, uint8_t stencil)
{
    float col[4] = { color.x, color.y, color.z, color.w };
    m_Context->ClearRenderTargetView(rtv, col);
    m_Context->ClearDepthStencilView(dsv, clearFlags, depth, stencil);
}

void Dx11RenderContext::SetShaderResources(ShaderStage stage, uint32_t startSlot, Span<GraphicsResourceHandle> srvs)
{
    ASSERT(srvs.size() < 128);
    ID3D11ShaderResourceView* views[128];
    UINT numViews = std::min((UINT)srvs.size(), (UINT)128);
    for(size_t j = 0; j < numViews; ++j)
    {
        views[j] = owner->GetRawSRV(srvs[j]);
    }

    switch(stage)
    {
        case ShaderStage::Vertex:
            m_Context->VSSetShaderResources(startSlot, numViews, views);
            break;
        case ShaderStage::Pixel:
            m_Context->PSSetShaderResources(startSlot, numViews, views);
            break;
        case ShaderStage::Compute:
            m_Context->CSSetShaderResources(startSlot, numViews, views);
            break;
        case ShaderStage::Domain:
            m_Context->DSSetShaderResources(startSlot, numViews, views);
            break;
        case ShaderStage::Geometry:
            m_Context->GSSetShaderResources(startSlot, numViews, views);
            break;
        case ShaderStage::Hull:
            m_Context->HSSetShaderResources(startSlot, numViews, views);
            break;
        default:
            break;
    }
}

void Dx11RenderContext::SetSamplers(ShaderStage stage, uint32_t startSlot, Span<GraphicsResourceHandle> samplers)
{
    ASSERT(samplers.size() < 16);
    ID3D11SamplerState* rawSamplers[16];
    UINT numViews = std::min((UINT)samplers.size(), (UINT)16);
    for(size_t j = 0; j < numViews; ++j)
    {
        rawSamplers[j] = owner->GetRawSampler(samplers[j]);
    }

    switch(stage)
    {
        case ShaderStage::Vertex:
            m_Context->VSSetSamplers(startSlot, numViews, rawSamplers);
            break;
        case ShaderStage::Pixel:
            m_Context->PSSetSamplers(startSlot, numViews, rawSamplers);
            break;
        case ShaderStage::Compute:
            m_Context->CSSetSamplers(startSlot, numViews, rawSamplers);
            break;
        case ShaderStage::Domain:
            m_Context->DSSetSamplers(startSlot, numViews, rawSamplers);
            break;
        case ShaderStage::Geometry:
            m_Context->GSSetSamplers(startSlot, numViews, rawSamplers);
            break;
        case ShaderStage::Hull:
            m_Context->HSSetSamplers(startSlot, numViews, rawSamplers);
            break;
        default:
            break;
    
    }
}

void Dx11RenderContext::BeginFrame()
{
    m_Context->ClearState();

    m_PrevRenderState.VS = nullptr;
    m_PrevRenderState.PS = nullptr;
    m_PrevRenderState.InputLayout = GraphicsResourceHandle::Invalid();
    m_PrevRenderState.RS = GraphicsResourceHandle::Invalid();
}

void Dx11RenderContext::EndFrame()
{
}

void Dx11RenderContext::Flush()
{
    m_Context->Flush();
}

void Dx11RenderContext::ExecuteComputeItems(Span<ComputeItem> const& items)
{
    for(ComputeItem const& item : items)
    {
        std::array<ID3D11ShaderResourceView*, 128> srvs = {};
        std::array<ID3D11UnorderedAccessView*, 128> uavs = {};
        std::array<ID3D11Buffer*, 128> cbs = {};

        for(size_t i = 0; i < item.srvs.size(); ++i)
        {
            srvs[i] = owner->GetRawSRV(item.srvs[i]);
        }

        for(size_t i = 0; i < item.uavs.size(); ++i)
        {
            uavs[i] = owner->GetRawUAV(item.uavs[i]);
        }

        for(size_t i = 0; i < item.cbs.size(); ++i)
        {
            cbs[i] = owner->GetRawBuffer(item.cbs[i]);
        }

        m_Context->CSSetUnorderedAccessViews(0, (UINT)item.uavs.size(), uavs.data(), nullptr);
        m_Context->CSSetConstantBuffers(0, (UINT)item.cbs.size(), cbs.data());
        m_Context->CSSetShaderResources(0, (UINT)item.srvs.size(), srvs.data());
        m_Context->CSSetShader(item.shader, nullptr, 0);
        m_Context->Dispatch(item.dispatchX, item.dispatchY, item.dispatchZ);
    }

    m_Context->CSSetUnorderedAccessViews(0, 0, nullptr, nullptr);
    m_Context->CSSetShaderResources(0, 0, nullptr);
    m_Context->CSSetConstantBuffers(0, 0, nullptr);
}
