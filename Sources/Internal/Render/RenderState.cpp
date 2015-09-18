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


#include "Render/RenderState.h"
#include "Render/RenderManager.h"
#include "Render/Shader.h"
#include "Concurrency/Thread.h"
#include "Utils/Utils.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "FileSystem/YamlEmitter.h"

namespace DAVA
{
static const String RENDER_STATES_NAMES[] = 
{
	"STATE_BLEND",
	"STATE_DEPTH_TEST",
	"STATE_DEPTH_WRITE",
	"STATE_STENCIL_TEST",
	"STATE_CULL",

	"STATE_SCISSOR_TEST",
    
	"STATE_COLORMASK_RED",
	"STATE_COLORMASK_GREEN",
	"STATE_COLORMASK_BLUE",
	"STATE_COLORMASK_ALPHA"
};

#if defined(__DAVAENGINE_DIRECTX9__)
IDirect3DDevice9 * RenderState::direct3DDevice = 0; 
#endif

UniqueHandle RenderState::RENDERSTATE_2D_BLEND = InvalidUniqueHandle;
UniqueHandle RenderState::RENDERSTATE_2D_OPAQUE = InvalidUniqueHandle;
UniqueHandle RenderState::RENDERSTATE_3D_BLEND = InvalidUniqueHandle;
UniqueHandle RenderState::RENDERSTATE_3D_OPAQUE = InvalidUniqueHandle;
UniqueHandle RenderState::RENDERSTATE_DEFAULT = InvalidUniqueHandle;
    
UniqueHandle RenderState::TEXTURESTATE_EMPTY = InvalidUniqueHandle;


RenderState::RenderState()
{
    renderer = RenderManager::Instance()->GetRenderer();

	Reset(false);
	
	stateHandle = InvalidUniqueHandle;
	textureState = InvalidUniqueHandle;
}

RenderState::~RenderState()
{

}
    
//#define LOG_FINAL_RENDER_STATE
    
/**
    Function to reset state to original zero state.
 */
void RenderState::Reset(bool doHardwareReset)
{
    color.r = 1.0f;
    color.g = 1.0f;
    color.b = 1.0f;
    color.a = 1.0f;

    shader = 0;
    
    if (doHardwareReset)
    {
		RenderManager* rm = RenderManager::Instance();
       // RenderManager::Instance()->LockNonMain();
		
		const RenderStateData& renderStateData =
        rm->GetRenderStateData(RenderState::RENDERSTATE_2D_BLEND);
//        Logger::FrameworkDebug("Do hardware reset");
        // PrintBackTraceToLog();
        
        SetEnableBlendingInHW(renderStateData.state);
        SetBlendModeInHW(renderStateData.sourceFactor, renderStateData.destFactor);
        SetDepthTestInHW(renderStateData.state);
        SetDepthWriteInHW(renderStateData.state);
        SetColorMaskInHW(renderStateData.state);
        
        for (uint32 textureLevel = 0; textureLevel < MAX_TEXTURE_LEVELS; ++textureLevel)
        {
            SetTextureLevelInHW(textureLevel, NULL);
        }
		
		textureState = RenderState::TEXTURESTATE_EMPTY;
		stateHandle = RenderState::RENDERSTATE_DEFAULT;
		
    }
}

bool RenderState::IsEqual(RenderState * anotherState)
{
    if (stateHandle != anotherState->stateHandle) return false;
	if (textureState != anotherState->textureState) return false;
    if (color != anotherState->color) return false;
    
    return true;
}

void RenderState::Flush(RenderState * hardwareState) const
{
	RENDERER_UPDATE_STATS(renderStateSwitches++);
	
	if(hardwareState->stateHandle != stateHandle)
	{
		RENDERER_UPDATE_STATS(renderStateFullSwitches++);
		
		const RenderStateData& currentData = RenderManager::Instance()->GetRenderStateData(stateHandle);
		const RenderStateData& hardwareData = RenderManager::Instance()->GetRenderStateData(hardwareState->stateHandle);
		
		uint32 state = currentData.state;
		uint32 diffState = state ^ hardwareData.state;
		if (diffState != 0)
		{
			if (diffState & RenderStateData::STATE_BLEND)
				SetEnableBlendingInHW(state);
			
			if (diffState & RenderStateData::STATE_DEPTH_TEST)
				SetDepthTestInHW(state);
			
			if (diffState & RenderStateData::STATE_DEPTH_WRITE)
				SetDepthWriteInHW(state);
			
			if (diffState & RenderStateData::STATE_CULL)
				SetCullInHW(state);
			
			if (diffState & RenderStateData::STATE_COLORMASK_ALL)
				SetColorMaskInHW(state);
			
			if (diffState & RenderStateData::STATE_STENCIL_TEST)
				SetStensilTestInHW(state);
			
			if (diffState & RenderStateData::STATE_SCISSOR_TEST)
				SetScissorTestInHW(state);
        }
		
		if (currentData.sourceFactor != hardwareData.sourceFactor ||
			currentData.destFactor != hardwareData.destFactor)
        {
            SetBlendModeInHW(currentData.sourceFactor, currentData.destFactor);
        }
        
        if (currentData.cullMode != hardwareData.cullMode)
        {
            SetCullModeInHW(currentData.cullMode);
        }
        
        if (hardwareData.depthFunc != currentData.depthFunc)
        {
            SetDepthFuncInHW(currentData.depthFunc);
        }
		
		if(hardwareData.fillMode != currentData.fillMode)
		{
			SetFillModeInHW(currentData.fillMode);
		}
		
		if (hardwareData.stencilFunc[0] != currentData.stencilFunc[0] ||
			hardwareData.stencilFunc[1] != currentData.stencilFunc[1] ||
			hardwareData.stencilMask != currentData.stencilMask ||
			hardwareData.stencilRef != currentData.stencilRef)
		{
			SetStencilFuncInHW(currentData.stencilFunc[0],
							   currentData.stencilFunc[1],
							   currentData.stencilRef,
							   currentData.stencilMask);
		}
		
		if (hardwareData.stencilPass[0] != currentData.stencilPass[0] ||
			hardwareData.stencilPass[1] != currentData.stencilPass[1] ||
			hardwareData.stencilFail[0] != currentData.stencilFail[0] ||
			hardwareData.stencilFail[1] != currentData.stencilFail[1] ||
			hardwareData.stencilZFail[0] != currentData.stencilZFail[0] ||
			hardwareData.stencilZFail[1] != currentData.stencilZFail[1])
		{
			SetStencilOpInHW(currentData.stencilFail[0],
							 currentData.stencilZFail[0],
							 currentData.stencilPass[0],
							 currentData.stencilFail[1],
							 currentData.stencilZFail[1],
							 currentData.stencilPass[1]);
		}
				
		hardwareState->stateHandle = stateHandle;
	}

#if defined (LOG_FINAL_RENDER_STATE)
    Logger::FrameworkDebug("RenderState::Flush started");
#endif
	
    if (color != hardwareState->color)
    {
        hardwareState->color = color;
    }

    if(textureState != hardwareState->textureState &&
       textureState != InvalidUniqueHandle)
    {
        if(InvalidUniqueHandle == hardwareState->textureState)
        {
            const TextureStateData& currentTextureData = RenderManager::Instance()->GetTextureState(textureState);
            for (size_t i = 0; i < MAX_TEXTURE_COUNT; ++i)
            {
                SetTextureLevelInHW(static_cast<uint32>(i), currentTextureData.textures[i]);
            }
        }
        else
        {
            const TextureStateData& currentTextureData = RenderManager::Instance()->GetTextureState(textureState);
            const TextureStateData& hardwareTextureData = RenderManager::Instance()->GetTextureState(hardwareState->textureState);
            
            uint32 minIndex = Min(currentTextureData.minmaxTextureIndex & 0x000000FF, hardwareTextureData.minmaxTextureIndex & 0x000000FF);
            uint32 maxIndex = Max((currentTextureData.minmaxTextureIndex & 0x0000FF00), (hardwareTextureData.minmaxTextureIndex & 0x0000FF00)) >> 8;
            for(uint32 i = minIndex; i <= maxIndex; ++i)
            {
                if(currentTextureData.textures[i] != hardwareTextureData.textures[i])
                {
                    SetTextureLevelInHW(i, currentTextureData.textures[i]);
                }
            }
        }
        
        hardwareState->textureState = textureState;
        
        RENDERER_UPDATE_STATS(textureStateFullSwitches++);
    }
    
	
#if defined(__DAVAENGINE_OPENGL__)
        RENDER_VERIFY(glActiveTexture(GL_TEXTURE0));
#endif

    if (shader)
    {
        shader->Bind();
        shader->BindDynamicParameters();
    }
    else Shader::Unbind();
    hardwareState->shader = shader;
    
#if defined (LOG_FINAL_RENDER_STATE)
    Logger::FrameworkDebug("RenderState::Flush finished");
#endif    
}
    
    
#if defined (__DAVAENGINE_OPENGL__)

    
inline void RenderState::SetColorMaskInHW(uint32 state) const
{
    GLboolean redMask = (state & RenderStateData::STATE_COLORMASK_RED) != 0;
    GLboolean greenMask = (state & RenderStateData::STATE_COLORMASK_GREEN) != 0;
    GLboolean blueMask = (state & RenderStateData::STATE_COLORMASK_BLUE) != 0;
    GLboolean alphaMask = (state & RenderStateData::STATE_COLORMASK_ALPHA) != 0;
    
#if defined (LOG_FINAL_RENDER_STATE)
    Logger::FrameworkDebug("RenderState::colormask = %d %d %d %d", redMask, greenMask, blueMask, alphaMask);
#endif    
    RENDER_VERIFY(glColorMask(redMask, 
                              greenMask, 
                              blueMask, 
                              alphaMask));
}

inline void RenderState::SetStensilTestInHW(uint32 state) const
{
	if (state & RenderStateData::STATE_STENCIL_TEST)
	{
#if defined (LOG_FINAL_RENDER_STATE)
        Logger::FrameworkDebug("RenderState::stencil = true");
#endif    
		RENDER_VERIFY(glEnable(GL_STENCIL_TEST));
	}
	else
	{
#if defined (LOG_FINAL_RENDER_STATE)
        Logger::FrameworkDebug("RenderState::stencil = false");
#endif    
		RENDER_VERIFY(glDisable(GL_STENCIL_TEST));
	}
}

inline void RenderState::SetEnableBlendingInHW(uint32 state) const
{
    if (state & RenderStateData::STATE_BLEND)
    {
#if defined (LOG_FINAL_RENDER_STATE)
        Logger::FrameworkDebug("RenderState::blend = true");
#endif    
        RENDER_VERIFY(glEnable(GL_BLEND));
    }
    else 
    {
#if defined (LOG_FINAL_RENDER_STATE)
        Logger::FrameworkDebug("RenderState::blend = false");
#endif    
        RENDER_VERIFY(glDisable(GL_BLEND));
    }
}
    
inline void RenderState::SetCullInHW(uint32 state) const
{
    if (state & RenderStateData::STATE_CULL)
    {
#if defined (LOG_FINAL_RENDER_STATE)
        Logger::FrameworkDebug("RenderState::cullface = true");
#endif    

        RENDER_VERIFY(glEnable(GL_CULL_FACE));
    }
    else 
    {
#if defined (LOG_FINAL_RENDER_STATE)
        Logger::FrameworkDebug("RenderState::cullface = false");
#endif    
        RENDER_VERIFY(glDisable(GL_CULL_FACE));
    }
}

inline void RenderState::SetCullModeInHW(eFace cullMode) const
{
#if defined (LOG_FINAL_RENDER_STATE)
    Logger::FrameworkDebug("RenderState::cull_mode = %d", cullMode);
#endif    

    RENDER_VERIFY(glCullFace(CULL_FACE_MAP[cullMode]));
}


inline void RenderState::SetBlendModeInHW(eBlendMode  sourceFactor,
											eBlendMode  destFactor) const
{
#if defined (LOG_FINAL_RENDER_STATE)
    Logger::FrameworkDebug("RenderState::blend_src_dst = (%d, %d)", sourceFactor, destFactor);
#endif    

    RENDER_VERIFY(glBlendFunc(BLEND_MODE_MAP[sourceFactor], BLEND_MODE_MAP[destFactor]));
}

inline void RenderState::SetTextureLevelInHW(uint32 textureLevel, Texture* texture) const
{
	RENDER_VERIFY(glActiveTexture(GL_TEXTURE0 + textureLevel));
    if(texture)
    {
#if defined (LOG_FINAL_RENDER_STATE)
        Logger::FrameworkDebug("RenderState::bind_texture %d = (%d)", textureLevel, texture->id);
#endif
        RenderManager::Instance()->HWglBindTexture(texture->id, texture->textureType);
    }else
    {
#if defined (LOG_FINAL_RENDER_STATE)
        Logger::FrameworkDebug("RenderState::bind_texture %d = (%d)", textureLevel, 0);
#endif    
        RenderManager::Instance()->HWglBindTexture(0);
    }    
}
inline void RenderState::SetDepthTestInHW(uint32 state) const
{
    if(state & RenderStateData::STATE_DEPTH_TEST)
    {
#if defined (LOG_FINAL_RENDER_STATE)
        Logger::FrameworkDebug("RenderState::depth_test = true");
#endif    
        RENDER_VERIFY(glEnable(GL_DEPTH_TEST));
    }
    else
    {
#if defined (LOG_FINAL_RENDER_STATE)
        Logger::FrameworkDebug("RenderState::depth_test = false");
#endif    
        RENDER_VERIFY(glDisable(GL_DEPTH_TEST));
    }    
}

inline void RenderState::SetDepthWriteInHW(uint32 state) const
{
    if(state & RenderStateData::STATE_DEPTH_WRITE)
    {
#if defined (LOG_FINAL_RENDER_STATE)
        Logger::FrameworkDebug("RenderState::depth_mask = true");
#endif

        RENDER_VERIFY(glDepthMask(GL_TRUE));
    }
    else
    {
#if defined (LOG_FINAL_RENDER_STATE)
        Logger::FrameworkDebug("RenderState::depth_mask = false");
#endif    
        RENDER_VERIFY(glDepthMask(GL_FALSE));
    }
}
    
inline void RenderState::SetDepthFuncInHW(eCmpFunc depthFunc) const
{
#if defined (LOG_FINAL_RENDER_STATE)
    Logger::FrameworkDebug("RenderState::depth func = (%d)", depthFunc);
#endif    

	RENDER_VERIFY(glDepthFunc(COMPARE_FUNCTION_MAP[depthFunc]));
}

inline void RenderState::SetScissorTestInHW(uint32 state) const
{
	if(state & RenderStateData::STATE_SCISSOR_TEST)
	{
#if defined (LOG_FINAL_RENDER_STATE)
        Logger::FrameworkDebug("RenderState::scissor_test = true");
#endif  
		RENDER_VERIFY(glEnable(GL_SCISSOR_TEST));
	}
	else
	{
#if defined (LOG_FINAL_RENDER_STATE)
        Logger::FrameworkDebug("RenderState::scissor_test = false");
#endif  
		RENDER_VERIFY(glDisable(GL_SCISSOR_TEST));
	}
}

inline void RenderState::SetFillModeInHW(eFillMode fillMode) const
{
#if defined(__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WIN32__)
	RENDER_VERIFY(glPolygonMode(GL_FRONT_AND_BACK, FILLMODE_MAP[fillMode]));
#endif
}

inline void RenderState::SetStencilRefInHW() const
{
}

inline void RenderState::SetStencilMaskInHW() const
{
}

inline void RenderState::SetStencilFuncInHW(eCmpFunc stencilFunc0, eCmpFunc stencilFunc1, int32 stencilRef, uint32 stencilMask) const
{
	if(stencilFunc0 == stencilFunc1)
	{
		RENDER_VERIFY(glStencilFunc(COMPARE_FUNCTION_MAP[stencilFunc0], stencilRef, stencilMask));
	}
	else
	{
		RENDER_VERIFY(glStencilFuncSeparate(CULL_FACE_MAP[FACE_FRONT], COMPARE_FUNCTION_MAP[stencilFunc0], stencilRef, stencilMask));
		RENDER_VERIFY(glStencilFuncSeparate(CULL_FACE_MAP[FACE_BACK], COMPARE_FUNCTION_MAP[stencilFunc1], stencilRef, stencilMask));
	}
}

inline void RenderState::SetStencilPassInHW() const
{
	//nothing
}

inline void RenderState::SetStencilFailInHW() const
{
	//nothing
}

inline void RenderState::SetStencilZFailInHW() const
{
	//nothing
}

inline void RenderState::SetStencilOpInHW(eStencilOp stencilFail0,
										  eStencilOp stencilZFail0,
										  eStencilOp stencilPass0,
										  eStencilOp stencilFail1,
										  eStencilOp stencilZFail1,
										  eStencilOp stencilPass1) const
{
	RENDER_VERIFY(glStencilOpSeparate(CULL_FACE_MAP[FACE_FRONT], STENCIL_OP_MAP[stencilFail0], STENCIL_OP_MAP[stencilZFail0], STENCIL_OP_MAP[stencilPass0]));
	RENDER_VERIFY(glStencilOpSeparate(CULL_FACE_MAP[FACE_BACK], STENCIL_OP_MAP[stencilFail1], STENCIL_OP_MAP[stencilZFail1], STENCIL_OP_MAP[stencilPass1]));
}

    
#elif defined(__DAVAENGINE_DIRECTX9__)

inline void RenderState::SetColorMaskInHW()
{
    DWORD flags = 0;
    if (state & STATE_COLORMASK_RED)
        flags |= D3DCOLORWRITEENABLE_RED;
    
    if (state & STATE_COLORMASK_GREEN)
        flags |= D3DCOLORWRITEENABLE_GREEN;
    
    if (state & STATE_COLORMASK_BLUE)
        flags |= D3DCOLORWRITEENABLE_BLUE;
    
    if (state & STATE_COLORMASK_ALPHA)
        flags |= D3DCOLORWRITEENABLE_ALPHA;
    
    RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, flags));
}

inline void RenderState::SetEnableBlendingInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, (state & STATE_BLEND) != 0));
}

inline void RenderState::SetCullInHW()
{
	if (!(state & STATE_CULL))
	{
		RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
	}
}

inline void RenderState::SetCullModeInHW()
{
	if ((state & STATE_CULL))
	{
		RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_CULLMODE, CULL_FACE_MAP[cullMode]));
	}
}


inline void RenderState::SetBlendModeInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_SRCBLEND, BLEND_MODE_MAP[sourceFactor]));
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_DESTBLEND, BLEND_MODE_MAP[destFactor]));
}

inline void RenderState::SetTextureLevelInHW(uint32 textureLevel)
{
	IDirect3DTexture9 * texture = 0;
	if(currentTexture[textureLevel])
		texture = currentTexture[textureLevel]->id;

	if (state & (STATE_TEXTURE0 << textureLevel))
	{
		RENDER_VERIFY(direct3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE ));
		RENDER_VERIFY(direct3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE ));
	}else 
	{
		RENDER_VERIFY(direct3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 ));
		RENDER_VERIFY(direct3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2));
	}
	RENDER_VERIFY(direct3DDevice->SetTexture(textureLevel, texture));
}
inline void RenderState::SetDepthTestInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_ZENABLE, (state & STATE_DEPTH_TEST) != 0));
}

inline void RenderState::SetDepthWriteInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_ZWRITEENABLE , (state & STATE_DEPTH_WRITE) !=0));
}

inline void RenderState::SetAlphaTestInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, (state & STATE_ALPHA_TEST) != 0));
}

inline void RenderState::SetAlphaTestFuncInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_ALPHAFUNC, COMPARE_FUNCTION_MAP[alphaFunc]));
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_ALPHAREF , alphaFuncCmpValue));
}

inline void RenderState::SetDepthFuncInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_ZFUNC, COMPARE_FUNCTION_MAP[alphaFunc]));
}

inline void RenderState::SetScissorTestInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, (state & STATE_SCISSOR_TEST)));
}

inline void RenderState::SetScissorRectInHW()
{
	RECT rect;
	rect.left = (LONG)scissorRect.x;
	rect.right = (LONG)scissorRect.dx;
	rect.top = (LONG)scissorRect.y;
	rect.bottom = (LONG)scissorRect.dy;
	RENDER_VERIFY(direct3DDevice->SetScissorRect(&rect));
}

inline void RenderState::SetStensilTestInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILENABLE, (state & STATE_STENCIL_TEST) != 0));
}

inline void RenderState::SetStencilRefInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILREF, stencilState.ref));
}

inline void RenderState::SetStencilMaskInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILMASK, stencilState.mask));
}

inline void RenderState::SetStencilFuncInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILFUNC, stencilState.func[0]));
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_CCW_STENCILFUNC, stencilState.func[1]));
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILREF, stencilState.ref));
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILMASK, stencilState.mask));
}

inline void RenderState::SetStencilPassInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILPASS, stencilState.pass[0]));
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_CCW_STENCILPASS, stencilState.pass[1]));
}

inline void RenderState::SetStencilFailInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILFAIL, stencilState.fail[0]));
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_CCW_STENCILFAIL, stencilState.fail[1]));
}

inline void RenderState::SetStencilZFailInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILZFAIL, stencilState.zFail[0]));
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_CCW_STENCILZFAIL, stencilState.zFail[1]));
}

inline void RenderState::SetStencilOpInHW()
{
}


#endif 
    
//RenderState::StencilState::StencilState()
//{
//	ref = 0;
//	mask = 0xFFFFFFFF;
//	func[0] = func[1] = CMP_ALWAYS;
//	pass[0] = pass[1] = STENCILOP_KEEP;
//	fail[0] = fail[1] = STENCILOP_KEEP;
//	zFail[0] = zFail[1] = STENCILOP_KEEP;
//}

void RenderState::LoadFromYamlFile(const FilePath & filePath)
{
	YamlParser * parser = YamlParser::Create(filePath);
	if (!parser)
	{
		Logger::Error("Failed to open yaml file: %s", filePath.GetAbsolutePathname().c_str());
		return;
	}

	YamlNode * rootNode = parser->GetRootNode();
	if (!rootNode)
	{
		Logger::Warning("yaml file: %s is empty", filePath.GetAbsolutePathname().c_str());
		return;
	}

	LoadFromYamlNode(rootNode);

	SafeRelease(parser);
}

void RenderState::LoadFromYamlNode(const YamlNode * rootNode)
{
	if (!rootNode)
		return;

	RenderStateData stateData;
	
	const YamlNode * renderStateNode = rootNode->Get("RenderState");
	if(renderStateNode)
	{
		const YamlNode * stateNode = renderStateNode->Get("state");
		if(stateNode)
		{
			Vector<String> states;
			Split(stateNode->AsString(), "| ", states);
			uint32 currentState = 0;
			for(Vector<String>::const_iterator it = states.begin(); it != states.end(); it++)
				currentState |= GetRenderStateByName((*it));
			
			stateData.state = currentState;
		}

		const YamlNode * blendSrcNode = renderStateNode->Get("blendSrc");
		const YamlNode * blendDestNode = renderStateNode->Get("blendDest");
		if(blendSrcNode && blendDestNode)
		{
			eBlendMode newBlendScr = GetBlendModeByName(blendSrcNode->AsString());
			eBlendMode newBlendDest = GetBlendModeByName(blendDestNode->AsString());
			
			stateData.sourceFactor = newBlendScr;
			stateData.destFactor = newBlendDest;
		}

		const YamlNode * cullModeNode = renderStateNode->Get("cullMode");
		if(cullModeNode)
		{
			int32 newCullMode = (int32)GetFaceByName(cullModeNode->AsString());
			stateData.cullMode = (eFace)newCullMode;
		}

		const YamlNode * depthFuncNode = renderStateNode->Get("depthFunc");
		if(depthFuncNode)
		{
			eCmpFunc newDepthFunc = GetCmpFuncByName(depthFuncNode->AsString());
			stateData.depthFunc = newDepthFunc;
		}
		
		const YamlNode * fillModeNode = renderStateNode->Get("fillMode");
		if(fillModeNode)
		{
			eFillMode newFillMode = GetFillModeByName(fillModeNode->AsString());
			stateData.fillMode = newFillMode;
		}

//		const YamlNode * alphaFuncNode = renderStateNode->Get("alphaFunc");
//		const YamlNode * alphaFuncCmpValueNode = renderStateNode->Get("alphaFuncCmpValue");
//		if(alphaFuncNode && alphaFuncCmpValueNode)
//		{
//			eCmpFunc newAlphaFunc = GetCmpFuncByName(alphaFuncNode->AsString());
//			float32 newCmpValue = alphaFuncCmpValueNode->AsFloat();
//		
//			//DO NOTHING FOR NOW
//		}

		const YamlNode * stencilNode = renderStateNode->Get("stencil");
		if(stencilNode)
		{
			const YamlNode * stencilRefNode = stencilNode->Get("ref");
			if(stencilRefNode)
				stateData.stencilRef = stencilRefNode->AsInt32();

			const YamlNode * stencilMaskNode = stencilNode->Get("mask");
			if(stencilMaskNode)
				stateData.stencilMask = stencilMaskNode->AsUInt32();

			const YamlNode * stencilFuncNode = stencilNode->Get("funcFront");
			if(stencilFuncNode)
			{
				stateData.stencilFunc[FACE_FRONT] = GetCmpFuncByName(stencilFuncNode->AsString());
			}
			
			stencilFuncNode = stencilNode->Get("funcBack");
			if(stencilFuncNode)
			{
				stateData.stencilFunc[FACE_BACK] = GetCmpFuncByName(stencilFuncNode->AsString());
			}

			const YamlNode * stencilPassNode = stencilNode->Get("passFront");
			if(stencilPassNode)
			{
				stateData.stencilPass[FACE_FRONT] = GetStencilOpByName(stencilPassNode->AsString());
			}
			
			stencilPassNode = stencilNode->Get("passBack");
			if(stencilPassNode)
			{
				stateData.stencilPass[FACE_BACK] = GetStencilOpByName(stencilPassNode->AsString());
			}

			const YamlNode * stencilFailNode = stencilNode->Get("failFront");
			if(stencilFailNode)
			{
				stateData.stencilFail[FACE_FRONT] = GetStencilOpByName(stencilFailNode->AsString());
			}
			
			stencilFailNode = stencilNode->Get("failBack");
			if(stencilFailNode)
			{
				stateData.stencilFail[FACE_BACK] = GetStencilOpByName(stencilFailNode->AsString());
			}

			const YamlNode * stencilZFailNode = stencilNode->Get("zFailFront");
			if(stencilZFailNode)
			{
				stateData.stencilZFail[FACE_FRONT] = GetStencilOpByName(stencilZFailNode->AsString());
			}
			
			stencilZFailNode = stencilNode->Get("zFailBack");
			if(stencilZFailNode)
			{
				stateData.stencilZFail[FACE_BACK] = GetStencilOpByName(stencilZFailNode->AsString());
			}
		}
		
		stateHandle = RenderManager::Instance()->CreateRenderState(stateData);
	}
}

bool RenderState::SaveToYamlFile(const FilePath & filePath)
{
	YamlNode* resultNode = SaveToYamlNode();
	YamlEmitter::SaveToYamlFile(filePath, resultNode);

	return true;
}
	
YamlNode * RenderState::SaveToYamlNode(YamlNode * parentNode /* = 0 */)
{
	if(!parentNode)
		parentNode = new YamlNode(YamlNode::TYPE_MAP);
	
	static RenderStateData emptyState;
	
	const RenderStateData& renderState = (stateHandle != InvalidUniqueHandle) ? RenderManager::Instance()->GetRenderStateData(stateHandle) : emptyState;

	YamlNode * rootNode = new YamlNode(YamlNode::TYPE_MAP);

	YamlNode * stencilNode = new YamlNode(YamlNode::TYPE_MAP);
	stencilNode->Add("ref", renderState.stencilRef);
	stencilNode->Add("mask", (int32)renderState.stencilMask);
	stencilNode->Add("funcFront", CMP_FUNC_NAMES[(int32)renderState.stencilFunc[FACE_FRONT]]);
	stencilNode->Add("funcBack", CMP_FUNC_NAMES[(int32)renderState.stencilFunc[FACE_BACK]]);
	stencilNode->Add("passFront", STENCIL_OP_NAMES[(int32)renderState.stencilPass[FACE_FRONT]]);
	stencilNode->Add("passBack", STENCIL_OP_NAMES[(int32)renderState.stencilPass[FACE_BACK]]);
	stencilNode->Add("failFront", STENCIL_OP_NAMES[(int32)renderState.stencilFail[FACE_FRONT]]);
	stencilNode->Add("failBack", STENCIL_OP_NAMES[(int32)renderState.stencilFail[FACE_BACK]]);
	stencilNode->Add("zFailFront", STENCIL_OP_NAMES[(int32)renderState.stencilZFail[FACE_FRONT]]);
	stencilNode->Add("zFailBack", STENCIL_OP_NAMES[(int32)renderState.stencilZFail[FACE_BACK]]);
	rootNode->AddNodeToMap("stencil", stencilNode);

	rootNode->Add("blendSrc", BLEND_MODE_NAMES[(int32)renderState.sourceFactor]);
	rootNode->Add("blendDest", BLEND_MODE_NAMES[(int32)renderState.destFactor]);
	rootNode->Add("cullMode", FACE_NAMES[(int32)renderState.cullMode]);
	rootNode->Add("depthFunc", CMP_FUNC_NAMES[(int32)renderState.depthFunc]);
	rootNode->Add("alphaFunc", CMP_FUNC_NAMES[(int32)0]);
	rootNode->Add("alphaFuncCmpValue", 0);
	rootNode->Add("fillMode", FILL_MODE_NAMES[renderState.fillMode]);

	Vector<String> statesStrs;
	GetCurrentStateStrings(renderState.state, statesStrs);
	String stateString;
	for(Vector<String>::const_iterator it = statesStrs.begin(); it != statesStrs.end(); it++)
	{
		stateString += (*it);
		if((it+1) != statesStrs.end())
			stateString += " | ";
	}

	rootNode->Add("state", stateString);

	parentNode->AddNodeToMap("RenderState", rootNode);

	return parentNode;
}

void RenderState::GetCurrentStateStrings(uint32 state, Vector<String> & statesStrs)
{
	statesStrs.clear();

	for(uint32 bit = 0; bit < 32; bit++)
	{
		uint32 tempState = 1 << bit;
		if (state & tempState)
			statesStrs.push_back(RENDER_STATES_NAMES[bit]);
	}
}

uint32 RenderState::GetRenderStateByName(const String & str)
{
	for(uint32 i = 0; i < STATE_COUNT; i++)
	{
		if(RENDER_STATES_NAMES[i] == str)
		{
			uint32 tempState = 1 << i;
            return tempState;
		}
	}
	return 0;
}
	
void RenderState::CopyTo(RenderState* target) const
{
	target->renderer = renderer;
    
    target->color = color;

	target->stateHandle = stateHandle;
	target->textureState = textureState;
	
	/*for(int i = 0; i < MAX_TEXTURE_LEVELS; ++i)
	{
		target->currentTexture[i] = SafeRetain(currentTexture[i]);
	}*/	
}
	
void RenderState::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	static RenderStateData emptyState;
	
	const RenderStateData& renderState = (stateHandle != InvalidUniqueHandle) ? RenderManager::Instance()->GetRenderStateData(stateHandle) : emptyState;
	
	archive->SetUInt32("state", renderState.state);
	Vector4 vecColor(color.r, color.g, color.b, color.a);
	archive->SetVector4("color", vecColor);
	archive->SetInt32("sourceFactor", (int32)renderState.sourceFactor);
	archive->SetInt32("destFactor", (int32)renderState.destFactor);
	archive->SetInt32("cullMode", (int32)renderState.cullMode);
	archive->SetInt32("alphaFunc", (int32)0);
	archive->SetInt32("alphaFuncCmpValue", (int32)0);
	archive->SetInt32("depthFunc", (int32)renderState.depthFunc);
	archive->SetInt32("fillMode", (int32)renderState.fillMode);
	
	archive->SetInt32("stencil_ref", renderState.stencilRef);
	archive->SetUInt32("stencil_mask", renderState.stencilMask);
	uint8 stencilPackedFuncs[8];
	stencilPackedFuncs[0] = renderState.stencilFail[0];
	stencilPackedFuncs[1] = renderState.stencilFail[1];
	stencilPackedFuncs[2] = renderState.stencilFunc[0];
	stencilPackedFuncs[3] = renderState.stencilFunc[1];
	stencilPackedFuncs[4] = renderState.stencilPass[0];
	stencilPackedFuncs[5] = renderState.stencilPass[1];
	stencilPackedFuncs[6] = renderState.stencilZFail[0];
	stencilPackedFuncs[7] = renderState.stencilZFail[1];
	archive->SetByteArray("stencil_funcs", stencilPackedFuncs, 8);
}

void RenderState::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	RenderStateData renderState;
	
	renderState.state = archive->GetUInt32("state");
	Vector4 vecColor = archive->GetVector4("color");
	color = Color(vecColor.x, vecColor.y, vecColor.z, vecColor.w);
	renderState.sourceFactor = (eBlendMode)archive->GetInt32("sourceFactor");
	renderState.destFactor = (eBlendMode)archive->GetInt32("destFactor");
	renderState.cullMode = (eFace)archive->GetInt32("cullMode");
	//alphaFunc = (eCmpFunc)archive->GetInt32("alphaFunc");
	//alphaFuncCmpValue = (uint8)archive->GetInt32("alphaFuncCmpValue");
	renderState.depthFunc = (eCmpFunc)archive->GetInt32("depthFunc");
	renderState.fillMode = (eFillMode)archive->GetInt32("fillMode");
	
	renderState.stencilRef = archive->GetInt32("stencil_ref");
	renderState.stencilMask = archive->GetUInt32("stencil_mask");
	const uint8* stencilPackedFuncs = archive->GetByteArray("stencil_funcs");
	
	renderState.stencilFail[0] = (eStencilOp)stencilPackedFuncs[0];
	renderState.stencilFail[1] = (eStencilOp)stencilPackedFuncs[1];
	renderState.stencilFunc[0] = (eCmpFunc)stencilPackedFuncs[2];
	renderState.stencilFunc[1] = (eCmpFunc)stencilPackedFuncs[3];
	renderState.stencilPass[0] = (eStencilOp)stencilPackedFuncs[4];
	renderState.stencilPass[1] = (eStencilOp)stencilPackedFuncs[5];
	renderState.stencilZFail[0] = (eStencilOp)stencilPackedFuncs[6];
	renderState.stencilZFail[1] = (eStencilOp)stencilPackedFuncs[7];
}

void RenderState::InitDefaultStates()
{
    RenderStateData defaultStateData;
	
	defaultStateData.state = RenderState::DEFAULT_2D_STATE_BLEND;
	defaultStateData.cullMode = FACE_BACK;
	defaultStateData.depthFunc = CMP_NEVER;
	defaultStateData.sourceFactor = BLEND_SRC_ALPHA;
	defaultStateData.destFactor = BLEND_ONE_MINUS_SRC_ALPHA;
	defaultStateData.fillMode = FILLMODE_SOLID;
	
	RenderState::RENDERSTATE_2D_BLEND = RenderManager::Instance()->CreateRenderState(defaultStateData);

    defaultStateData.state = RenderState::DEFAULT_2D_STATE;
    defaultStateData.cullMode = FACE_BACK;
	defaultStateData.depthFunc = CMP_NEVER;
	defaultStateData.sourceFactor = BLEND_SRC_ALPHA;
	defaultStateData.destFactor = BLEND_ONE_MINUS_SRC_ALPHA;
	defaultStateData.fillMode = FILLMODE_SOLID;

    RenderState::RENDERSTATE_2D_OPAQUE = RenderManager::Instance()->CreateRenderState(defaultStateData);

    defaultStateData.state = RenderState::DEFAULT_3D_STATE;
	defaultStateData.cullMode = FACE_BACK;
	defaultStateData.depthFunc = CMP_LESS;
	defaultStateData.sourceFactor = BLEND_SRC_ALPHA;
	defaultStateData.destFactor = BLEND_ONE_MINUS_SRC_ALPHA;
	defaultStateData.fillMode = FILLMODE_SOLID;

    RenderState::RENDERSTATE_3D_OPAQUE = RenderManager::Instance()->CreateRenderState(defaultStateData);
    
	defaultStateData.state = RenderState::DEFAULT_3D_STATE_BLEND;
	defaultStateData.cullMode = FACE_BACK;
	defaultStateData.depthFunc = CMP_LESS;
	defaultStateData.sourceFactor = BLEND_SRC_ALPHA;
	defaultStateData.destFactor = BLEND_ONE_MINUS_SRC_ALPHA;
	defaultStateData.fillMode = FILLMODE_SOLID;
    
	RenderState::RENDERSTATE_3D_BLEND = RenderManager::Instance()->CreateRenderState(defaultStateData);
	
	defaultStateData.state = RenderStateData::STATE_COLORMASK_ALL;
	defaultStateData.cullMode = FACE_COUNT;
	defaultStateData.depthFunc = CMP_TEST_MODE_COUNT;
	defaultStateData.sourceFactor = BLEND_MODE_COUNT;
	defaultStateData.destFactor = BLEND_MODE_COUNT;
	defaultStateData.fillMode = FILLMODE_COUNT;
	defaultStateData.stencilRef = 0;
	defaultStateData.stencilMask = 0;
	defaultStateData.stencilFunc[0] = defaultStateData.stencilFunc[1] = CMP_TEST_MODE_COUNT;
	defaultStateData.stencilPass[0] = defaultStateData.stencilPass[1] = STENCILOP_COUNT;
	defaultStateData.stencilFail[0] = defaultStateData.stencilFail[1] = STENCILOP_COUNT;
	defaultStateData.stencilZFail[0] = defaultStateData.stencilZFail[1] = STENCILOP_COUNT;
	RenderState::RENDERSTATE_DEFAULT = RenderManager::Instance()->CreateRenderState(defaultStateData);
    
    TextureStateData textureData;
	RenderState::TEXTURESTATE_EMPTY = RenderManager::Instance()->CreateTextureState(textureData);
}

};
