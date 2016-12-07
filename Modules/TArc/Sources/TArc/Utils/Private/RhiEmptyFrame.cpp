#include "TArc/Utils/RhiEmptyFrame.h"

#include "Render/Renderer.h"
#include "Render/RenderHelper.h"

namespace DAVA
{
namespace TArc
{
RhiEmptyFrame::RhiEmptyFrame()
{
    const rhi::HTexture nullTexture;
    const rhi::Viewport nullViewport(0, 0, 1, 1);
    Renderer::BeginFrame();
    RenderHelper::CreateClearPass(nullTexture, nullTexture, 0, Color::Clear, nullViewport);
}

RhiEmptyFrame::~RhiEmptyFrame()
{
    Renderer::EndFrame();
}
} // namespace TArc
} // namespace DAVA
