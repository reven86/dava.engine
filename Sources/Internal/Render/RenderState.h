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

/*
class BlendEnable
{
public:
    inline void UpdateVarGL(bool value)
    {
#if defined(__DAVAENGINE_OPENGL__)
        if (value)glEnable(GL_BLEND);
        else glDisable(GL_BLEND);
#elif defined(__DAVAENGINE_DIRECTX9__)
        
#endif
    }
    
};

class RenderStateBlock;
    
class RenderStateVariable
{
public:
    RenderStateVariable(RenderStateBlock * mainBlock);
    virtual ~RenderStateVariable();
    
    
    
};

template<class IMPL_CLASS, class V>
class RenderStateVariableImpl : public RenderStateVariable
{
public:
    RenderStateVariableImpl(RenderStateBlock * mainBlock);
    
    inline void UpdateVarGL() { renderStateImpl.UpdateVarGL(value); };
    inline void Set(const V & _value) { value = _value; };
                    
    V value;
    IMPL_CLASS renderStateImpl;
};


class RenderStateBlock
{
public:
    enum 
    {
        STATE_TYPE_UINT32 = 0,    
        STATE_TYPE_COLOR = 1,
    };
	enum
	{
        STATE_BLEND     = 0,
		STATE_BLENDFUNC = 1,  
        //        STATE_CULLFACE  = 1 << 2,     
        //        STATE_DEPTHTEST = 1 << 3,  

        STATE_ENABLE_TEXTURING = 2,    // Fixed func only
        STATE_COLOR = 3,  
	};
    enum
    {
        STATE_COUNT = 4
    };
    
    void Init(Core::eRenderer renderer);
    void Flush();
    
    
    typedef void (RenderStateBlock::*UpdateVarFunc)(uint32 value, uint32 oldValue);
    typedef void (RenderStateBlock::*UpdateVarFuncColor)(Color & value, Color & oldValue);

    void UpdateBlendGL(uint32 value, uint32 oldValue);
    void UpdateBlendDX(uint32 value, uint32 oldValue);
    
    void UpdateBlendFuncGL(uint32 func, uint32 oldValue);
    void UpdateBlendFuncDX(uint32 func, uint32 oldValue);

    void UpdateEnableTexturingGL(uint32 func, uint32 oldValue);
    void UpdateEnableTexturingDX(uint32 func, uint32 oldValue);
    
    void UpdateEnableColorGL(Color & value, Color & oldValue);
    void UpdateEnableColorDX(Color & value, Color & oldValue);

//    void UpdateCullFaceGL(uint32 value, uint32 oldValue);
//    void UpdateCullFaceDX(uint32 value, uint32 oldValue);

    void RegisterStateUInt32(uint32 state, UpdateVarFunc func);  
    void RegisterStateColor(uint32 state, UpdateVarFuncColor func);  
    
    
    
    inline void SetStateValue(uint32 state, uint32 value);
    inline uint32 GetStateValue(uint32 state); 
    
    
    
    uint32 activeStateSet;
    uint32 currentStateSet;
    
    uint32 stateType[STATE_COUNT];
    uint32 varValue[STATE_COUNT];
    Color varValueColor[STATE_COUNT];
    uint32 currentRendererValue[STATE_COUNT];
    Color currentRendererValueColor[STATE_COUNT];
    UpdateVarFunc varUpdateFuncArray[STATE_COUNT];
    UpdateVarFuncColor varUpdateFuncArrayColor[STATE_COUNT];
    uint32 varCount;
    
    
    bool isDebug;
};
    
    
inline void RenderStateBlock::SetStateValue(uint32 state, uint32 value)
{
    varValue[state] = value;
    currentStateSet |= (1 << state); // flag that value was changed
}

inline uint32 RenderStateBlock::GetStateValue(uint32 state) 
{
    return varValue[state];
}
    
*/
class Texture;
class Shader;

class RenderState : public Serializable
{
public:  
    /*enum
    {
        // bit flags
        STATE_BLEND = 1 << 0,            // 
        STATE_DEPTH_TEST = 1 << 1,       
        STATE_DEPTH_WRITE = 1 << 2,
        STATE_STENCIL_TEST = 1 << 3,     
        STATE_CULL = 1 << 4,

        STATE_ALPHA_TEST = 1 << 5,          // fixed func only / in programmable pipeline can check for consistency
		STATE_SCISSOR_TEST = 1 << 6,

        STATE_TEXTURE0 = 1 << 7,            // fixed func only / in programmable pipeline only checks for consistency
        STATE_TEXTURE1 = 1 << 8,            // fixed func only / in programmable pipeline only checks for consistency
        STATE_TEXTURE2 = 1 << 9,            // fixed func only / in programmable pipeline only checks for consistency
        STATE_TEXTURE3 = 1 << 10,            // fixed func only / in programmable pipeline only checks for consistency
		STATE_TEXTURE4 = 1 << 11,
		STATE_TEXTURE5 = 1 << 12,
		STATE_TEXTURE6 = 1 << 13,
		STATE_TEXTURE7 = 1 << 14,
        
        STATE_COLORMASK_RED =  1 << 15,
        STATE_COLORMASK_GREEN = 1 << 16,
        STATE_COLORMASK_BLUE = 1 << 17,
        STATE_COLORMASK_ALPHA = 1 << 18,
        STATE_COLORMASK_ALL = (STATE_COLORMASK_RED | STATE_COLORMASK_GREEN | STATE_COLORMASK_BLUE | STATE_COLORMASK_ALPHA),
        
        // 4 bits for sourceBlendFactor
        // 4 bits for destBlendFactor
        // 
        // 32 bits * 4 for textures
        // 32 bits * 4 for color ??? can be switched to 4ub.
    };*/

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
    
    /*enum
    {
        STATE_COLOR                 = 1 << 0, 
        STATE_BLEND_ENABLED         = 1 << 1,
        STATE_SRC_BLEND             = 1 << 2,
        STATE_DEST_BLEND            = 1 << 3, 
        STATE_BLEND_FUNC            = (1 << 2) | (1 << 3),
        STATE_TEXTURE_ENABLED       = 1 << 4,
        STATE_TEXTURE               = 1 << 5,
        STATE_DEPTH_TEST_ENABLED    = 1 << 6,
        STATE_DEPTH_WRITE_ENABLED   = 1 << 7,
        STATE_STENCIL_TEST_ENABLED  = 1 << 8,
//        STATE_ALPHA_TEST_ENABLED    = 1 << 9,
//        STATE_SCISSOR_TEST_ENABLED  = 1 << 10,
        
        
        STATE_TEXTURE_0
        
    };*/

    RenderState();
    ~RenderState();
        
    Core::eRenderer renderer;
    //uint32 state;
    ////mutable uint32 changeSet;
    
    Color		color;
	Rect		scissorRect;

	UniqueHandle stateHandle;
	UniqueHandle textureState;
    //eBlendMode	sourceFactor, destFactor;
    //eFace		cullMode;
    //eCmpFunc	alphaFunc;
    //uint8		alphaFuncCmpValue;
	//eCmpFunc	depthFunc;
		//eFillMode	fillMode;

	/*struct StencilState
	{
		StencilState();
		int32 ref;
		uint32 mask;
		eCmpFunc func[2];
		eStencilOp pass[2];
		eStencilOp fail[2];
		eStencilOp zFail[2];

		void SetCmpFuncFront(int32 val){ func[0] = (eCmpFunc)val; }
		int32 GetCmpFuncFront(){ return func[0]; }
		void SetCmpFuncBack(int32 val){ func[1] = (eCmpFunc)val; }
		int32 GetCmpFuncBack(){ return func[1]; }
		
		void SetPassFront(int32 val) { pass[0] = (eStencilOp)val; }
		int32 GetPassFront() { return pass[0]; }
		void SetPassBack(int32 val) { pass[1] = (eStencilOp)val; }
		int32 GetPassBack() { return pass[1]; }

		void SetFailFront(int32 val) { fail[0] = (eStencilOp)val; }
		int32 GetFailFront() { return fail[0]; }
		void SetFailBack(int32 val) { fail[1] = (eStencilOp)val; }
		int32 GetFailBack() { return fail[1]; }

		void SetZFailFront(int32 val) { zFail[0] = (eStencilOp)val; }
		int32 GetZFailFront() { return zFail[0]; }
		void SetZFailBack(int32 val) { zFail[1] = (eStencilOp)val; }
		int32 GetZFailBack() { return zFail[1]; }

		INTROSPECTION(StencilState, 
			MEMBER(ref, "ref", I_VIEW | I_EDIT)
			MEMBER(mask, "mask", I_VIEW | I_EDIT)

			PROPERTY("cmpFuncFront", "cmpFuncFront", GetCmpFuncFront, SetCmpFuncFront, I_VIEW | I_EDIT)
			PROPERTY("cmpFuncBack", "cmpFuncBack", GetCmpFuncBack, SetCmpFuncBack, I_VIEW | I_EDIT)

			PROPERTY("passFront", "iPassFront", GetPassFront, SetPassFront, I_VIEW | I_EDIT)
			PROPERTY("passBack", "iPassBack", GetPassBack, SetPassBack, I_VIEW | I_EDIT)

			PROPERTY("failFront", "iFailFront", GetFailFront, SetFailFront, I_VIEW | I_EDIT)
			PROPERTY("failBack", "iFailBack", GetFailBack, SetFailBack, I_VIEW | I_EDIT)

			PROPERTY("IZFailFront", "iZFailFront", GetZFailFront, SetZFailFront, I_VIEW | I_EDIT)
			PROPERTY("IZFailBack", "iZFailBack", GetZFailBack, SetZFailBack, I_VIEW | I_EDIT)
			)
	};
	StencilState stencilState;*/
    
    static const uint32 MAX_TEXTURE_LEVELS = 8;
    //Texture * currentTexture[MAX_TEXTURE_LEVELS];
    Shader * shader;
    
    //void AppendState(uint32 state);
    //void RemoveState(uint32 state);
    //void SetState(uint32 state);
    //uint32 GetState();
    
    // STATE_COLOR
    inline void SetColor(float32 _r, float32 _g, float32 _b, float32 _a);
    inline void SetColor(const Color & _color);
	inline void ResetColor();
    inline const Color & GetColor() const;
    
    // STATE_BLEND_FUNC
    //inline void SetBlendMode(eBlendMode _sourceFactor, eBlendMode _destFactor);
	//inline eBlendMode GetSrcBlend();
	//inline eBlendMode GetDestBlend();
    
    // STATE_TEXTURE
    //inline void SetTexture(Texture *texture, uint32 textureLevel = 0);
	//inline Texture * GetTexture(uint32 textureLevel = 0);
    
    // SHADER
    inline void SetShader(Shader * shader);
    
    // CULL MODE
	//inline int32 GetCullMode();
    //inline void SetCullMode(int32 mode);
    
    // ALPHA
    //inline void SetAlphaFunc(eCmpFunc func, float32 cmpValue);

	// DEPTH
	//inline void SetDepthFunc(eCmpFunc func);

	// STENCIL
	//inline void SetStencilRef(int32 ref);
	//inline void SetStencilMask(uint32 mask);
	//inline void SetStencilFunc(eFace face, eCmpFunc func);
	//inline void SetStencilPass(eFace face, eStencilOp operation);
	//inline void SetStencilFail(eFace face, eStencilOp operation);
	//inline void SetStencilZFail(eFace face, eStencilOp operation);

	//SCISSOR
	inline void SetScissorRect(const Rect & rect);

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
	inline void SetScissorRectInHW() const;

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

    
//inline void RenderState::AppendState(uint32 _state)
//{
//   state |= _state;
//}

//inline void RenderState::RemoveState(uint32 _state)
//{
//    state &= ~_state;
//}

//inline void RenderState::SetState(uint32 _state)
//{
//    state = _state;
//}

//inline uint32 RenderState::GetState()
//{
//    return state;
//}
    
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

//inline void RenderState::SetBlendMode(eBlendMode _sourceFactor, eBlendMode _destFactor)
//{
    ////if ((sourceFactor != _sourceFactor) || (destFactor != _destFactor))
//    {
//        sourceFactor = _sourceFactor;
//        destFactor = _destFactor;
        ////changeSet |= STATE_CHANGED_SRC_BLEND | STATE_CHANGED_DEST_BLEND;
//    }
//}
    
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

// CULL MODE
//inline int32 RenderState::GetCullMode()
//{
//	return cullMode;
//}


//inline void RenderState::SetCullMode(int32 _cullMode)
//{
    ////if (cullMode != _cullMode)
//    {
//        cullMode = (eFace)_cullMode;
        ////changeSet |= STATE_CHANGED_CULLMODE;
//    }
//}

//inline void RenderState::SetAlphaFunc(eCmpFunc func, float32 cmpValue)
//{
//    uint8 newCmpValue = (uint8)(cmpValue * 255.0f);
//    //if ((alphaFunc != func) || (alphaFuncCmpValue != newCmpValue))
//    {
//        alphaFunc = func;
//        alphaFuncCmpValue = newCmpValue;
//        //changeSet |= STATE_CHANGED_ALPHA_FUNC;
//    }
//}

//inline void RenderState::SetDepthFunc(eCmpFunc func)
//{
//	//if(depthFunc != func)
//	{
//		depthFunc = func;
//		//changeSet |= STATE_CHANGED_DEPTH_FUNC;
//	}
//}

//inline void RenderState::SetScissorRect(const Rect & rect)
//{
//	//if(scissorRect != rect)
//	{
//		scissorRect = rect;
//		//changeSet |= STATE_CHANGED_SCISSOR_RECT;
//	}
//}

//inline void RenderState::SetFillMode(eFillMode _fillMode)
//{
//	fillMode = _fillMode;
//}
    
//inline eBlendMode RenderState::GetSrcBlend()
//{
//    return sourceFactor;
//}

//inline eBlendMode RenderState::GetDestBlend()
//{
//    return destFactor;
//}

// STATE_TEXTURE
/*inline void RenderState::SetTexture(Texture *texture, uint32 textureLevel)
{
	if(texture != currentTexture[textureLevel])
	{
		SafeRelease(currentTexture[textureLevel]);
		currentTexture[textureLevel] = SafeRetain(texture);
	}
}

inline Texture * RenderState::GetTexture(uint32 textureLevel)
{
    return currentTexture[textureLevel];
}*/

//inline void RenderState::SetStencilRef(int32 ref)
//{
//	stencilState.ref = ref;
//}

//inline void RenderState::SetStencilMask(uint32 mask)
//{
//	stencilState.mask = mask;
//}

//inline void RenderState::SetStencilFunc(eFace face, eCmpFunc func)
//{
//	if(face == FACE_FRONT_AND_BACK)
//	{
//		stencilState.func[0] = stencilState.func[1] = func;
//	}
//	else
//	{
//		stencilState.func[face] = func;
//	}
//}

//inline void RenderState::SetStencilPass(eFace face, eStencilOp operation)
//{
//	if(face == FACE_FRONT_AND_BACK)
//	{
//		stencilState.pass[0] = stencilState.pass[1] = operation;
//	}
//	else
//	{
//		stencilState.pass[face] = operation;
//	}
//}

//inline void RenderState::SetStencilFail(eFace face, eStencilOp operation)
//{
//	if(face == FACE_FRONT_AND_BACK)
//	{
//		stencilState.fail[0] = stencilState.fail[1] = operation;
//	}
//	else
//	{
//		stencilState.fail[face] = operation;
//	}
//}

//inline void RenderState::SetStencilZFail(eFace face, eStencilOp operation)
//{
//	if(face == FACE_FRONT_AND_BACK)
//	{
//		stencilState.zFail[0] = stencilState.zFail[1] = operation;
//	}
//	else
//	{
//		stencilState.zFail[face] = operation;
//	}
//}
    
};

#endif // __DAVAENGINE_RENDERSTATEBLOCK_H__
