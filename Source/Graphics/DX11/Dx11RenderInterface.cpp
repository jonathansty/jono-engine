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
inline void SetDebugObjectName(IDXGIObject* obj, std::string const& name)
{
    ENSURE_HR(obj->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(name.size()), name.c_str()));
}
} // namespace Helpers

void Dx11RenderInterface::Init()
{
    MEMORY_TAG(MemoryCategory::Graphics);

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

GraphicsResourceHandle Dx11RenderInterface::CreateBuffer(BufferDesc const& desc, void* initialData)
{
    ComPtr<ID3D11Buffer> buffer;
    if(initialData)
    {
		D3D11_SUBRESOURCE_DATA subresourceData{};
        subresourceData.pSysMem = initialData;
		ENSURE_HR(m_Device->CreateBuffer(&desc, &subresourceData, buffer.GetAddressOf()));
    }
    else
    {
		ENSURE_HR(m_Device->CreateBuffer(&desc, nullptr, buffer.GetAddressOf()));
    }



    // #TODO: Find first free slot 
    auto slotH = m_Buffers.Push(buffer);
    GraphicsResourceHandle handle = GraphicsResourceHandle(GRT_Buffer, slotH.gen, slotH.id);
    return handle;
}

GraphicsResourceHandle Dx11RenderInterface::CreateShaderResourceView(GraphicsResourceHandle srcBuffer, SrvDesc const& desc)
{
    ComPtr<ID3D11ShaderResourceView> srv;

    ID3D11Buffer* b = GetRawBuffer(srcBuffer);
    ENSURE_HR(m_Device->CreateShaderResourceView(b, &desc, srv.GetAddressOf()));

    auto h = m_SRVs.Push(srv);

    GraphicsResourceHandle handle = GraphicsResourceHandle(GRT_ShaderResourceView, h.gen, h.id);
    return handle;

}

GraphicsResourceHandle Dx11RenderInterface::CreateUnorderedAccessView(GraphicsResourceHandle srcBuffer, UavDesc const& desc)
{
    ComPtr<ID3D11UnorderedAccessView> uav;
    ID3D11Buffer* b = GetRawBuffer(srcBuffer);
    ENSURE_HR(m_Device->CreateUnorderedAccessView(b, &desc, &uav));

    auto h = m_UAVs.Push(uav);
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

void Dx11RenderContext::BeginFrame()
{
    m_Context->ClearState();

    m_PrevRenderState.VS = nullptr;
    m_PrevRenderState.PS = nullptr;
    m_PrevRenderState.InputLayout = nullptr;
    m_PrevRenderState.RS = nullptr;
}

void Dx11RenderContext::EndFrame()
{
}

void Dx11RenderContext::Flush()
{
    m_Context->Flush();
}
