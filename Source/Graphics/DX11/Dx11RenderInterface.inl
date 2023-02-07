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

void Dx11RenderContext::IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
    m_Context->IASetPrimitiveTopology(topology);
}

void Dx11RenderContext::ExecuteComputeItem(ComputeItem const& item)
{
    std::vector<ComputeItem> i = { item };
    ExecuteComputeItems(i);
}

void Dx11RenderContext::RSSetState(ID3D11RasterizerState* rs)
{
    if (m_PrevRenderState.RS != rs)
    {
        m_Context->RSSetState(rs);
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

inline void Dx11RenderContext::OMSetDepthStencilState(ID3D11DepthStencilState* dss, uint32_t stencilRef)
{
    m_Context->OMSetDepthStencilState(dss, (UINT)stencilRef);
}

inline void Dx11RenderContext::OMSetBlendState(ID3D11BlendState* bs, std::array<float, 4> blendFactor, uint32_t sampleMask)
{
    m_Context->OMSetBlendState(bs, (FLOAT*)blendFactor.data(), sampleMask);
}

inline void Dx11RenderContext::RSSetViewports(Span<Viewport> vps)
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
