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


#ifndef __DAVAENGINE_SHARED_FBO__
#define __DAVAENGINE_SHARED_FBO__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Render/Texture.h"
#include "Render/UniqueStateSet.h"
	
namespace DAVA
{

class SharedFBO : public BaseObject
{
public:
	struct Block
	{
		Vector2 offset;
		Vector2 size;

		int32 poolIndex;//for internal usage
	};

	struct Setup
	{
		Vector2 size; //texture size
		PixelFormat pixelFormat;
		Texture::DepthFormat depthFormat;
		Vector<std::pair<int32, Vector2> > blocks; //pair is <blocksCount, blockSize>
	};
protected:
	~SharedFBO();
public:
	SharedFBO(Setup * setup);

	Block * AcquireBlock(const Vector2 & size);
	void ReleaseBlock(Block * block);
	Texture * GetTexture();
	UniqueHandle GetTextureHandle();

private:
	SharedFBO();
	Texture * texture;
	Vector<Block*> blocks;

	Vector<Vector2> sizes;
	Vector<Deque<Block*> > queues;
	Vector<int32> frees;
	
	UniqueHandle fboTextureState;

	int32 FindIndexForSize(const Vector2 & size);
	Block * GetBlock(int32 poolIndex);
};

};

#endif //__DAVAENGINE_SHARED_FBO__
