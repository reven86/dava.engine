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


#ifndef __DAVAENGINE_RENDERSTATEBLOCK_H__
#define __DAVAENGINE_RENDERSTATEBLOCK_H__

#include "Render/RenderBase.h"
#include "Core/Core.h"
#include "Render/Texture.h"
#include "Base/Serializable.h"

#include "Render/UniqueStateSet.h"
#include "Render/RenderStateData.h"

namespace DAVA
{

class Texture;
class Shader;

class RenderState : public Serializable
{
public:  

	static const uint32 STATE_COUNT = 19;

	static const uint32 IGNORE_SAVE_LOAD_RENDER_STATES = (RenderStateData::STATE_TEXTURE0 |
														  RenderStateData::STATE_TEXTURE1 |
														  RenderStateData::STATE_TEXTURE2 |
														  RenderStateData::STATE_TEXTURE3 |
														  RenderStateData::STATE_TEXTURE4 |
														  RenderStateData::STATE_TEXTURE5 |
														  RenderStateData::STATE_TEXTURE6 |
														  RenderStateData::STATE_TEXTURE7);

    static const uint32 DEFAULT_2D_STATE = (RenderStateData::STATE_TEXTURE0 |
											RenderStateData::STATE_COLORMASK_ALL);
    static const uint32 DEFAULT_2D_STATE_BLEND = (RenderStateData::STATE_BLEND |
												  RenderStateData::STATE_TEXTURE0 |
												  RenderStateData::STATE_COLORMASK_ALL);
    
    static const uint32 DEFAULT_3D_STATE = (RenderStateData::STATE_DEPTH_WRITE |
											RenderStateData::STATE_DEPTH_TEST |
											RenderStateData::STATE_CULL |
											RenderStateData::STATE_TEXTURE0 |
											RenderStateData::STATE_COLORMASK_ALL);
	
    static const uint32 DEFAULT_3D_STATE_BLEND = (RenderStateData::STATE_BLEND |
												  RenderStateData::STATE_DEPTH_WRITE |
												  RenderStateData::STATE_DEPTH_TEST |
												  RenderStateData::STATE_CULL |
												  RenderStateData::STATE_TEXTURE0 |
												  RenderStateData::STATE_COLORMASK_ALL);
    
    static UniqueHandle RENDERSTATE_2D_BLEND;
	static UniqueHandle RENDERSTATE_2D_OPAQUE;
	static UniqueHandle RENDERSTATE_3D_BLEND;
    static UniqueHandle RENDERSTATE_3D_OPAQUE;
	static UniqueHandle RENDERSTATE_DEFAULT;
    
    static UniqueHandle TEXTURESTATE_EMPTY;


    enum
    {
        STATE_CHANGED_COLOR = 1 << 0,       // bit marks that color was changed during set
        STATE_CHANGED_SRC_BLEND = 1 << 1,  
        STATE_CHANGED_DEST_BLEND = 1 << 2, 
        STATE_CHANGED_CULLMODE = 1 << 3,
        STATE_CHANGED_SHADER = 1 << 4,
        STATE_CHANGED_ALPHA_FUNC = 1 << 5,
		STATE_CHANGED_DEPTH_FUNC = 1 << 6,
		STATE_CHANGED_SCISSOR_RECT = 1 << 7,
    
        STATE_CHANGED_TEXTURE0 = 1 << 8,        
        STATE_CHANGED_TEXTURE1 = 1 << 9,        
        STATE_CHANGED_TEXTURE2 = 1 << 10,        
        STATE_CHANGED_TEXTURE3 = 1 << 11,
		STATE_CHANGED_TEXTURE4 = 1 << 12,
		STATE_CHANGED_TEXTURE5 = 1 << 13,
		STATE_CHANGED_TEXTURE6 = 1 << 14,
		STATE_CHANGED_TEXTURE7 = 1 << 15,

		STATE_CHANGED_STENCIL_REF = 1 << 16,
		STATE_CHANGED_STENCIL_MASK = 1 << 17,
		STATE_CHANGED_STENCIL_FUNC = 1 << 18,
		STATE_CHANGED_STENCIL_PASS = 1 << 19,
		STATE_CHANGED_STENCIL_FAIL = 1 << 20,
		STATE_CHANGED_STENCIL_ZFAIL = 1 << 21,
    };
    /*
        algorithm: 
        diffstate = state ^ oldstate;
        if (diffstate != 0)
        {
            go state by state
            // blend 
            // depth test
            // depth write
            // stencil
        }
        if (changevars != 0)
        {
            // go var by var
            
        }
     */
    

    RenderState();
    ~RenderState();
        
    Core::eRenderer renderer;
    
    Color		color;

	UniqueHandle stateHandle;
	UniqueHandle textureState;
    
    static const uint32 MAX_TEXTURE_LEVELS = 8;
    Shader * shader;
    
    // STATE_COLOR
    inline void SetColor(float32 _r, float32 _g, float32 _b, float32 _a);
    inline void SetColor(const Color & _color);
	inline void ResetColor();
    inline const Color & GetColor() const;
    
    // SHADER
    inline void SetShader(Shader * shader);
    
	//FILL
	inline void SetFillMode(eFillMode fillMode);

	inline void SetEnableBlendingInHW(uint32 state) const;
	inline void SetTextureLevelInHW(uint32 textureLevel, Texture* texture) const;
	inline void SetBlendModeInHW(eBlendMode	sourceFactor,
								 eBlendMode  destFactor) const;
	inline void SetDepthTestInHW(uint32 state) const;
	inline void SetDepthWriteInHW(uint32 state) const;
	inline void SetDepthFuncInHW(eCmpFunc depthFunc) const;
	inline void SetCullInHW(uint32 state) const;
	inline void SetCullModeInHW(eFace cullMode) const;
	inline void SetColorInHW() const;
	inline void SetColorMaskInHW(uint32 state) const;
	inline void SetStensilTestInHW(uint32 state) const;

	inline void SetAlphaTestInHW(uint32 state) const;
	inline void SetAlphaTestFuncInHW(eCmpFunc alphaFunc, uint8 alphaFuncCmpValue) const;

	inline void SetStencilRefInHW() const;
	inline void SetStencilMaskInHW() const;
	inline void SetStencilFuncInHW(eCmpFunc stencilFunc0,
								   eCmpFunc stencilFunc1,
								   int32 stencilRef,
								   uint32 stencilMask) const;
	inline void SetStencilPassInHW() const;
	inline void SetStencilFailInHW() const;
	inline void SetStencilZFailInHW() const;
	inline void SetStencilOpInHW(eStencilOp stencilFail0,
								 eStencilOp stencilZFail0,
								 eStencilOp stencilPass0,
								 eStencilOp stencilFail1,
								 eStencilOp stencilZFail1,
								 eStencilOp stencilPass1) const;

	inline void SetScissorTestInHW(uint32 state) const;

	inline void SetFillModeInHW(eFillMode fillMode) const;
    
    /**
     \brief Function to load render state from yaml file.
	 \param[in] filePath path to file
     */
	void LoadFromYamlFile(const FilePath & filePath);

	/**
	 \brief Function to load render state from yaml node.
	 \param[in] rootNode root yaml node
	 */
	void LoadFromYamlNode(const YamlNode * rootNode);

    /**
     \brief Function to save render state to yaml file.
	 \param[in] filePath path to file
     */
	bool SaveToYamlFile(const FilePath & filePath);
	
	/**
	 \brief Function to save render state to yaml node.
	 \param[in] rootNode root yaml node
	 */
	YamlNode * SaveToYamlNode(YamlNode * rootNode = 0);

    /**
        Function to reset state to original zero state.
     */
    void Reset(bool doHardwareReset);
    
    /**
        Function to flush state into graphics hardware
        It checks what was changed from previous flush
        It updates previous state block to current state
     */
    void Flush(RenderState * previousState) const;
    
    /**
        Compare states
     */
    bool IsEqual(RenderState * anotherState);

	//introspection related

	void GetCurrentStateStrings(uint32 state, Vector<String> & statesStrs);

	static uint32 GetRenderStateByName(const String & str);
	
	void CopyTo(RenderState* target) const;
	
	virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
	virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);
    
    static void InitDefaultStates();

private:
	RenderState(const RenderState & renderState);
	RenderState * operator=(const RenderState & renderState);

public:

#if defined(__DAVAENGINE_DIRECTX9__)
	static IDirect3DDevice9 * direct3DDevice; 
#endif
//	INTROSPECTION(RenderState,
//		MEMBER(state, "state", I_VIEW | I_EDIT)
//		PROPERTY("CullMode", "Cull Mode", GetCullMode, SetCullMode, I_VIEW | I_EDIT)
//		MEMBER(stencilState, "Stencil state", I_VIEW | I_EDIT)
//		)
};


// Implementation of inline functions
inline void RenderState::SetColor(float32 _r, float32 _g, float32 _b, float32 _a)
{
    //if ((color.r != _r) || (color.g != _g) || (color.b != _b) || (color.a != _a))
    {
        color.r = _r;
        color.g = _g;
        color.b = _b;
        color.a = _a;
        //changeSet |= STATE_CHANGED_COLOR;
    }
}

inline void RenderState::SetColor(const Color & _color)
{
    //if (color != _color)
    {
        color = _color;
        //changeSet |= STATE_CHANGED_COLOR;
    }
}

inline void RenderState::ResetColor()
{
    //if ((color.r != 1.0f) || (color.g != 1.0f) || (color.b != 1.0f) || (color.a != 1.0f))
    {
        color.r = color.g = color.b = color.a = 1.0f;
        //changeSet |= STATE_CHANGED_COLOR;
    }
}


inline const Color & RenderState::GetColor() const
{
    return color;
}

// SHADER
inline void RenderState::SetShader(Shader * _shader)
{
    shader = _shader;
// Rethink concept of caching shader / shader data
//    if (shader != _shader)
//    {
//        shader = _shader;
//        changeSet |= STATE_CHANGED_SHADER;
//    }
}
    
};

#endif // __DAVAENGINE_RENDERSTATEBLOCK_H__
