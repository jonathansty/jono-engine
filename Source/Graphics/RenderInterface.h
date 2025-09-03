#pragma once

class IRenderInterface
{
public:
    // TODO

private:
    // TODO
};


#include "DX11/Dx11RenderInterface.h"
using RenderInterface = Dx11RenderInterface;
using RenderContext = Dx11RenderContext;

extern RenderInterface* GetRI();


