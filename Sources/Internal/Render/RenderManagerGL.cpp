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


#include "Render/RenderBase.h"
#include "Render/RenderManager.h"
#include "Render/Texture.h"
#include "Render/2D/Sprite.h"
#include "Utils/Utils.h"
#include "Core/Core.h"
#include "Render/OGLHelpers.h"
#include "Render/Shader.h"

#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"
#include "FileSystem/FileSystem.h"
#include "Utils/StringFormat.h"
#include "Render/PixelFormatDescriptor.h"

#ifdef __DAVAENGINE_OPENGL__

namespace DAVA
{
	
#if defined(__DAVAENGINE_WIN32__)

static HDC hDC;
static HGLRC hRC;
static HWND hWnd;
static HINSTANCE hInstance;

bool RenderManager::Create(HINSTANCE _hInstance, HWND _hWnd)
{
	hInstance = _hInstance;
	hWnd = _hWnd;

	hDC = GetDC(hWnd);

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int iFormat = ChoosePixelFormat( hDC, &pfd );
	SetPixelFormat( hDC, iFormat, &pfd );

	hRC = wglCreateContext(hDC);

	renderContextId = (uint64)hRC;
	
	Thread::secondaryContext = wglCreateContext(hDC);
	Thread::currentDC = hDC;

	wglShareLists(Thread::secondaryContext, hRC);
	wglMakeCurrent(hDC, hRC);

	glewInit();

	return true;
}

void RenderManager::Release()
{
	Singleton<RenderManager>::Release();

	wglMakeCurrent(0, 0);
	wglDeleteContext(hRC);
	ReleaseDC(hWnd, hDC);	
}

bool RenderManager::ChangeDisplayMode(DisplayMode mode, bool isFullscreen)
{
	hardwareState.Reset(false);
	currentState.Reset(true);
	return true;
}	
	
#else //#if defined(__DAVAENGINE_WIN32__)

bool RenderManager::Create()
{
	return true;
}
	
void RenderManager::Release()
{
	
}

#endif //#if defined(__DAVAENGINE_WIN32__)


bool IsGLExtensionSupported(const String &extension)
{
    String::size_type spacePosition = extension.find(" ");
    if(String::npos != spacePosition || extension.empty())
    {
        /* Extension names should not have spaces. */
        Logger::FrameworkDebug("[IsGLExtensionSupported] extension %s isn't supported", extension.c_str());
        return false;
    }
    
    String extensions((const char8 *)glGetString(GL_EXTENSIONS));
    String::size_type extPosition = extensions.find(extension);
    return (String::npos != extPosition);
}

    
void RenderManager::DetectRenderingCapabilities()
{
#if defined(__DAVAENGINE_MACOS__)
	caps.isHardwareCursorSupported = true;
#elif defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
	caps.isHardwareCursorSupported = false;
#endif

#if defined(__DAVAENGINE_IPHONE__)
    caps.isPVRTCSupported = IsGLExtensionSupported("GL_IMG_texture_compression_pvrtc");
	caps.isPVRTC2Supported = IsGLExtensionSupported("GL_IMG_texture_compression_pvrtc2");
    caps.isETCSupported = false;

	caps.isDXTSupported = false;
    caps.isBGRA8888Supported = IsGLExtensionSupported("GL_APPLE_texture_format_BGRA8888");
    caps.isFloat16Supported = IsGLExtensionSupported("GL_OES_texture_half_float");
    caps.isFloat32Supported = IsGLExtensionSupported("GL_OES_texture_float");
	caps.isATCSupported = IsGLExtensionSupported("GL_AMD_compressed_ATC_texture");
#elif defined(__DAVAENGINE_ANDROID__)
    //TODO: added correct
    caps.isPVRTCSupported = IsGLExtensionSupported("GL_IMG_texture_compression_pvrtc");
	caps.isPVRTC2Supported = IsGLExtensionSupported("GL_IMG_texture_compression_pvrtc2");
    caps.isETCSupported = IsGLExtensionSupported("GL_OES_compressed_ETC1_RGB8_texture");

	caps.isDXTSupported = IsGLExtensionSupported("GL_EXT_texture_compression_s3tc");
    caps.isBGRA8888Supported = false;
    caps.isFloat16Supported = IsGLExtensionSupported("GL_OES_texture_half_float");
    caps.isFloat32Supported = IsGLExtensionSupported("GL_OES_texture_float");
	caps.isATCSupported = IsGLExtensionSupported("GL_AMD_compressed_ATC_texture");
    caps.isGlDepth24Stencil8Supported = IsGLExtensionSupported("GL_DEPTH24_STENCIL8");
    caps.isGlDepthNvNonLinearSupported = IsGLExtensionSupported("GL_DEPTH_COMPONENT16_NONLINEAR_NV");
    
#   if (__ANDROID_API__ < 18)
    InitFakeOcclusion();
#   endif

#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)

	caps.isPVRTCSupported = false;

#if defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT) || defined(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) || defined(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) || defined(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
	caps.isDXTSupported = IsGLExtensionSupported("GL_EXT_texture_compression_s3tc");
#else //DXT
	caps.isDXTSupported = false;
#endif //DXT

#if defined(GL_ETC1_RGB8_OES)
	caps.isETCSupported = IsGLExtensionSupported("GL_OES_compressed_ETC1_RGB8_texture");
#else //ETC1
	caps.isETCSupported = false;
#endif //ETC1

#if defined(GL_ATC_RGB_AMD) || defined(GL_ATC_RGBA_EXPLICIT_ALPHA_AMD) || defined(GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD)
	caps.isATCSupported = IsGLExtensionSupported("GL_AMD_compressed_ATC_texture");
#else //ATC
	caps.isATCSupported = false;
#endif //ATC

	caps.isBGRA8888Supported = IsGLExtensionSupported("GL_IMG_texture_format_BGRA8888");
    caps.isFloat16Supported = IsGLExtensionSupported("GL_ARB_half_float_pixel");
    caps.isFloat32Supported = IsGLExtensionSupported("GL_ARB_texture_float");
#endif

    caps.isOpenGLES3Supported = (renderer == Core::RENDERER_OPENGL_ES_3_0);

	PixelFormatDescriptor::InitializePixelFormatDescriptors();

    int maxVertexTextureUnits = 0;
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxVertexTextureUnits);
    caps.isVertexTextureUnitsSupported = (maxVertexTextureUnits > 0);
    caps.isFramebufferFetchSupported = IsGLExtensionSupported("GL_EXT_shader_framebuffer_fetch");
}

bool RenderManager::IsDeviceLost()
{
	return false;
}

void RenderManager::BeginFrame()
{
    stats.Clear();
    SetViewport(Rect(0, 0, -1, -1), true);
	
	SetRenderOrientation(Core::Instance()->GetScreenOrientation());
	DVASSERT(!currentRenderTarget);
	//DVASSERT(!currentRenderEffect);
	DVASSERT(clipStack.empty());
	DVASSERT(renderTargetStack.empty());
	Reset();
	isInsideDraw = true;
    
    //ClearUniformMatrices();
}
	
//void RenderManager::SetDrawOffset(const Vector2 &offset)
//{
//	glMatrixMode(GL_PROJECTION);
//	glTranslatef(offset.x, offset.y, 0.0f);
//	glMatrixMode(GL_MODELVIEW);
//}

void RenderManager::PrepareRealMatrix()
{
    if (mappingMatrixChanged)
    {
        mappingMatrixChanged = false;
        Vector2 realDrawScale(viewMappingDrawScale.x * userDrawScale.x, viewMappingDrawScale.y * userDrawScale.y);
        Vector2 realDrawOffset(viewMappingDrawOffset.x + userDrawOffset.x * viewMappingDrawScale.x, viewMappingDrawOffset.y + userDrawOffset.y * viewMappingDrawScale.y);
	
        if (realDrawScale != currentDrawScale || realDrawOffset != currentDrawOffset)
        {
            currentDrawScale = realDrawScale;
            currentDrawOffset = realDrawOffset;

            Matrix4 glTranslate, glScale;
            glTranslate.glTranslate(currentDrawOffset.x, currentDrawOffset.y, 0.0f);
            glScale.glScale(currentDrawScale.x, currentDrawScale.y, 1.0f);
            
            renderer2d.viewMatrix = glScale * glTranslate;
            SetDynamicParam(PARAM_VIEW, &renderer2d.viewMatrix, UPDATE_SEMANTIC_ALWAYS);
        }
    }
    
    Matrix4 glTranslate, glScale;
    glTranslate.glTranslate(currentDrawOffset.x, currentDrawOffset.y, 0.0f);
    glScale.glScale(currentDrawScale.x, currentDrawScale.y, 1.0f);
    
    Matrix4 check = glScale * glTranslate;
    DVASSERT(check == renderer2d.viewMatrix);
}
	

void RenderManager::EndFrame()
{
	isInsideDraw = false;
#if defined(__DAVAENGINE_WIN32__)
	::SwapBuffers(hDC);
#endif //#if defined(__DAVAENGINE_WIN32__)
	
	RENDER_VERIFY(;);	// verify at the end of the frame
    
    if(needGLScreenShot)
    {
        needGLScreenShot = false;
        MakeGLScreenShot();
    }
}
    
void RenderManager::MakeGLScreenShot()
{
    Logger::FrameworkDebug("RenderManager::MakeGLScreenShot");
#if defined(__DAVAENGINE_OPENGL__)
    

    int32 width = frameBufferWidth;
    int32 height = frameBufferHeight;
    
    const PixelFormatDescriptor & formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(FORMAT_RGBA8888);
    
    Logger::FrameworkDebug("RenderManager::MakeGLScreenShot w=%d h=%d", width, height);
    
    // picture is rotated (framebuffer coordinates start from bottom left)
    Image *image = NULL;
    image = Image::Create(width, height, formatDescriptor.formatID);
    uint8 *imageData = image->GetData();
    
    int32 formatSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(formatDescriptor.formatID);
    uint8 *tempData;
    
    uint32 imageDataSize = width * height * formatSize;
    tempData = new uint8[imageDataSize];

    RENDER_VERIFY(glBindFramebuffer(GL_FRAMEBUFFER, fboViewRenderbuffer));
//#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
//    glBindFramebuffer(GL_FRAMEBUFFER_BINDING_OES, fboViewRenderbuffer);
//#else
//    glBindFramebuffer(GL_FRAMEBUFFER_BINDING_EXT, fboViewRenderbuffer);
//#endif
    
    RENDER_VERIFY(glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ));
    RENDER_VERIFY(glReadPixels(0, 0, width, height, formatDescriptor.format, formatDescriptor.type, (GLvoid *)tempData));
    
    //TODO: optimize (ex. use pre-allocated buffer instead of dynamic allocation)
    
    // iOS frame buffer starts from bottom left corner, but we need from top left, so we rotate picture here
    uint32 newIndex = 0;
    uint32 oldIndex = 0;

    //MacOS
    //TODO: test on Windows and android

    for(int32 h = height - 1; h >= 0; --h)
    {
        for(int32 w = 0; w < width; ++w)
        {
            for(int32 b = 0; b < formatSize; ++b)
            {
                oldIndex = formatSize*width*h + formatSize*w + b;
                imageData[newIndex++] = tempData[oldIndex];
            }
        }
    }
    
    SafeDeleteArray(tempData);
    
    if(screenShotCallback)
    {
		(*screenShotCallback)(image);
    }
	SafeRelease(image);
    
#endif //#if defined(__DAVAENGINE_OPENGL__)
}
    
void RenderManager::SetViewport(const Rect & rect, bool precaleulatedCoordinates)
{    
    if ((rect.dx < 0.0f) && (rect.dy < 0.0f))
    {
        viewport = rect;
        if (currentRenderTarget)
        {
            RENDER_VERIFY(glViewport(0, 0, currentRenderTarget->GetTexture()->GetWidth(), currentRenderTarget->GetTexture()->GetHeight()));
        }
        else
        {
            RENDER_VERIFY(glViewport(0, 0, frameBufferWidth, frameBufferHeight));
        }
        return;
    }
    if (precaleulatedCoordinates) 
    {
        viewport = rect;
        RENDER_VERIFY(glViewport((int32)rect.x, (int32)rect.y, (int32)rect.dx, (int32)rect.dy));
        return;
    }

    PrepareRealMatrix();
    

	int32 x = (int32)(rect.x * currentDrawScale.x + currentDrawOffset.x);
	int32 y = (int32)(rect.y * currentDrawScale.y + currentDrawOffset.y);
	int32 width, height;
    width = (int32)(rect.dx * currentDrawScale.x);
    height = (int32)(rect.dy * currentDrawScale.y);    
    
    if (renderOrientation!=Core::SCREEN_ORIENTATION_TEXTURE)
    {
        y = frameBufferHeight - y - height;
    }
    
    RENDER_VERIFY(glViewport(x, y, width, height));
    viewport.x = (float32)x;
    viewport.y = (float32)y;
    viewport.dx = (float32)width;
    viewport.dy = (float32)height;
}

    

// Viewport management
void RenderManager::SetRenderOrientation(int32 orientation)
{
	renderOrientation = orientation;
	
    if (orientation != Core::SCREEN_ORIENTATION_TEXTURE)
    {
        renderer2d.projMatrix.glOrtho(0.0f, (float32)frameBufferWidth, (float32)frameBufferHeight, 0.0f, -1.0f, 1.0f);
    }else{
        renderer2d.projMatrix.glOrtho(0.0f, (float32)currentRenderTarget->GetTexture()->GetWidth(),
                                      0.0f, (float32)currentRenderTarget->GetTexture()->GetHeight(), -1.0f, 1.0f);
    }
    retScreenWidth = frameBufferWidth;
    retScreenHeight = frameBufferHeight;
	
    
    SetDynamicParam(PARAM_PROJ, &renderer2d.projMatrix, UPDATE_SEMANTIC_ALWAYS);

    IdentityModelMatrix();
    
	RENDER_VERIFY(;);

	IdentityMappingMatrix();
	SetVirtualViewScale();
	SetVirtualViewOffset();

}

void RenderManager::SetCullOrder(eCullOrder cullOrder)
{
    glFrontFace(cullOrder);
}
    
void RenderManager::FlushState()
{
	PrepareRealMatrix();
    
    currentState.Flush(&hardwareState);
}

void RenderManager::FlushState(RenderState * stateBlock)
{
	PrepareRealMatrix();
	
	stateBlock->Flush(&hardwareState);
}

void RenderManager::HWDrawArrays(ePrimitiveType type, int32 first, int32 count)
{
	static const int32 types[PRIMITIVETYPE_COUNT] = 
	{
		GL_POINTS,			// 		PRIMITIVETYPE_POINTLIST = 0,
		GL_LINES,			// 		PRIMITIVETYPE_LINELIST,
		GL_LINE_STRIP,		// 		PRIMITIVETYPE_LINESTRIP,
		GL_TRIANGLES,		// 		PRIMITIVETYPE_TRIANGLELIST,
		GL_TRIANGLE_STRIP,	// 		PRIMITIVETYPE_TRIANGLESTRIP,
		GL_TRIANGLE_FAN,	// 		PRIMITIVETYPE_TRIANGLEFAN,
	};

	GLuint mode = types[type];

	if(debugEnabled)
	{
		Logger::FrameworkDebug("Draw arrays texture stated: id %d", currentState.textureState);
	}

    RENDER_VERIFY(glDrawArrays(mode, first, count));
    stats.drawArraysCalls++;
    switch(type)
    {
        case PRIMITIVETYPE_POINTLIST: 
            stats.primitiveCount[type] += count;
            break;
        case PRIMITIVETYPE_LINELIST:
            stats.primitiveCount[type] += count / 2;
            break;
        case PRIMITIVETYPE_LINESTRIP:
            stats.primitiveCount[type] += count - 1;
            break;
        case PRIMITIVETYPE_TRIANGLELIST:
            stats.primitiveCount[type] += count / 3;
            break;
        case PRIMITIVETYPE_TRIANGLEFAN:
        case PRIMITIVETYPE_TRIANGLESTRIP:
            stats.primitiveCount[type] += count - 2;
            break;
        default:
            break;
    };
}

void RenderManager::HWDrawElements(ePrimitiveType type, int32 count, eIndexFormat indexFormat, void * indices)
{
	static const int32 types[PRIMITIVETYPE_COUNT] = 
	{
		GL_POINTS,			// 		PRIMITIVETYPE_POINTLIST = 0,
		GL_LINES,			// 		PRIMITIVETYPE_LINELIST,
		GL_LINE_STRIP,		// 		PRIMITIVETYPE_LINESTRIP,
		GL_TRIANGLES,		// 		PRIMITIVETYPE_TRIANGLELIST,
		GL_TRIANGLE_STRIP,	// 		PRIMITIVETYPE_TRIANGLESTRIP,
		GL_TRIANGLE_FAN,	// 		PRIMITIVETYPE_TRIANGLEFAN,
	};
	
	GLuint mode = types[type];
	
	if(debugEnabled)
	{
		Logger::FrameworkDebug("Draw arrays texture state: id %d", currentState.textureState);
	}
#if defined(__DAVAENGINE_IPHONE__)
#if not defined(GL_UNSIGNED_INT)
#define GL_UNSIGNED_INT 0
#endif //not defined(GL_UNSIGNED_INT)
#endif // __DAVAENGINE_IPHONE__

	static const int32 indexTypes[2] = 
	{
		GL_UNSIGNED_SHORT, 
		GL_UNSIGNED_INT,
	};
	
	RENDER_VERIFY(glDrawElements(mode, count, indexTypes[indexFormat], indices));
    stats.drawElementsCalls++;
    switch(type)
    {
        case PRIMITIVETYPE_POINTLIST: 
            stats.primitiveCount[type] += count;
            break;
        case PRIMITIVETYPE_LINELIST:
            stats.primitiveCount[type] += count / 2;
            break;
        case PRIMITIVETYPE_LINESTRIP:
            stats.primitiveCount[type] += count - 1;
            break;
        case PRIMITIVETYPE_TRIANGLELIST:
            stats.primitiveCount[type] += count / 3;
            break;
        case PRIMITIVETYPE_TRIANGLEFAN:
        case PRIMITIVETYPE_TRIANGLESTRIP:
            stats.primitiveCount[type] += count - 2;
            break;
        default:
            break;
    };
}

void RenderManager::ClearWithColor(float32 r, float32 g, float32 b, float32 a)
{
	RENDER_VERIFY(glClearColor(r, g, b, a));
	RENDER_VERIFY(glClear(GL_COLOR_BUFFER_BIT));
}

void RenderManager::ClearDepthBuffer(float32 depth)
{
#if defined(__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glClearDepthf(depth));
#else //#if defined(__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glClearDepth(depth));
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glClear(GL_DEPTH_BUFFER_BIT));
}

void RenderManager::ClearStencilBuffer(int32 stencil)
{
	RENDER_VERIFY(glClearStencil(stencil));
	RENDER_VERIFY(glClear(GL_STENCIL_BUFFER_BIT));
}
    
void RenderManager::Clear(const Color & color, float32 depth, int32 stencil)
{
    RENDER_VERIFY(glClearColor(color.r, color.g, color.b, color.a));
#if defined(__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glClearDepthf(depth));
#else //#if defined(__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glClearDepth(depth));
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glClearStencil(stencil));

    
    RENDER_VERIFY(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

void RenderManager::SetHWClip(const Rect &rect)
{
	PrepareRealMatrix();
	currentClip = rect;
	if(rect.dx < 0 || rect.dy < 0)
	{
		RENDER_VERIFY(glDisable(GL_SCISSOR_TEST));
		return;
	}
	int32 x = (int32)(rect.x * currentDrawScale.x + currentDrawOffset.x);
	int32 y = (int32)(rect.y * currentDrawScale.y + currentDrawOffset.y);
	int32 x2= (int32)ceilf((rect.dx + rect.x) * currentDrawScale.x + currentDrawOffset.x);
	int32 y2= (int32)ceilf((rect.dy + rect.y) * currentDrawScale.y + currentDrawOffset.y);
	int32 width = x2 - x;
	int32 height = y2 - y;
    
    if (renderOrientation!=Core::SCREEN_ORIENTATION_TEXTURE)
    {
        y = frameBufferHeight/* * Core::GetVirtualToPhysicalFactor()*/ - y - height;
    }
	
	RENDER_VERIFY(glEnable(GL_SCISSOR_TEST));
	RENDER_VERIFY(glScissor(x, y, width, height));
}


void RenderManager::SetHWRenderTargetSprite(Sprite *renderTarget)
{
    currentRenderTarget = renderTarget;
    
	if (renderTarget == NULL)
	{
//#if defined(__DAVAENGINE_IPHONE__)
//		RENDER_VERIFY(glBindFramebufferOES(GL_FRAMEBUFFER_OES, fboViewFramebuffer));
//#elif defined(__DAVAENGINE_ANDROID__)
////        renderTarget->GetTexture()->renderTargetModified = true;
//#else //Non ES platforms
//		RENDER_VERIFY(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboViewFramebuffer));
//#endif //PLATFORMS
        HWglBindFBO(fboViewFramebuffer);

        
        SetViewport(Rect(0, 0, -1, -1), true);

		SetRenderOrientation(Core::Instance()->GetScreenOrientation());
	}
	else
	{
		renderOrientation = Core::SCREEN_ORIENTATION_TEXTURE;
//#if defined(__DAVAENGINE_IPHONE__)
//		RENDER_VERIFY(glBindFramebufferOES(GL_FRAMEBUFFER_OES, renderTarget->GetTexture()->fboID));
//#elif defined(__DAVAENGINE_ANDROID__)
//		BindFBO(renderTarget->GetTexture()->fboID);
//        renderTarget->GetTexture()->renderTargetModified = true;
//#else //Non ES platforms
//		RENDER_VERIFY(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderTarget->GetTexture()->fboID));
//#endif //PLATFORMS
		HWglBindFBO(renderTarget->GetTexture()->fboID);
//#if defined(__DAVAENGINE_ANDROID__)
//        renderTarget->GetTexture()->renderTargetModified = true;
//#endif //#if defined(__DAVAENGINE_ANDROID__)

        
        SetViewport(Rect(0, 0, (float32)(renderTarget->GetTexture()->width), (float32)(renderTarget->GetTexture()->height)), true);

//		RENDER_VERIFY(glMatrixMode(GL_PROJECTION));
//		RENDER_VERIFY(glLoadIdentity());
//#if defined(__DAVAENGINE_IPHONE__)
//		RENDER_VERIFY(glOrthof(0.0f, renderTarget->GetTexture()->width, 0.0f, renderTarget->GetTexture()->height, -1.0f, 1.0f));
//#else 
//		RENDER_VERIFY(glOrtho(0.0f, renderTarget->GetTexture()->width, 0.0f, renderTarget->GetTexture()->height, -1.0f, 1.0f));
//#endif

        renderer2d.projMatrix.glOrtho(0.0f, (float32)renderTarget->GetTexture()->width, 0.0f, (float32)renderTarget->GetTexture()->height, -1.0f, 1.0f);
        SetDynamicParam (PARAM_PROJ, &renderer2d.projMatrix, UPDATE_SEMANTIC_ALWAYS);
        
		//RENDER_VERIFY(glMatrixMode(GL_MODELVIEW));
		//RENDER_VERIFY(glLoadIdentity());
        IdentityModelMatrix();
		IdentityMappingMatrix(); 

		viewMappingDrawScale.x = renderTarget->GetResourceToPhysicalFactor();
		viewMappingDrawScale.y = renderTarget->GetResourceToPhysicalFactor();
        mappingMatrixChanged = true;
//		Logger::FrameworkDebug("Sets with render target: Scale %.4f,    Offset: %.4f, %.4f", viewMappingDrawScale.x, viewMappingDrawOffset.x, viewMappingDrawOffset.y);
		RemoveClip();
	}
}

void RenderManager::SetHWRenderTargetTexture(Texture * renderTarget)
{
    //currentRenderTarget = renderTarget;
	renderOrientation = Core::SCREEN_ORIENTATION_TEXTURE;
	//IdentityModelMatrix();
	//IdentityMappingMatrix();
	HWglBindFBO(renderTarget->fboID);
	//RemoveClip();
}


void RenderManager::DiscardFramebufferHW(uint32 attachments)
{
#ifdef __DAVAENGINE_IPHONE__
    if (!attachments) 
      return;
    GLenum discards[3];
    int32 discardsCount=0;
    if (attachments&COLOR_ATTACHMENT)
        discards[discardsCount++]=GL_COLOR_ATTACHMENT0;
    if (attachments&DEPTH_ATTACHMENT)
        discards[discardsCount++]=GL_DEPTH_ATTACHMENT;
    if (attachments&STENCIL_ATTACHMENT)
        discards[discardsCount++]=GL_STENCIL_ATTACHMENT;
    RENDER_VERIFY(glDiscardFramebufferEXT(GL_FRAMEBUFFER, discardsCount, discards));
#endif
}
    
void RenderManager::HWglBindBuffer(GLenum target, GLuint buffer)
{
    DVASSERT(target - GL_ARRAY_BUFFER <= 1);
    if (bufferBindingId[target - GL_ARRAY_BUFFER] != buffer)
    {
        RENDER_VERIFY(glBindBuffer(target, buffer));
        bufferBindingId[target - GL_ARRAY_BUFFER] = buffer;
    }
}
    
void RenderManager::AttachRenderData()
{
    if (!currentRenderData)return;
    RENDERER_UPDATE_STATS(attachRenderDataCount++);
    
	Shader * shader = hardwareState.shader;
    uint32 currentAttributeMask = shader->GetRequiredVertexFormat();
    
    if (attachedRenderData == currentRenderData && cachedAttributeMask == currentAttributeMask)
    {
        if ((attachedRenderData->vboBuffer != 0) && (attachedRenderData->indexBuffer != 0))
        {
            RENDERER_UPDATE_STATS(attachRenderDataSkipCount++);
            return;
        }
    }
    
    bool vboChanged = ((NULL == attachedRenderData) || (NULL == currentRenderData) || (!currentRenderData->HasVertexAttachment()) || (attachedRenderData->vboBuffer != currentRenderData->vboBuffer) || (0 == currentRenderData->vboBuffer));
    bool iboChanged = ((NULL == attachedRenderData) || (NULL == currentRenderData) || (!currentRenderData->HasVertexAttachment()) || (attachedRenderData->indexBuffer != currentRenderData->indexBuffer) || (0 == currentRenderData->indexBuffer));
    
    attachedRenderData = currentRenderData;
    
    const int DEBUG = 0;
    
    {
        if(iboChanged)
        {
            HWglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentRenderData->indexBuffer);
        }
    
    
        if(vboChanged)
        {
            int32 currentEnabledStreams = 0;
            
            HWglBindBuffer(GL_ARRAY_BUFFER, currentRenderData->vboBuffer);
            
            int32 size = (int32)currentRenderData->streamArray.size();
            
            //DVASSERT(size >= shader->GetAttributeCount() && "Shader attribute count higher than model attribute count");
            if (size < shader->GetAttributeCount())
            {
                // Logger::Error("Shader attribute count higher than model attribute count");
            }
            
            for (int32 k = 0; k < size; ++k)
            {
                RenderDataStream * stream = currentRenderData->streamArray[k];
                GLboolean normalized = GL_FALSE;
                
                int32 attribIndex = shader->GetAttributeIndex(stream->formatMark);
                if (attribIndex != -1)
                {
                    int32 attribIndexBitPos = (1 << attribIndex);
                    
                    if(TYPE_UNSIGNED_BYTE == stream->type)
                    {
                        normalized = GL_TRUE;
                    }
                    RENDER_VERIFY(glVertexAttribPointer(attribIndex, stream->size, VERTEX_DATA_TYPE_TO_GL[stream->type], normalized, stream->stride, stream->pointer));
                    if (DEBUG)Logger::FrameworkDebug("shader glVertexAttribPointer: %d", attribIndex);
                    
                    if (!(cachedEnabledStreams & attribIndexBitPos))  // enable only if it was not enabled on previous step
                    {
                        RENDER_VERIFY(glEnableVertexAttribArray(attribIndex));
                        if (DEBUG)Logger::FrameworkDebug("shader glEnableVertexAttribArray: %d", attribIndex);
                    }
                    
                    currentEnabledStreams |= attribIndexBitPos;
                }
            }
            
            if(cachedEnabledStreams != currentEnabledStreams)
            {
                // now we should disable all attribs, that are was enable previously and should not be enabled now
                int attribIndex = 0;
                int32 streamsToDisable = (cachedEnabledStreams ^ currentEnabledStreams) & cachedEnabledStreams;
                while(0 != streamsToDisable)
                {
                    if(streamsToDisable & 0x1)
                    {
                        if(DEBUG)Logger::FrameworkDebug("shader glDisableVertexAttribArray: %d", attribIndex);
                        RENDER_VERIFY(glDisableVertexAttribArray(attribIndex));
                    }
                    
                    streamsToDisable = streamsToDisable >> 1;
                    attribIndex++;
                }
                
                cachedEnabledStreams = currentEnabledStreams;
            }
        }
        
        cachedAttributeMask = currentAttributeMask;
    }
}


//void RenderManager::InitGL20()
//{
//    colorOnly = 0;
//    colorWithTexture = 0;
//    
//    
//    colorOnly = new Shader();
//    colorOnly->LoadFromYaml("~res:/Shaders/Default/fixed_func_color_only.shader");
//    colorWithTexture = new Shader();
//    colorWithTexture->LoadFromYaml("~res:/Shaders/Default/fixed_func_texture.shader");
//        
//    
//}
//
//void RenderManager::ReleaseGL20()
//{
//    SafeRelease(colorOnly);
//    SafeRelease(colorWithTexture);
//}
//

    
    
int32 RenderManager::HWglGetLastTextureID(int textureType)
{
    return lastBindedTexture[textureType];

    
//#if defined(__DAVAENGINE_ANDROID__)
//    return lastBindedTexture;
//#else //#if defined(__DAVAENGINE_ANDROID__)
//    int32 saveId = 0;
//    glGetIntegerv(GL_TEXTURE_BINDING_2D, &saveId);
//    //    GLenum err = glGetError();
//    //    if (err != GL_NO_ERROR)
//    //        Logger::Error("%s file:%s line:%d gl failed with errorcode: 0x%08x", "glGetIntegerv(GL_TEXTURE_BINDING_2D, saveId)", __FILE__, __LINE__, err);
//    return saveId;
//#endif //#if defined(__DAVAENGINE_ANDROID__)
}
	
void RenderManager::HWglBindTexture(int32 tId, uint32 textureType)
{
    if(0 != tId)
    {
        RENDER_VERIFY(glBindTexture((Texture::TEXTURE_2D == textureType) ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP, tId));
        
        		//GLenum err = glGetError();
        		//if (err != GL_NO_ERROR)
        		//	Logger::Error("%s file:%s line:%d gl failed with errorcode: 0x%08x", "glBindTexture(GL_TEXTURE_2D, tId)", __FILE__, __LINE__, err);
        
        lastBindedTexture[textureType] = tId;
		lastBindedTextureType = textureType;
    }
}
	
void RenderManager::HWglForceBindTexture(int32 tId, uint32 textureType)
{
	glBindTexture((Texture::TEXTURE_2D == textureType) ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP, tId);
	
	//GLenum err = glGetError();
	//if (err != GL_NO_ERROR)
	//	Logger::Error("%s file:%s line:%d gl failed with errorcode: 0x%08x", "glBindTexture(GL_TEXTURE_2D, tId)", __FILE__, __LINE__, err);
	
	lastBindedTexture[textureType] = tId;
	lastBindedTextureType = textureType;
}

int32 RenderManager::HWglGetLastFBO()
{
    return lastBindedFBO;
//    int32 saveFBO = 0;
//#if defined(__DAVAENGINE_IPHONE__)
//    RENDER_VERIFY(glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &saveFBO));
//#elif defined(__DAVAENGINE_ANDROID__)
//    saveFBO = lastBindedFBO;
//#else //Non ES platforms
//    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &saveFBO);
//    
//    //    GLenum err = glGetError();
//    //    if (err != GL_NO_ERROR)
//    //        Logger::Error("%s file:%s line:%d gl failed with errorcode: 0x%08x", "glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &saveFBO)", __FILE__, __LINE__, err);
//    
//#endif //PLATFORMS
//    
//    return saveFBO;
}

void RenderManager::HWglBindFBO(const int32 fbo)
{
    //	if(0 != fbo)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);	// Unbind the FBO for now
//#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
//        glBindFramebufferOES(GL_FRAMEBUFFER_OES, fbo);	// Unbind the FBO for now
//#else //Non ES platforms
//        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);	// Unbind the FBO for now
//#endif //PLATFORMS
        
        //		GLenum err = glGetError();
        //		if (err != GL_NO_ERROR)
        //			Logger::Error("%s file:%s line:%d gl failed with errorcode: 0x%08x", "glBindFramebuffer(GL_FRAMEBUFFER_, tId)", __FILE__, __LINE__, err);
        
        
        lastBindedFBO = fbo;
    }
}
    
void RenderManager::DiscardDepth()
{
#ifdef __DAVAENGINE_IPHONE__
    static const GLenum discards[]  = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    RENDER_VERIFY(glDiscardFramebufferEXT(GL_FRAMEBUFFER,2,discards));
#endif
}

#if defined(__DAVAENGINE_ANDROID__)
void RenderManager::Lost()
{
    cachedEnabledStreams = 0;
    cachedAttributeMask = 0;
    
    bufferBindingId[0] = 0;
    bufferBindingId[1] = 0;

	//enabledAttribCount = 0;
	for(int32 i = 0; i < Texture::TEXTURE_TYPE_COUNT; ++i)
	{
		lastBindedTexture[i] = 0;
	}
    lastBindedTextureType = Texture::TEXTURE_2D;

	lastBindedFBO = 0;
}

void RenderManager::Invalidate()
{

}
#endif


};

#endif // __DAVAENGINE_OPENGL__
