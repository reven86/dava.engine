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
#include "Render/2D/Sprite.h"
#include "Utils/Utils.h"
#include "Core/Core.h"
#include "Render/Shader.h"
#include "Render/RenderDataObject.h"
#include "Render/ShaderCache.h"
#include "Render/GPUFamilyDescriptor.h"
#include "Render/PixelFormatDescriptor.h"

namespace DAVA
{
    
Shader * RenderManager::FLAT_COLOR = 0;
Shader * RenderManager::TEXTURE_MUL_FLAT_COLOR = 0;
Shader * RenderManager::TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST = 0;
Shader * RenderManager::TEXTURE_MUL_FLAT_COLOR_IMAGE_A8 = NULL;

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
    needGLScreenShot(false),
    screenShotCallback(NULL)
{
    // Create shader cache singleton
    ShaderCache * cache = new ShaderCache();
    cache = 0;
    
//	Logger::FrameworkDebug("[RenderManager] created");

    GPUFamilyDescriptor::SetupGPUParameters();
    
	renderOrientation = 0;
	currentRenderTarget = 0;
	
	currentRenderEffect = 0;

	frameBufferWidth = 0;
	frameBufferHeight = 0;
	retScreenWidth = 0;
	retScreenHeight = 0;

	fps = 60;

	debugEnabled = false;
	fboViewRenderbuffer = 0;
	fboViewFramebuffer = 0;
	
	userDrawOffset = Vector2(0, 0);
	userDrawScale = Vector2(1, 1);

	viewMappingDrawOffset = Vector2(0, 0);
	viewMappingDrawScale = Vector2(1, 1);

	currentDrawOffset = Vector2(0, 0);
	currentDrawScale = Vector2(1, 1);
    mappingMatrixChanged = true;
	
	isInsideDraw = false;

#if defined(__DAVAENGINE_DIRECTX9__)
	depthStencilSurface = 0;
	backBufferSurface = 0;
#endif
    
    
#if defined (__DAVAENGINE_OPENGL__)
    bufferBindingId[0] = 0;
    bufferBindingId[1] = 0;
    
	for(uint32 i  = 0; i < Texture::TEXTURE_TYPE_COUNT; ++i)
	{
		lastBindedTexture[i] = 0;
	}
	lastBindedTextureType = Texture::TEXTURE_2D;
	
    lastBindedFBO = 0;
	
#endif //#if defined (__DAVAENGINE_OPENGL__)
    
	cursor = 0;
    currentRenderData = 0;
    cachedEnabledStreams = 0;
    
    statsFrameCountToShowDebug = 0;
    frameToShowDebugStats = -1;
    
    FLAT_COLOR = 0;
    TEXTURE_MUL_FLAT_COLOR = 0;
    TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST = 0;
    TEXTURE_MUL_FLAT_COLOR_IMAGE_A8 = 0;
	
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
    SafeRelease(FLAT_COLOR);
    SafeRelease(TEXTURE_MUL_FLAT_COLOR);
    SafeRelease(TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST);
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

FastName RenderManager::FLAT_COLOR_SHADER("~res:/Shaders/renderer2dColor");
FastName RenderManager::TEXTURE_MUL_FLAT_COLOR_SHADER("~res:/Shaders/renderer2dTexture");


void RenderManager::Init(int32 _frameBufferWidth, int32 _frameBufferHeight)
{
    DetectRenderingCapabilities();
    
    
    if (!FLAT_COLOR)
    {
        FLAT_COLOR = SafeRetain(ShaderCache::Instance()->Get(FLAT_COLOR_SHADER, FastNameSet()));
    }
    
    if (!TEXTURE_MUL_FLAT_COLOR)
    {
        TEXTURE_MUL_FLAT_COLOR = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_MUL_FLAT_COLOR_SHADER, FastNameSet()));

    }
    if (!TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST)
    {
        FastNameSet set;
        set.Insert(FastName("ALPHA_TEST_ENABLED"));
        TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_MUL_FLAT_COLOR_SHADER, set));
    }
    
    if(!TEXTURE_MUL_FLAT_COLOR_IMAGE_A8)
    {
        FastNameSet set;
        set.Insert(FastName("IMAGE_A8"));
        TEXTURE_MUL_FLAT_COLOR_IMAGE_A8 = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_MUL_FLAT_COLOR_SHADER, set));
    }

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

	currentRenderTarget = NULL;

	currentClip.x = 0;
	currentClip.y = 0;
	currentClip.dx = -1;
	currentClip.dy = -1;
	
//	for (uint32 idx = 0; idx < MAX_TEXTURE_LEVELS; ++idx)
//        currentTexture[idx] = 0;

	userDrawOffset = Vector2(0, 0);
	userDrawScale = Vector2(1, 1);
	
	currentDrawOffset = Vector2(0, 0);
	currentDrawScale = Vector2(1, 1);
    mappingMatrixChanged = true;
	//currentState.Reset(false);
//	glLoadIdentity();
}

int32 RenderManager::GetRenderOrientation()
{
	return renderOrientation;
}
	
int32 RenderManager::GetScreenWidth()
{
	return retScreenWidth;	
}
int32 RenderManager::GetScreenHeight()
{
	return retScreenHeight;
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
	
/*void RenderManager::SetTexture(Texture *texture, uint32 textureLevel)
{	
    currentState.SetTexture(texture, textureLevel);
}
	
Texture *RenderManager::GetTexture(uint32 textureLevel)
{
    DVASSERT(textureLevel < RenderState::MAX_TEXTURE_LEVELS);
	return currentState.currentTexture[textureLevel];	
}*/
    
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
		
void RenderManager::SetClip(const Rect &rect)
{
	SetHWClip(rect);
}
	
void RenderManager::RemoveClip()
{
	SetHWClip(Rect(0,0,-1,-1));
}

void RenderManager::ClipRect(const Rect &rect)
{
	Rect r = currentClip;
	if(r.dx < 0)
	{
		r.dx = (float32)retScreenWidth * Core::GetPhysicalToVirtualFactor();
	}
	if(r.dy < 0)
	{
		r.dy = (float32)retScreenHeight * Core::GetPhysicalToVirtualFactor();
	}
	
	r = r.Intersection(rect);
	SetHWClip(r);
}

void RenderManager::ClipPush()
{
	clipStack.push(currentClip);
}

void RenderManager::ClipPop()
{
	if(clipStack.empty())
	{
		Rect r(0, 0, -1, -1);
		SetClip(r);
	}
	else
	{
		Rect r = clipStack.top();
		SetClip(r);
	}
	clipStack.pop();
}
	
void RenderManager::InitFBO(GLuint _viewRenderbuffer, GLuint _viewFramebuffer)
{
	fboViewRenderbuffer = _viewRenderbuffer;
	fboViewFramebuffer = _viewFramebuffer;
}

void RenderManager::SetRenderTarget(Sprite *renderTarget)
{
//	Logger::Info("Set Render target");
	RenderTarget rt;
	rt.spr = currentRenderTarget;
	rt.orientation = renderOrientation;
	renderTargetStack.push(rt);
		
	ClipPush();
	PushDrawMatrix();
	PushMappingMatrix();
	IdentityDrawMatrix();
	SetHWRenderTargetSprite(renderTarget);
}

void RenderManager::SetRenderTarget(Texture * renderTarget)
{
	SetHWRenderTargetTexture(renderTarget);
}

void RenderManager::RestoreRenderTarget()
{
//	Logger::Info("Restore Render target");
	RenderTarget rt = renderTargetStack.top();
	renderTargetStack.pop();
	SetHWRenderTargetSprite(rt.spr);

	PopDrawMatrix();
	PopMappingMatrix();
	ClipPop();
}

bool RenderManager::IsRenderTarget()
{
	return currentRenderTarget != NULL;
}
    
/*
bool RenderManager::IsDepthTestEnabled()
{
    return (hardwareState.state & RenderStateBlock::STATE_DEPTH_TEST) != 0;
}

bool RenderManager::IsDepthWriteEnabled()
{
    return (depthWriteEnabled & RenderStateBlock::STATE_DEPTH_WRITE) != 0;
}
*/

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
	
	
void RenderManager::SetDrawTranslate(const Vector2 &offset)
{
    mappingMatrixChanged = true;
	userDrawOffset.x += offset.x * userDrawScale.x;
	userDrawOffset.y += offset.y * userDrawScale.y;
}

void RenderManager::SetDrawTranslate(const Vector3 &offset)
{
    mappingMatrixChanged = true;
    userDrawOffset.x += offset.x * userDrawScale.x;
    userDrawOffset.y += offset.y * userDrawScale.y;
}

const Vector2& RenderManager::GetDrawTranslate() const
{
    return userDrawOffset;
}

void RenderManager::SetDrawScale(const Vector2 &scale)
{
    mappingMatrixChanged = true;
	userDrawScale.x *= scale.x;
	userDrawScale.y *= scale.y;
}

const Vector2& RenderManager::GetDrawScale() const
{
    return userDrawScale;
}
	
void RenderManager::IdentityDrawMatrix()
{
    mappingMatrixChanged = true;
	userDrawScale.x = 1.0f;
	userDrawScale.y = 1.0f;

	userDrawOffset.x = 0.0f;
	userDrawOffset.y = 0.0f;
}

void RenderManager::IdentityMappingMatrix()
{
    mappingMatrixChanged = true;
	viewMappingDrawOffset = Vector2(0.0f, 0.0f);
	viewMappingDrawScale = Vector2(1.0f, 1.0f);
}
	
void RenderManager::IdentityModelMatrix()
{
    mappingMatrixChanged = true;
    currentDrawOffset = Vector2(0.0f, 0.0f);
    currentDrawScale = Vector2(1.0f, 1.0f);

    renderer2d.viewMatrix = Matrix4::IDENTITY;
}
    
	
	
void RenderManager::SetPhysicalViewScale()
{
    mappingMatrixChanged = true;
	viewMappingDrawScale.x = 1.0f;
	viewMappingDrawScale.y = 1.0f;
}

void RenderManager::SetPhysicalViewOffset()
{
    mappingMatrixChanged = true;
	viewMappingDrawOffset = Core::Instance()->GetPhysicalDrawOffset();
}

void RenderManager::SetVirtualViewScale()
{
    mappingMatrixChanged = true;
	viewMappingDrawScale.x = Core::GetVirtualToPhysicalFactor();
	viewMappingDrawScale.y = Core::GetVirtualToPhysicalFactor();
}

void RenderManager::SetVirtualViewOffset()
{
    mappingMatrixChanged = true;
	viewMappingDrawOffset.x -= Core::Instance()->GetVirtualScreenXMin() * viewMappingDrawScale.x;
	viewMappingDrawOffset.y -= Core::Instance()->GetVirtualScreenYMin() * viewMappingDrawScale.y;
}
	
void RenderManager::PushDrawMatrix()
{
	DrawMatrix dm;
	dm.userDrawOffset = userDrawOffset;
	dm.userDrawScale = userDrawScale;
	matrixStack.push(dm);
}

void RenderManager::PopDrawMatrix()
{
	IdentityDrawMatrix();
	DrawMatrix dm = matrixStack.top();
	matrixStack.pop();
	userDrawOffset = dm.userDrawOffset;
	userDrawScale = dm.userDrawScale;
	PrepareRealMatrix();
}
	
void RenderManager::PushMappingMatrix()
{
	DrawMatrix dm;
	dm.userDrawOffset = viewMappingDrawOffset;
	dm.userDrawScale = viewMappingDrawScale;
	mappingMatrixStack.push(dm);
}

void RenderManager::PopMappingMatrix()
{
	IdentityMappingMatrix();
	DrawMatrix dm = mappingMatrixStack.top();
	mappingMatrixStack.pop();
	viewMappingDrawOffset = dm.userDrawOffset;
	viewMappingDrawScale = dm.userDrawScale;
    DVASSERT(mappingMatrixChanged == true);
	PrepareRealMatrix();
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
    
void RenderManager::RectFromRenderOrientationToViewport(Rect & rect)
{

}

//const Matrix4 & RenderManager::GetMatrix(eMatrixType type)
//{
//    return matrices[type];
//}

//const Matrix3 & RenderManager::GetNormalMatrix()
//{
//    if (uniformMatrixFlags[UNIFORM_MATRIX_NORMAL] == 0)
//    {
//        //GetUniformMatrix(UNIFORM_MATRIX_MODELVIEWPROJECTION);
//        const Matrix4 & modelViewMatrix = GetMatrix(MATRIX_MODELVIEW);
//        
//        modelViewMatrix.GetInverse(uniformMatrices[UNIFORM_MATRIX_NORMAL]);
//        uniformMatrices[UNIFORM_MATRIX_NORMAL].Transpose();
//        uniformMatrixNormal = uniformMatrices[UNIFORM_MATRIX_NORMAL];
//        uniformMatrixFlags[UNIFORM_MATRIX_NORMAL] = 1; // matrix is ready
//    }
//    return uniformMatrixNormal;
//}

//const Matrix4 & RenderManager::GetUniformMatrix(eUniformMatrixType type)
//{
//    if (uniformMatrixFlags[type] == 0)
//    {
//        if (type == UNIFORM_MATRIX_MODELVIEWPROJECTION)
//        {
//            uniformMatrices[type] =  matrices[MATRIX_MODELVIEW] * matrices[MATRIX_PROJECTION];
//        }
//        uniformMatrixFlags[type] = 1; // matrix is ready
//    }
//    return uniformMatrices[type];
//}
//    
//void RenderManager::ClearUniformMatrices()
//{
//    for (int32 k = 0; k < UNIFORM_MATRIX_COUNT; ++k)
//        uniformMatrixFlags[k] = 0;
//}
    
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
    for (int32 k = 0; k < PRIMITIVETYPE_COUNT; ++k)
        primitiveCount[k] = 0;
    dynamicParamUniformBindCount = 0;
    materialParamUniformBindCount = 0;
    spriteDrawCount = 0;
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
	
#if defined(__DAVAENGINE_OPENGL__) && (defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__))
	
#if defined(__DAVAENGINE_WIN32__)
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
    
    
void RenderManager::Setup2DMatrices()
{
    Matrix4 glTranslate, glScale;
    glTranslate.glTranslate(currentDrawOffset.x, currentDrawOffset.y, 0.0f);
    glScale.glScale(currentDrawScale.x, currentDrawScale.y, 1.0f);
    renderer2d.viewMatrix = glScale * glTranslate;
    
    RenderManager::SetDynamicParam(PARAM_WORLD, &Matrix4::IDENTITY, UPDATE_SEMANTIC_ALWAYS);
    RenderManager::SetDynamicParam(PARAM_VIEW, &renderer2d.viewMatrix, UPDATE_SEMANTIC_ALWAYS);
    RenderManager::SetDynamicParam(PARAM_PROJ, &renderer2d.projMatrix, UPDATE_SEMANTIC_ALWAYS);
}
    

	
};
