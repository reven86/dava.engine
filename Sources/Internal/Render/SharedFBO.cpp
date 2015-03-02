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



#include "Render/SharedFBO.h"
#include "Render/RenderManager.h"
#include "Math/RectPacker.h"

namespace DAVA
{

bool SortBlocks(SharedFBO::Block * a, SharedFBO::Block * b)
{
	return a->size.SquareLength() > b->size.SquareLength();
}

SharedFBO::SharedFBO(Setup * setup)
:	texture(0)
{
	texture = Texture::CreateFBO((uint32)setup->size.x, (uint32)setup->size.y, setup->pixelFormat, setup->depthFormat);
	
	TextureStateData textureState;
	textureState.SetTexture(0, texture);
	fboTextureState = RenderManager::Instance()->CreateTextureState(textureState);

	int32 blocksCount = static_cast<int32>(setup->blocks.size());
	for(int32 i = 0; i < blocksCount; ++i)
	{
		sizes.push_back(setup->blocks[i].second);
		Deque<Block*> queue;
		std::pair<int32, Vector2> & blockSetup = setup->blocks[i];
		int32 count = blockSetup.first;
		for(int32 j = 0; j < count; ++j)
		{
			Block * block = new Block();
			block->poolIndex = i;
			block->size = blockSetup.second;
			blocks.push_back(block);
			queue.push_back(block);
		}

		queues.push_back(queue);
		frees.push_back(static_cast<int32>(queue.size()));
	}

	std::sort(blocks.begin(), blocks.end(), SortBlocks);
	
	RectPacker packer(Rect2i(0, 0, (int32)setup->size.x, (int32)setup->size.y));
	int32 blocksSize = static_cast<int32>(blocks.size());
	for(int32 i = 0; i < blocksSize; ++i)
	{
		bool res = packer.AddRect(Size2i((int32)blocks[i]->size.dx, (int32)blocks[i]->size.dy), blocks[i]);
		if(!res)
		{
			Logger::Error("SharedFBO failed to pack a rect %.0f %.0f, %d left unpacked", blocks[i]->size.dx, blocks[i]->size.dy, blocksSize-i);
			break;
		}
	}

	for(int32 i = 0; i < blocksSize; ++i)
	{
		Rect2i * rect = packer.SearchRectForPtr(blocks[i]);
		blocks[i]->offset = Vector2((float32)rect->x, (float32)rect->y);
	}
}

SharedFBO::~SharedFBO()
{
	RenderManager::Instance()->ReleaseTextureState(fboTextureState);
	SafeRelease(texture);

    for(auto& block : blocks)
    {
        SafeDelete(block);
    }
}

SharedFBO::Block * SharedFBO::AcquireBlock(const Vector2 & size)
{
	int32 index = FindIndexForSize(size);

	if(index > 0)
	{
		if(frees[index] < frees[index-1])
		{
			index--;
		}
	}

	Block * block = 0;
	int32 maxIndex = static_cast<int32>(frees.size()-1);

	//debug
	//Logger::FrameworkDebug("Free blocks:"); 
	//for(int32 i = 0; i <= maxIndex; ++i)
	//{
	//	Logger::FrameworkDebug("(%.1f_%.1f) %d", sizes[i].x, sizes[i].y, frees[i]);
	//}

	//first try to find block of closest size
	block = GetBlock(index);

	//try to get larger block
	if(!block)
	{
		int32 largerIndex = index;
		while((!block) && (largerIndex > 0))
		{
			largerIndex--;
			block = GetBlock(largerIndex);
		}
	}

	//try to get smaller block
	if(!block)
	{
		int32 smallerIndex = index;
		while((!block) && (smallerIndex < maxIndex))
		{
			smallerIndex++;
			block = GetBlock(smallerIndex);
		}
	}

	return block;
}

SharedFBO::Block * SharedFBO::GetBlock(int32 poolIndex)
{
	if(frees[poolIndex] > 0)
	{
		DVASSERT(queues[poolIndex].size() > 0);

		Block * block = queues[poolIndex].front();
		queues[poolIndex].pop_front();
		frees[poolIndex]--;

		return block;
	}
	else
	{
		return 0;
	}
}

void SharedFBO::ReleaseBlock(SharedFBO::Block * block)
{
	queues[block->poolIndex].push_back(block);
	frees[block->poolIndex]++;
}

int32 SharedFBO::FindIndexForSize(const Vector2 & size)
{
	int32 sizesSize = static_cast<int32>(sizes.size());
	int32 closestIndex = 1;
    
	for(int32 i = sizesSize-1; i >= 0; --i)
	{
		if((sizes[i].x >= size.x) && (sizes[i].y >= size.y))
		{
			return i;
		}
	}

	return closestIndex;
}

Texture * SharedFBO::GetTexture()
{
	return texture;
}

UniqueHandle SharedFBO::GetTextureHandle()
{
	return fboTextureState;
}
};
