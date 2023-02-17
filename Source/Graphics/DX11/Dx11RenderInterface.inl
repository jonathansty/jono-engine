#pragma once

void Dx11RenderContext::IASetIndexBuffer(GraphicsResourceHandle const& buffer, DXGI_FORMAT format, uint32_t offset)
{
    if(buffer == GraphicsResourceHandle::Invalid())
    {
        m_Context->IASetIndexBuffer(nullptr, format, offset);
    }
    else
    {
        m_Context->IASetIndexBuffer(owner->GetRawBuffer(buffer), format, offset);
    }
}

void Dx11RenderContext::IASetInputLayout(GraphicsResourceHandle const& inputLayout)
{
    if(m_PrevRenderState.InputLayout != inputLayout)
    {
        m_Context->IASetInputLayout(owner->GetRawInputLayout(inputLayout));
    }
    m_PrevRenderState.InputLayout = inputLayout;
}

void Dx11RenderContext::SetViewport(Viewport const& vp)
{
    D3D11_VIEWPORT dx11VP = { vp.x, vp.y, vp.width, vp.height, vp.minZ, vp.maxZ };
    m_Context->RSSetViewports(1, &dx11VP);
}
void Dx11RenderContext::SetScissorRects(Span<Rect> rects)
{
    D3D11_RECT dx11r[16];
    size_t numRects = std::min<size_t>(16, rects.size());
    for (size_t i =0; i < numRects; ++i)
    {
        Rect const& r = rects[i];
        dx11r[i] = D3D11_RECT{ 
            (LONG)r.topLeftX,
            (LONG)r.topLeftY,
            (LONG)r.bottomRightX,
            (LONG)r.bottomRightY 
        };
    }
    m_Context->RSSetScissorRects((UINT)numRects, dx11r);
}


void Dx11RenderContext::IASetPrimitiveTopology(PrimitiveTopology topology)
{
    m_Context->IASetPrimitiveTopology(Dx11GetPrimitiveTopology(topology));
}

void Dx11RenderContext::ExecuteComputeItem(ComputeItem const& item)
{
    ExecuteComputeItems({ item });
}

void Dx11RenderContext::RSSetState(GraphicsResourceHandle rs)
{
    ASSERT(rs.data.type == GRT_RasterizerState);
    if (m_PrevRenderState.RS != rs)
    {
        m_Context->RSSetState(owner->GetRawResourceAs<ID3D11RasterizerState>(rs));
    }
    m_PrevRenderState.RS = rs;
}

void Dx11RenderContext::PSSetShader(ID3D11PixelShader* ps)
{
    if (m_PrevRenderState.PS != ps)
    {
        m_Context->PSSetShader(ps, nullptr, 0);
    }
    m_PrevRenderState.PS = ps;
}

void Dx11RenderContext::VSSetShader(ID3D11VertexShader* vs)
{
    if (m_PrevRenderState.VS != vs)
    {
        m_Context->VSSetShader(vs, nullptr, 0);
    }
    m_PrevRenderState.VS = vs;
}

inline void Dx11RenderContext::OMSetDepthStencilState(GraphicsResourceHandle dss, uint32_t stencilRef)
{
    ASSERT(dss.GetType() == GRT_DepthStencilState);
    m_Context->OMSetDepthStencilState(owner->GetRawResourceAs<ID3D11DepthStencilState>(dss), (UINT)stencilRef);
}

inline void Dx11RenderContext::OMSetBlendState(GraphicsResourceHandle bs, std::array<float, 4> blendFactor, uint32_t sampleMask)
{
    ASSERT(bs.GetType() == GRT_BlendState);
    m_Context->OMSetBlendState(owner->GetRawResourceAs<ID3D11BlendState>(bs), (FLOAT*)blendFactor.data(), sampleMask);
}

inline void Dx11RenderContext::SetViewports(Span<Viewport> vps)
{
    std::array<D3D11_VIEWPORT, 16> viewports;
    ASSERT(vps.size() < viewports.size());
    for (size_t i = 0; i < std::min(vps.size(), viewports.size()); ++i)
    {
        viewports[i].TopLeftX = vps[i].x;
        viewports[i].TopLeftY = vps[i].y;
        viewports[i].Width = vps[i].width;
        viewports[i].Height = vps[i].height;
        viewports[i].MinDepth = vps[i].minZ;
        viewports[i].MaxDepth = vps[i].maxZ;
    }

    m_Context->RSSetViewports((UINT)vps.size(), viewports.data());
}

void Dx11RenderContext::IASetVertexBuffers(uint32_t startSlot, Span<GraphicsResourceHandle const> buffers, Span<UINT const> strides, Span<UINT const> offsets)
{
    if(buffers.size() == 0)
    {
        m_Context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    }
    else
    {
        // #TODO: Set as an array rather than each buffer independently
        uint32_t slot = startSlot;
        size_t j = 0;
        for(GraphicsResourceHandle const& handle : buffers)
        {
            UINT rawStrides = strides[j];
            UINT rawOffsets = offsets[j];
            ID3D11Buffer* b = owner->GetRawBuffer(handle);
            m_Context->IASetVertexBuffers(slot, 1, &b, &rawStrides, &rawOffsets);
        }
    }
}

Texture2DDesc Dx11RenderInterface::GetTexture2DDesc(GraphicsResourceHandle const& resource) const
{
    ID3D11Texture2D* tex = GetRawTexture2D(resource);

    Texture2DDesc desc{};
    tex->GetDesc(&desc);

    return desc;
}

void Dx11RenderContext::DrawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset)
{
    m_Context->DrawIndexed((UINT)indexCount, (UINT)indexOffset, (UINT)vertexOffset);
}
