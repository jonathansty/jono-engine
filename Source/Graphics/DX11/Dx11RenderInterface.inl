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
    m_Context->IASetInputLayout(owner->GetRawInputLayout(inputLayout));
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
