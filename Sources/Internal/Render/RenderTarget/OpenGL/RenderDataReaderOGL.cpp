/*==================================================================================
 Copyright (c) 2008, binaryzebra
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the binaryzebra nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/

#include "Render/RenderTarget/OpenGL/RenderDataReaderOGL.h"
#include "Render/RenderTarget/OpenGL/RenderTargetFactoryOGL.h"
#include "Render/RenderTarget/OpenGL/RenderTargetOGL.h"
#include "Render/RenderManager.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/Image/Image.h"
#include "Render/RenderHelper.h"

namespace DAVA
{

Image* RenderDataReaderOGL::ReadTextureData(Texture* tx, UniqueHandle renderState)
{
    RenderTargetDescriptor rtDesc;
    FillTextureDataReaderDescriptor(tx, rtDesc);

    RenderTarget* renderTarget = RenderTargetFactory::Instance()->CreateRenderTarget(rtDesc);

    RenderManager::Instance()->SetTextureState(RenderState::TEXTURESTATE_EMPTY);
    RenderManager::Instance()->FlushState();

    renderTarget->BeginRender();

    RenderHelper::Instance()->Setup2dCanvas((float32)tx->width, (float32)tx->height);

    Sprite* drawSprite = Sprite::CreateFromTexture(tx, 0, 0, (float32)tx->width, (float32)tx->height, true);

    Sprite::DrawState drawState;
    drawState.SetPosition(0, 0);
    drawState.SetRenderState(renderState);
    drawSprite->Draw(&drawState);

    SafeRelease(drawSprite);

    PixelFormat imageFormat = FORMAT_INVALID;
    uint32 width = 0;
    uint32 height = 0;
    GetImageInfo(renderTarget, imageFormat, width, height);

    Image* resultImage = ReadCurrentColorData(imageFormat, width, height);

    renderTarget->EndRender();

    SafeRelease(renderTarget);

    return resultImage;
}

bool RenderDataReaderOGL::ReadTextureDataToBuffer(Texture* tx, uint8** outData)
{
    *outData = NULL;

    RenderTargetDescriptor rtDesc;
    FillTextureDataReaderDescriptor(tx, rtDesc);

    RenderManager::Instance()->SetTextureState(RenderState::TEXTURESTATE_EMPTY);
    RenderManager::Instance()->FlushState();

    RenderTarget* renderTarget = RenderTargetFactory::Instance()->CreateRenderTarget(rtDesc);

    PixelFormat imageFormat = FORMAT_INVALID;
    uint32 width = 0;
    uint32 height = 0;
    GetImageInfo(renderTarget, imageFormat, width, height);

    const PixelFormatDescriptor& formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(imageFormat);

    bool canReadPixels = (FORMAT_RGBA8888 == imageFormat);
    if(canReadPixels)
    {
        renderTarget->BeginRender();

        RenderHelper::Instance()->Setup2dCanvas((float32)tx->width, (float32)tx->height);

        Sprite* drawSprite = Sprite::CreateFromTexture(tx, 0, 0, (float32)tx->width, (float32)tx->height, true);

        Sprite::DrawState drawState;
        drawState.SetPosition(0, 0);
        drawState.SetRenderState(RenderState::RENDERSTATE_2D_BLEND);
        drawSprite->Draw(&drawState);

        SafeRelease(drawSprite);

        *outData = new uint8[width * height * 4];

        RENDER_VERIFY(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        RENDER_VERIFY(glReadPixels(0,
                                   0,
                                   width,
                                   height,
                                   formatDescriptor.format,
                                   formatDescriptor.type,
                                   (GLvoid*)*outData));
        
        renderTarget->EndRender();
        
    }
    
    SafeRelease(renderTarget);

    return canReadPixels;
}

Image* RenderDataReaderOGL::ReadColorData(RenderTarget* renderTarget)
{
    DVASSERT(renderTarget);

    Image* resultImage = NULL;

    PixelFormat imageFormat = FORMAT_INVALID;
    uint32 width = 0;
    uint32 height = 0;

    if(GetImageInfo(renderTarget, imageFormat, width, height))
    {
        RenderTargetOGL* renderTargetOGL = static_cast<RenderTargetOGL*>(renderTarget);

        renderTargetOGL->BindRenderTarget();

        uint32 framebufferWidth = renderTargetOGL->GetColorAttachment()->GetFramebufferWidth();
        uint32 framebufferHeight = renderTargetOGL->GetColorAttachment()->GetFramebufferHeight();

        Rect viewport(0.0f, 0.0f, (float)framebufferWidth, (float32)framebufferHeight);

        RenderManager::Instance()->ClipPush();
        RenderManager::Instance()->PushDrawMatrix();
        RenderManager::Instance()->PushMappingMatrix();
        RenderManager::Instance()->IdentityDrawMatrix();

        //VI: I believe all this stuff like render orientation doesn't belong to RenderManager
        RenderManager::Instance()->SetRenderOrientation(Core::SCREEN_ORIENTATION_TEXTURE, viewport.dx, viewport.dy);
        RenderManager::Instance()->SetViewport(viewport, true);
        RenderManager::Instance()->RemoveClip();

        resultImage = ReadCurrentColorData(imageFormat, width, height);

        renderTargetOGL->UnbindRenderTarget();

        //VI: I believe all this stuff like render orientation doesn't belong to RenderManager
        RenderManager::Instance()->SetRenderOrientation(Core::Instance()->GetScreenOrientation());
        RenderManager::Instance()->PopDrawMatrix();
        RenderManager::Instance()->PopMappingMatrix();
        RenderManager::Instance()->ClipPop();
    }
    else
    {
        Logger::Error("[RenderDataReaderOGL::ReadColorData] Render target has no color attachment!");
        SafeRelease(resultImage);
    }

    DVASSERT(resultImage);
    
    return resultImage;
}

Image* RenderDataReaderOGL::ReadCurrentColorData(PixelFormat pixelFormat, uint32 width, uint32 height)
{
    Image* resultImage = NULL;

    const PixelFormatDescriptor & formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(pixelFormat);

    resultImage = Image::Create(width,
                                height,
                                formatDescriptor.formatID);
    uint8 *imageData = resultImage->GetData();

    if(FORMAT_INVALID != formatDescriptor.formatID)
    {
        RENDER_VERIFY(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        RENDER_VERIFY(glReadPixels(0,
                                   0,
                                   width,
                                   height,
                                   formatDescriptor.format,
                                   formatDescriptor.type,
                                   (GLvoid*)imageData));
    }
    else
    {
        Logger::Error("[RenderDataReaderOGL::ReadCurrentColorData] Invalid framebuffer format!");
        SafeRelease(resultImage);
    }

    DVASSERT(resultImage);
    
    return resultImage;
}

bool RenderDataReaderOGL::GetImageInfo(RenderTarget* renderTarget,
                                       PixelFormat& outPixelFormat,
                                       uint32& outWidth,
                                       uint32& outHeight)
{
    bool result = false;

    FramebufferAttachment* color0Attachment = renderTarget->GetColorAttachment(0);
    if(color0Attachment != NULL)
    {
        outPixelFormat = RenderTargetFactoryOGL::MapFramebufferFormatToOGLFormat(color0Attachment->GetFramebufferFormat());

        outWidth = color0Attachment->GetFramebufferWidth();
        outHeight = color0Attachment->GetFramebufferHeight();

        result = true;
    }
    else
    {
        Logger::Error("[RenderDataReaderOGL::GetImageInfo] Invalid render target. No color render target specified!");
    }

    return result;
}

void RenderDataReaderOGL::FillTextureDataReaderDescriptor(Texture* sourceTexture,
                                                          RenderTargetDescriptor& outDesc)
{
    DVASSERT(sourceTexture);

    FramebufferDescriptor colorFramebuffer(FramebufferDescriptor::FRAMEBUFFER_COLOR0,
                                           FramebufferDescriptor::FORMAT_RGBA8888,
                                           FramebufferDescriptor::PRE_ACTION_CLEAR,
                                           FramebufferDescriptor::POST_ACTION_STORE,
                                           sourceTexture->width,
                                           sourceTexture->height);

    outDesc.SetClearColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
    outDesc.SetClearDepth(0.0f);
    outDesc.SetClearStencil(0);

    outDesc.AddFramebuffer(colorFramebuffer);
    
    DVASSERT(outDesc.Validate());
}


};