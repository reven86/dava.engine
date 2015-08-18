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
#include "Concurrency/Thread.h"

namespace DAVA
{

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

const String STENCIL_OP_NAMES[STENCILOP_COUNT] =
{
	"STENCILOP_KEEP",
	"STENCILOP_ZERO",
	"STENCILOP_REPLACE",
    "STENCILOP_INVERT",
	"STENCILOP_INCR",
    "STENCILOP_DECR",
	"STENCILOP_INCR_WRAP",	
	"STENCILOP_DECR_WRAP"
	
};   

const String FILL_MODE_NAMES[FILLMODE_COUNT] =
{
	"FILLMODE_POINT",
	"FILLMODE_WIREFRAME",
	"FILLMODE_SOLID"
};

rhi::CmpFunc GetCmpFuncByName(const String & cmpFuncStr)
{
    for(uint32 i = 0; i < CMP_TEST_MODE_COUNT; i++)
        if(cmpFuncStr == CMP_FUNC_NAMES[i])
            return (rhi::CmpFunc)i;

    return (rhi::CmpFunc)CMP_TEST_MODE_COUNT;
}

rhi::StencilOperation GetStencilOpByName(const String & stencilOpStr)
{
    for(uint32 i = 0; i < STENCILOP_COUNT; i++)
        if(stencilOpStr == STENCIL_OP_NAMES[i])
            return (rhi::StencilOperation)i;

    return (rhi::StencilOperation)STENCILOP_COUNT;
}


/*RHI_COMPLETE - make this stuff correspond with PolygonGroup::UpdateDataPointersAndStreams*/
inline uint32 GetPossibleTexcoordSemantic(uint32 index)
{
    switch (index)
    {
    case 0:
        return EVF_TEXCOORD0;// | EVF_CUBETEXCOORD0;
    case 1:
        return EVF_TEXCOORD1;// | EVF_CUBETEXCOORD1;
    case 2:
        return EVF_TEXCOORD2;// | EVF_CUBETEXCOORD2;
    case 3:
        return EVF_PIVOT; //    | EVF_TEXCOORD3 | EVF_CUBETEXCOORD3;
    case 4:
        return EVF_ANGLE_SIN_COS;
    case 5:
        return EVF_FLEXIBILITY;
    }
    
    return 0;
}


uint32 GetVertexLayoutRequiredFormat(const rhi::VertexLayout& layout)
{
    uint32 res = 0;
    for (uint32 i = 0, sz = layout.ElementCount(); i < sz; ++i)
    {
        rhi::VertexSemantics semantic = layout.ElementSemantics(i);
        switch (semantic)
        {
        case rhi::VS_POSITION:
            res |= EVF_VERTEX;
            break;
        case rhi::VS_NORMAL:
            res |= EVF_NORMAL;
            break;
        case rhi::VS_COLOR:
            res |= EVF_COLOR;
            break;
        case rhi::VS_TEXCOORD:
            res |= GetPossibleTexcoordSemantic(layout.ElementSemanticsIndex(i));
            break;
        case rhi::VS_TANGENT:
            res |= EVF_TANGENT;
            break;
        case rhi::VS_BINORMAL:
            res |= EVF_BINORMAL;
            break;
        case rhi::VS_BLENDWEIGHT:
            res |= EVF_JOINTWEIGHT;
            break;
        case rhi::VS_BLENDINDEX:
            res |= EVF_JOINTINDEX;
            break;
        default: break;
        }

    }    
    return res;
}

};
