#pragma once

#include "GraphicsResourceHandle.h"

class Dx11RenderContext
{
public:

private:
};



using BufferDesc = D3D11_BUFFER_DESC;
using SrvDesc = D3D11_SHADER_RESOURCE_VIEW_DESC;
using UavDesc = D3D11_UNORDERED_ACCESS_VIEW_DESC;

template<typename T>
class SlotVector
{
public:
    struct Handle
    {
        uint32_t id;
        uint16_t gen;
    };

    T& Get(Handle const& h) 
	{
        ASSERT(m_Generations[h.id] == h.gen);
		return m_Storage[h.id];
    }

    Handle Push(T obj)
    {
        size_t id = m_Storage.size();

		Handle h{};
        auto it = std::find_if(m_Storage.begin(), m_Storage.end(), [](T const& rhs) { return rhs == nullptr; });
        if(it != m_Storage.end())
        {
            size_t d = std::distance(m_Storage.begin(), it);

            *it = obj;
            ++m_Generations[d];

            h.gen = m_Generations[d];
            h.id = (uint32_t)d;
        }
        else
        {
			m_Generations.push_back(0);
			m_Storage.push_back(obj);

			h.id = (uint32_t)id;
			h.gen = 0;
        }
        return h;
    }
     
private:
    std::vector<uint16_t> m_Generations;
    std::vector<T> m_Storage;
};

namespace Graphics
{
class Renderer;
};

class Dx11RenderInterface
{
    // #TODO: Remove all friend classes
    friend class Graphics::Renderer;
    friend class Material;
    friend class MaterialInstance;
    friend class GraphicsThread;

public:
    void Init();

    GraphicsResourceHandle CreateBuffer(BufferDesc const& desc, void* initialData);

    GraphicsResourceHandle CreateShaderResourceView(GraphicsResourceHandle srcBuffer, SrvDesc const& desc);
    GraphicsResourceHandle CreateUnorderedAccessView(GraphicsResourceHandle srcBuffer, UavDesc const& desc);


    void* Map(GraphicsResourceHandle buffer);
    void Unmap(GraphicsResourceHandle buffer);


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

private:
    ComPtr<ID3D11Device> m_Device;
    ComPtr<ID3D11DeviceContext> m_Context;
    ComPtr<IDXGIFactory> m_Factory;

    ComPtr<IWICImagingFactory> m_WicFactory;
    ComPtr<ID3DUserDefinedAnnotation> m_UserDefinedAnnotations;

    Dx11RenderContext m_RenderContext;

    std::set<ID3D11Resource*> m_Textures;

    SlotVector<ComPtr<ID3D11Buffer>> m_Buffers;
    SlotVector<ComPtr<ID3D11ShaderResourceView>> m_SRVs;
    SlotVector<ComPtr<ID3D11UnorderedAccessView>> m_UAVs;

};
