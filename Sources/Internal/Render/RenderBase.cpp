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
#include "Platform/Thread.h"

namespace DAVA
{
const String BLEND_MODE_NAMES[BLEND_MODE_COUNT] =
{
	"BLEND_NONE",
	"BLEND_ZERO",
	"BLEND_ONE",
	"BLEND_DST_COLOR",
	"BLEND_ONE_MINUS_DST_COLOR",
	"BLEND_SRC_ALPHA",
	"BLEND_ONE_MINUS_SRC_ALPHA",
	"BLEND_DST_ALPHA",
	"BLEND_ONE_MINUS_DST_ALPHA",
	"BLEND_SRC_ALPHA_SATURATE",
	"BLEND_SRC_COLOR",
	"BLEND_ONE_MINUS_SRC_COLOR"
};

#if defined(__DAVAENGINE_OPENGL__)
const GLint BLEND_MODE_MAP[BLEND_MODE_COUNT] =
{
	0,	// not a valid blend mode
	GL_ZERO,
	GL_ONE,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,
	GL_SRC_ALPHA_SATURATE,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
};
#elif defined(__DAVAENGINE_DIRECTX9__)
const GLint BLEND_MODE_MAP[BLEND_MODE_COUNT] =
{
	0,	// not a valid blend mode
	D3DBLEND_ZERO,
	D3DBLEND_ONE,
	D3DBLEND_DESTCOLOR,
	D3DBLEND_INVDESTCOLOR,
	D3DBLEND_SRCALPHA,
	D3DBLEND_INVSRCALPHA,
	D3DBLEND_DESTALPHA,
	D3DBLEND_INVDESTALPHA,
	D3DBLEND_SRCALPHASAT,
	D3DBLEND_SRCCOLOR,
	D3DBLEND_INVSRCCOLOR,
};
#endif

const String CMP_FUNC_NAMES[CMP_TEST_MODE_COUNT] =
{
	"CMP_NEVER",
	"CMP_LESS",
	"CMP_EQUAL",
	"CMP_LEQUAL",
	"CMP_GREATER",
	"CMP_NOTEQUAL",
	"CMP_GEQUAL",
	"CMP_ALWAYS"
};

#if defined(__DAVAENGINE_OPENGL__)
const GLint COMPARE_FUNCTION_MAP[CMP_TEST_MODE_COUNT] =
{
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS,
};
#elif defined(__DAVAENGINE_DIRECTX9__)
const GLint COMPARE_FUNCTION_MAP[CMP_TEST_MODE_COUNT] =
{
	D3DCMP_NEVER,
	D3DCMP_LESS,
	D3DCMP_EQUAL,
	D3DCMP_LESSEQUAL,
	D3DCMP_GREATER,
	D3DCMP_NOTEQUAL,
	D3DCMP_GREATEREQUAL,
	D3DCMP_ALWAYS,
};
#endif

const String FACE_NAMES[FACE_COUNT] =
{
	"FACE_FRONT",
	"FACE_BACK",
	"FACE_FRONT_AND_BACK"
};

#if defined(__DAVAENGINE_OPENGL__)
const GLint CULL_FACE_MAP[FACE_COUNT] =
{
	GL_FRONT,
	GL_BACK,
	GL_FRONT_AND_BACK,
};
#elif defined(__DAVAENGINE_DIRECTX9__)
const int32 CULL_FACE_MAP[FACE_COUNT] =
{
	D3DCULL_CCW,
	D3DCULL_CW,
	D3DCULL_NONE,
};
#endif

const String STENCIL_OP_NAMES[STENCILOP_COUNT] =
{
	"STENCILOP_KEEP",
	"STENCILOP_ZERO",
	"STENCILOP_REPLACE",
	"STENCILOP_INCR",
	"STENCILOP_INCR_WRAP",
	"STENCILOP_DECR",
	"STENCILOP_DECR_WRAP",
	"STENCILOP_INVERT"
};

#if defined(__DAVAENGINE_OPENGL__)
const GLint STENCIL_OP_MAP[STENCILOP_COUNT] =
{
	GL_KEEP,
	GL_ZERO,
	GL_REPLACE,
	GL_INCR,
	GL_INCR_WRAP,
	GL_DECR,
	GL_DECR_WRAP,
	GL_INVERT
};
#elif defined(__DAVAENGINE_DIRECTX9__)
const int32 STENCIL_OP_MAP[STENCILOP_COUNT] =
{
	D3DSTENCILOP_KEEP,
	D3DSTENCILOP_ZERO,
	D3DSTENCILOP_REPLACE,
	D3DSTENCILOP_INCRSAT,
	D3DSTENCILOP_INCR,
	D3DSTENCILOP_DECRSAT,
	D3DSTENCILOP_DECR,
	D3DSTENCILOP_INVERT
};
#endif
    
    
#if defined(__DAVAENGINE_OPENGL__)
    const GLint TEXTURE_WRAP_MAP[WRAP_COUNT] =
    {
        GL_CLAMP_TO_EDGE,
        GL_REPEAT
    };
#elif defined(__DAVAENGINE_DIRECTX9__)
    const int32 TEXTURE_WRAP_MAP[WRAP_COUNT] =
    {
        D3DTADDRESS_CLAMP,
        D3DTADDRESS_WRAP
    };
#endif
    
    
#if defined(__DAVAENGINE_OPENGL__)
    const GLint TEXTURE_FILTER_MAP[FILTER_COUNT] =
    {
        GL_NEAREST,
        GL_LINEAR,
        GL_NEAREST_MIPMAP_NEAREST,
        GL_LINEAR_MIPMAP_NEAREST,
        GL_NEAREST_MIPMAP_LINEAR,
        GL_LINEAR_MIPMAP_LINEAR
    };
#elif defined(__DAVAENGINE_DIRECTX9__)
    const int32 TEXTURE_FILTER_MAP[FILTER_COUNT] =
    {
        D3DTEXF_POINT,
        D3DTEXF_LINEAR,
        
        D3DTEXF_ANISOTROPIC,
        D3DTEXF_PYRAMIDALQUAD,
        D3DTEXF_GAUSSIANQUAD,
        D3DTEXF_CONVOLUTIONMONO
    };
#endif


const String FILL_MODE_NAMES[FILLMODE_COUNT] =
{
	"FILLMODE_POINT",
	"FILLMODE_WIREFRAME",
	"FILLMODE_SOLID"
};

#if defined(__DAVAENGINE_WINDOWS_STORE__)
__DAVAENGINE_WINDOWS_STORE_INCOMPLETE_IMPLEMENTATION__MARKER__
//FILLMODE_MAP is not implemented for WinStore (OpenGL ES with ANGLE)
#endif

#if defined(__DAVAENGINE_OPENGL__) && (defined(__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WINDOWS_DESKTOP__))
const GLint FILLMODE_MAP[FILLMODE_COUNT] =
{
	GL_POINT,
	GL_LINE,
	GL_FILL
};
#elif defined(__DAVAENGINE_DIRECTX9__)
const int32 FILLMODE_MAP[FILLMODE_COUNT] =
{
	D3DFILL_POINT,
	D3DFILL_WIREFRAME,
	D3DFILL_SOLID
};
#endif
    
#if defined(__DAVAENGINE_OPENGL__)
const GLint BUFFERDRAWTYPE_MAP[BDT_COUNT] =
{
    GL_STATIC_DRAW,
    GL_DYNAMIC_DRAW,
};
#elif defined(__DAVAENGINE_DIRECTX9__)
const int32 BUFFERDRAWTYPE_MAP[BDT_COUNT] =
{
    0,
    0,
};
#endif

/*
 FastName("modelViewProjectionMatrix"),
 FastName("modelViewMatrix"),
 FastName("projectionMatrix"),
 FastName("normalMatrix"),
 FastName("flatColor"),
 FastName("globalTime"),
 FastName("worldTranslate"),
 FastName("worldScale"),
 */

const FastName DYNAMIC_PARAM_NAMES[] =
    {
        FastName("unknownSemantic"),
        FastName("worldMatrix"),//PARAM_WORLD,
        FastName("invWorldMatrix"), //PARAM_INV_WORLD,
        FastName("worldInvTransposeMatrix"), //PARAM_WORLD_INV_TRANSPOSE,
        
        FastName("viewMatrix"), //PARAM_VIEW,
        FastName("invViewMatrix"), //PARAM_INV_VIEW,
        FastName("projMatrix"), //PARAM_PROJ,
        FastName("invProjMatrix"), //PARAM_INV_PROJ,
        
        FastName("worldViewMatrix"), //PARAM_WORLD_VIEW,
        FastName("invWorldViewMatrix"), //PARAM_INV_WORLD_VIEW,
        FastName("worldViewInvTransposeMatrix"), //PARAM_NORMAL, // NORMAL MATRIX
        
        FastName("viewProjMatrix"), //PARAM_VIEW_PROJ,
        FastName("invViewProjMatrix"), //PARAM_INV_VIEW_PROJ,
        
        FastName("worldViewProjMatrix"), //PARAM_WORLD_VIEW_PROJ,
        FastName("invWorldViewProjMatrix"), //PARAM_INV_WORLD_VIEW_PROJ,
        
        FastName("flatColor"),
        FastName("globalTime"),
        FastName("worldScale"),

        FastName("cameraPosition"), // PARAM_CAMERA_POS,
        FastName("cameraDirection"), // PARAM_CAMERA_DIR,
        FastName("cameraUp"), // PARAM_CAMERA_UP,
        
        FastName("lightPosition0"),
        FastName("lightColor0"),
        FastName("lightAmbientColor0"),

        FastName("localBoundingBox"),
        FastName("worldViewObjectCenter"),
        FastName("boundingBoxSize"),

        FastName("trunkOscillationParams"),
        FastName("leafOscillationParams"),
        FastName("speedTreeLightSmoothing"),

        FastName("sphericalHarmonics[0]"),

        FastName("jointPositions[0]"),
        FastName("jointQuaternions[0]"),
        FastName("jointsCount"),

        FastName("rtSize"),
        FastName("rtPixelSize"),
        FastName("rtHalfPixelSize"),
        FastName("rtAspectRatio")

//        FastName("objectPosition"),
//        FastName("objectScale"),
    };

RenderGuard::RenderGuard()
{
    wrongCall = false;
}

RenderGuard::~RenderGuard()
{

}

void RenderGuard::LowLevelRenderCall()
{
    if(!Thread::IsMainThread())
    {
        DVASSERT(0 && "Application tried to call GL or DX in separate thread without lock");
    }
    if (!RenderManager::Instance()->IsInsideDraw())
    {
        DVASSERT(0 && "Application tried to call GL or DX not between BeginFrame / EndFrame.");
    }
}

eBlendMode GetBlendModeByName(const String & blendStr)
{
    for(uint32 i = 0; i < BLEND_MODE_COUNT; i++)
        if(blendStr == BLEND_MODE_NAMES[i])
            return (eBlendMode)i;

    return BLEND_MODE_COUNT;
}

eCmpFunc GetCmpFuncByName(const String & cmpFuncStr)
{
    for(uint32 i = 0; i < CMP_TEST_MODE_COUNT; i++)
        if(cmpFuncStr == CMP_FUNC_NAMES[i])
            return (eCmpFunc)i;

    return CMP_TEST_MODE_COUNT;
}

eFace GetFaceByName(const String & faceStr)
{
    for(uint32 i = 0; i < FACE_COUNT; i++)
        if(faceStr == FACE_NAMES[i])
            return (eFace)i;

    return FACE_COUNT;
}

eStencilOp GetStencilOpByName(const String & stencilOpStr)
{
    for(uint32 i = 0; i < STENCILOP_COUNT; i++)
        if(stencilOpStr == STENCIL_OP_NAMES[i])
            return (eStencilOp)i;

    return STENCILOP_COUNT;
}

eFillMode GetFillModeByName(const String & fillModeStr)
{
    for(uint32 i = 0; i < FILLMODE_COUNT; i++)
        if(fillModeStr == FILL_MODE_NAMES[i])
            return (eFillMode)i;

    return FILLMODE_COUNT;
}

};