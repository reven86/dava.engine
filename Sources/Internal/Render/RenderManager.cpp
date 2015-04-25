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


#include "Render/RenderManager.h"
#include "Render/Texture.h"
#include "Utils/Utils.h"
#include "Core/Core.h"
#include "Render/Shader.h"
#include "Render/RenderDataObject.h"
#include "Render/ShaderCache.h"
#include "Render/GPUFamilyDescriptor.h"
#include "Render/PixelFormatDescriptor.h"

namespace DAVA
{
AutobindVariableData RenderManager::dynamicParameters[DYNAMIC_PARAMETERS_COUNT];
uint32  RenderManager::dynamicParamersRequireUpdate;
Matrix4 RenderManager::worldViewMatrix;
Matrix4 RenderManager::viewProjMatrix;
Matrix4 RenderManager::worldViewProjMatrix;
Matrix4 RenderManager::invWorldViewMatrix;
Matrix3 RenderManager::normalMatrix;
Matrix4 RenderManager::invWorldMatrix;
Matrix3 RenderManager::worldInvTransposeMatrix;

    
RenderManager::RenderManager(Core::eRenderer _renderer)
:   renderer(_renderer),
    currentState(),
    hardwareState(),
    currentRenderTarget(0),
    needGLScreenShot(false),
    screenShotCallback(nullptr)
{
    // Create shader cache singleton
    ShaderCache * cache = new ShaderCache();
    cache = 0;
    
//	Logger::FrameworkDebug("[RenderManager] created");

    GPUFamilyDescriptor::SetupGPUParameters();
	
	currentRenderEffect = 0;

	frameBufferWidth = 0;
	frameBufferHeight = 0;

	fps = 60;

	debugEnabled = false;
	fboViewRenderbuffer = 0;
	fboViewFramebuffer = 0;
	
	isInsideDraw = false;

#if defined(__DAVAENGINE_DIRECTX9__)
	depthStencilSurface = 0;
	backBufferSurface = 0;
#endif
    
    
#if defined (__DAVAENGINE_OPENGL__)
    bufferBindingId[0] = 0;
    bufferBindingId[1] = 0;
	
    lastBindedFBO = 0;
	
#endif //#if defined (__DAVAENGINE_OPENGL__)
    
	cursor = 0;
    currentRenderData = 0;
    cachedEnabledStreams = 0;
    cachedAttributeMask = 0;
    attachedRenderData = 0;
    
    statsFrameCountToShowDebug = 0;
    frameToShowDebugStats = -1;
	
	renderContextId = 0;
    
	InitDefaultRenderStates();
    
    for (uint32 paramIndex = 0; paramIndex < DYNAMIC_PARAMETERS_COUNT; ++paramIndex)
    {
        dynamicParameters[paramIndex].value = 0;
        dynamicParameters[paramIndex].updateSemantic = 0;
    }
    dynamicParamersRequireUpdate = 0;
    
    SetDynamicParam(PARAM_WORLD, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
    SetDynamicParam(PARAM_VIEW, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
    SetDynamicParam(PARAM_PROJ, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
}
	
RenderManager::~RenderManager()
{
    ShaderCache::Instance()->Release();
    
    currentRenderData = 0;
	SafeRelease(cursor);
	Logger::FrameworkDebug("[RenderManager] released");
}
	
void RenderManager::InitDefaultRenderStates()
{
    RenderState::InitDefaultStates();
    
	hardwareState.stateHandle = RenderState::RENDERSTATE_DEFAULT;
}

void RenderManager::SetDebug(bool isDebugEnabled)
{
	debugEnabled = isDebugEnabled;
}

void RenderManager::RequestGLScreenShot(ScreenShotCallbackDelegate *callback)
{ 
	screenShotCallback = callback;
	needGLScreenShot = true; 
}
	
bool RenderManager::IsInsideDraw()
{
	return isInsideDraw;
}
    
#ifdef __DAVAENGINE_ANDROID__    
void RenderManager::InitFBSize(int32 _frameBufferWidth, int32 _frameBufferHeight)
{
    frameBufferWidth = _frameBufferWidth;
	frameBufferHeight = _frameBufferHeight;

    hardwareState.Reset(false);
	currentState.Reset(true);
    
	Logger::FrameworkDebug("[RenderManager::InitFBSize] size: %d x %d", frameBufferWidth, frameBufferHeight);
}
#endif //    #ifdef __DAVAENGINE_ANDROID__    

void RenderManager::Init(int32 _frameBufferWidth, int32 _frameBufferHeight)
{
    DetectRenderingCapabilities();

#if defined(__DAVAENGINE_DIRECTX9__)
	currentState.direct3DDevice = GetD3DDevice();
#endif
    
	currentState.Reset(false);
    hardwareState.Reset(true);

#if defined(__DAVAENGINE_OPENGL__)
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)//Dizz: glDisableClientState functions are not supported by GL ES 2.0
    RENDER_VERIFY(glDisableClientState(GL_VERTEX_ARRAY));
    RENDER_VERIFY(glDisableClientState(GL_NORMAL_ARRAY));
    RENDER_VERIFY(glDisableClientState(GL_COLOR_ARRAY));
	for (int k = 0; k < RenderState::MAX_TEXTURE_LEVELS; ++k)
    {
        RENDER_VERIFY(glClientActiveTexture(GL_TEXTURE0 + k));
        RENDER_VERIFY(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
    }
    RENDER_VERIFY(glClientActiveTexture(GL_TEXTURE0));
#endif
#endif
    
	frameBufferWidth = _frameBufferWidth;
	frameBufferHeight = _frameBufferHeight;
#if defined (__DAVAENGINE_OPENGL__)
//	Logger::FrameworkDebug("[RenderManager::Init] orientation: %d x %d", frameBufferWidth, frameBufferHeight);
#else
//	Logger::FrameworkDebug("[RenderManager::Init] orientation: %d x %d ", frameBufferWidth, frameBufferHeight);
#endif
}

void RenderManager::Reset()
{
	ResetColor();
    
    lastBindedFBO = fboViewFramebuffer;
}

void RenderManager::SetColor(float32 r, float32 g, float32 b, float32 a)
{
    currentState.SetColor(r, g, b, a);
}
	
void RenderManager::SetColor(const Color & _color)
{
    currentState.SetColor(_color);
}
	
float32 RenderManager::GetColorR() const
{
	return currentState.color.r;
}
	
float32 RenderManager::GetColorG() const
{
	return currentState.color.g;
}
	
float32 RenderManager::GetColorB() const
{
	return currentState.color.b;
}
	
float32 RenderManager::GetColorA() const
{
	return currentState.color.a;
}
    
const Color & RenderManager::GetColor() const
{
    return currentState.color;
}

void RenderManager::ResetColor()
{
	currentState.ResetColor();
}
    
void RenderManager::SetShader(Shader * _shader)
{
//    SafeRelease(shader);
//    shader = SafeRetain(_shader);
    currentState.SetShader(_shader);
}

Shader * RenderManager::GetShader()
{
    return currentState.shader;
}


void RenderManager::SetRenderData(RenderDataObject * object)
{
    currentRenderData = object;
}

void RenderManager::InitFBO(GLuint _viewRenderbuffer, GLuint _viewFramebuffer)
{
	fboViewRenderbuffer = _viewRenderbuffer;
	fboViewFramebuffer = _viewFramebuffer;
}

Size2i RenderManager::GetFramebufferSize()
{
    return Size2i(frameBufferWidth, frameBufferHeight);
}

void RenderManager::SetRenderEffect(Shader * renderEffect)
{
    currentRenderEffect = renderEffect;
}

void RenderManager::DrawElements(ePrimitiveType type, int32 count, eIndexFormat indexFormat, void * indices)
{
    RenderManager::Instance()->SetShader(currentRenderEffect);
    RenderManager::Instance()->FlushState();
	RenderManager::Instance()->AttachRenderData();
    RenderManager::Instance()->HWDrawElements(type, count, indexFormat, indices);
}

void RenderManager::DrawArrays(ePrimitiveType type, int32 first, int32 count)
{
    RenderManager::Instance()->SetShader(currentRenderEffect);
    RenderManager::Instance()->FlushState();
	RenderManager::Instance()->AttachRenderData();
    RenderManager::Instance()->HWDrawArrays(type, first, count);
}

void RenderManager::Lock()
{
	glMutex.Lock();
}
void RenderManager::Unlock()
{
	glMutex.Unlock();
}

void RenderManager::LockRenderState()
{
    renderStateMutex.Lock();
}

void RenderManager::UnlockRenderState()
{
    renderStateMutex.Unlock();
}
    
void RenderManager::LockTextureState()
{
    textureStateMutex.Lock();
}

void RenderManager::UnlockTexturerState()
{
    textureStateMutex.Unlock();
}

void RenderManager::SetFPS(int32 newFps)
{
	fps = newFps;	
}
int32 RenderManager::GetFPS()
{
	return fps;
}

void RenderManager::SetCursor(Cursor * _cursor)
{
#if defined(__DAVAENGINE_MACOS__) || (defined(__DAVAENGINE_WIN32__) && defined(__DAVAENGINE_DIRECTX9__))
	SafeRelease(cursor);
	cursor = SafeRetain(_cursor);
	if (cursor)cursor->HardwareSet();
#endif
}

Cursor * RenderManager::GetCursor()
{
	return cursor;
}
	
const RenderManager::Caps & RenderManager::GetCaps()
{
	return caps;
}
    
RenderManager::Stats & RenderManager::GetStats()
{
    return stats;
}
    
void RenderManager::ClearStats()
{
    stats.Clear();
}
    
void RenderManager::Stats::Clear()
{
	//uint32 matrixMultiplicationCount = Matrix4::matrixMultiplicationCounter;
	//Matrix4::matrixMultiplicationCounter = 0;
    drawArraysCalls = 0;
    drawElementsCalls = 0;
    shaderBindCount = 0;
    occludedRenderBatchCount = 0;
	renderStateSwitches = 0;
	renderStateFullSwitches = 0;
	textureStateFullSwitches = 0;
	attachRenderDataCount = 0;
    attachRenderDataSkipCount = 0;
    for (int32 k = 0; k < PRIMITIVETYPE_COUNT; ++k)
        primitiveCount[k] = 0;
    dynamicParamUniformBindCount = 0;
    materialParamUniformBindCount = 0;
    spriteDrawCount = 0;
    
    visibleRenderObjectCount = 0;
    occludedRenderObjectCount = 0;
}

void RenderManager::EnableOutputDebugStatsEveryNFrame(int32 _frameToShowDebugStats)
{
    frameToShowDebugStats = _frameToShowDebugStats;
}

void RenderManager::ProcessStats()
{ 
    if (frameToShowDebugStats == -1)return;
    
    statsFrameCountToShowDebug++;
    if (statsFrameCountToShowDebug >= frameToShowDebugStats)
    {
        statsFrameCountToShowDebug = 0;
        Logger::FrameworkDebug("== Frame stats: DrawArraysCount: %d DrawElementCount: %d ==", stats.drawArraysCalls, stats.drawElementsCalls);
        for (int32 k = 0; k < PRIMITIVETYPE_COUNT; ++k)
            Logger::FrameworkDebug("== Primitive Stats: %d ==", stats.primitiveCount[k]);
        Logger::FrameworkDebug("== SpriteDrawCount: %d  ==", stats.spriteDrawCount);
    }
}
    
    /*void RenderManager::EnableAlphaTest(bool isEnabled)
{
    alphaTestEnabled = isEnabled;
}

 
void RenderManager::EnableCulling(bool isEnabled)
{
    cullingEnabled = isEnabled;
}*/
    

RenderOptions * RenderManager::GetOptions()
{
	return &options;
}
	
uint32 RenderManager::GetFBOViewFramebuffer() const
{
    return fboViewFramebuffer;
}

void RenderManager::SetRenderContextId(uint64 contextId)
{
	renderContextId = contextId;
}
	
uint64 RenderManager::GetRenderContextId()
{
	return renderContextId;
}
	
void RenderManager::VerifyRenderContext()
{
	
#if defined(__DAVAENGINE_OPENGL__) && (defined(__DAVAENGINE_WINDOWS_DESKTOP__) || defined(__DAVAENGINE_MACOS__))
	
#if defined(__DAVAENGINE_WINDOWS_DESKTOP__)
	uint64 curRenderContext = (uint64)wglGetCurrentContext();
#elif defined(__DAVAENGINE_MACOS__)
	uint64 curRenderContext = (uint64)CGLGetCurrentContext();
#endif
	
	//VI: if you see this assert then something wrong happened to
	//opengl context and current context doesn't match the one that was
	//stored as a reference context.
	//It may happen in several cases:
	//1. This check was performed in another thread without prior updating renderContextId
	//2. OpenGL context was invalidated and recreated without updating renderContextId
	//3. Something really bad happened and opengl context was set to thread local storage by external forces
	//In order to deal with cases 1) and 2) just check app logic and update renderContextId when it's appropriate.
	//In order to deal with 3) seek for the solution. For example on MacOSX 10.8 NSOpenPanel runs in another process.
	//And after returning from file selection dialog the opengl context is completely wrong until the end of current event loop.
	//In order to fix call QApplication::processEvents in case of Qt or equivalent in case of native app or
	//postpone result processing via delayed selector execution.
	DVASSERT(curRenderContext == renderContextId);
	
#endif
}
    
};
