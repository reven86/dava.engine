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


#ifndef __DAVAENGINE_RENDERSTATEDATA_H__
#define __DAVAENGINE_RENDERSTATEDATA_H__

#include "Render/RenderBase.h"

namespace DAVA
{
//this class holds exclusively render state data such as blen parameters, depth parameters etc
//it will be a part of RenderState

class RenderStateData
{
public:
	
	enum
	{
		// bit flags
		STATE_BLEND = 1 << 0,            //
		STATE_DEPTH_TEST = 1 << 1,
		STATE_DEPTH_WRITE = 1 << 2,
		STATE_STENCIL_TEST = 1 << 3,
		STATE_CULL = 1 << 4,
		
		STATE_SCISSOR_TEST = 1 << 6,
					
		STATE_COLORMASK_RED =  1 << 15,
		STATE_COLORMASK_GREEN = 1 << 16,
		STATE_COLORMASK_BLUE = 1 << 17,
		STATE_COLORMASK_ALPHA = 1 << 18,
		STATE_COLORMASK_ALL = (STATE_COLORMASK_RED | STATE_COLORMASK_GREEN | STATE_COLORMASK_BLUE | STATE_COLORMASK_ALPHA),
		
		//legacy fixed func states
		STATE_ALPHA_TEST = 1 << 5,          // fixed func only / in programmable pipeline can check for consistency
					
		STATE_TEXTURE0 = 1 << 7,            // fixed func only / in programmable pipeline only checks for consistency
		STATE_TEXTURE1 = 1 << 8,            // fixed func only / in programmable pipeline only checks for consistency
		STATE_TEXTURE2 = 1 << 9,            // fixed func only / in programmable pipeline only checks for consistency
		STATE_TEXTURE3 = 1 << 10,            // fixed func only / in programmable pipeline only checks for consistency
		STATE_TEXTURE4 = 1 << 11,
		STATE_TEXTURE5 = 1 << 12,
		STATE_TEXTURE6 = 1 << 13,
		STATE_TEXTURE7 = 1 << 14,

	};

	uint32 state; //one of STATE_* values bits
	
	//blend mode
	eBlendMode	sourceFactor;
	eBlendMode  destFactor;
	
	//cull mode
	eFace		cullMode;
	
	//depth function
	eCmpFunc	depthFunc;
	
	//fill mode
	eFillMode	fillMode;
	
	//stencil state
	int32 stencilRef;
	uint32 stencilMask;
	eCmpFunc stencilFunc[2];
	eStencilOp stencilPass[2];
	eStencilOp stencilFail[2];
	eStencilOp stencilZFail[2];
	
	RenderStateData() :
	state(0),
	sourceFactor(BLEND_ONE),
	destFactor(BLEND_ONE),
	cullMode(FACE_COUNT),
	depthFunc(CMP_NEVER),
	fillMode(FILLMODE_COUNT),
	stencilRef(0),
	stencilMask(0)
	{
		memset(stencilFunc, 0, sizeof(stencilFunc));
		memset(stencilPass, 0, sizeof(stencilPass));
		memset(stencilFail, 0, sizeof(stencilFail));
		memset(stencilZFail, 0, sizeof(stencilZFail));
	}
	
	RenderStateData(const RenderStateData& data)
	{
		state = data.state;
		
		sourceFactor = data.sourceFactor;
		destFactor = data.destFactor;
		
		//cull mode
		cullMode = data.cullMode;
		
		//depth function
		depthFunc = data.depthFunc;
		
		//fill mode
		fillMode = data.fillMode;
		
		//stencil state
		stencilRef = data.stencilRef;
		stencilMask = data.stencilMask;
		memcpy(stencilFunc, data.stencilFunc, sizeof(stencilFunc));
		memcpy(stencilPass, data.stencilPass, sizeof(stencilPass));
		memcpy(stencilFail, data.stencilFail, sizeof(stencilFail));
		memcpy(stencilZFail, data.stencilZFail, sizeof(stencilZFail));
	}
	
	RenderStateData& operator=(const RenderStateData& data)
	{
		state = data.state;
		
		sourceFactor = data.sourceFactor;
		destFactor = data.destFactor;
		
		//cull mode
		cullMode = data.cullMode;
		
		//depth function
		depthFunc = data.depthFunc;
		
		//fill mode
		fillMode = data.fillMode;
		
		//stencil state
		stencilRef = data.stencilRef;
		stencilMask = data.stencilMask;
		memcpy(stencilFunc, data.stencilFunc, sizeof(stencilFunc));
		memcpy(stencilPass, data.stencilPass, sizeof(stencilPass));
		memcpy(stencilFail, data.stencilFail, sizeof(stencilFail));
		memcpy(stencilZFail, data.stencilZFail, sizeof(stencilZFail));

		return *this;
	}
	
	void Clear()
	{
		//do nothing here for now. No resources to release.
	}
	
	bool Equals(const RenderStateData& data) const
	{
		return (0 == memcmp(this, &data, sizeof(RenderStateData)));
	}
};
};

#endif
