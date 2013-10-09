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
	RenderGuard::RenderGuard()
	{
		wrongCall = false;
	}

	RenderGuard::~RenderGuard()
	{
			
	}
	
	void RenderGuard::LowLevelRenderCall()
	{
		if((!Thread::IsMainThread()) && RenderManager::Instance()->GetNonMainLockCount() == 0)
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