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


#ifndef __DAVAENGINE_TEXTURESTATEDATAUNIQUEHANDLER_H__
#define __DAVAENGINE_TEXTURESTATEDATAUNIQUEHANDLER_H__

#include "Render/TextureStateData.h"

namespace DAVA
{
	class TextureStateDataUniqueHandler
	{
	public:
		
		void Assign(TextureStateData* to, const TextureStateData* from)
		{
			*to = *from;
		}
		
		void Release(TextureStateData* data)
		{
			//VI: do not release anything until Clear() called
			
			/*for(size_t i = 0; i < MAX_TEXTURE_COUNT; ++i)
			{
				if(data->textures[i])
				{
					data->textures[i]->Release();
				}
			}*/
		}
		
		void Clear(TextureStateData* data)
		{
			data->ReleaseAll();
		}
		
		bool Equals(const TextureStateData* a, const TextureStateData* b)
		{
			bool equals = (a == b);
			
			if(!equals)
			{
				equals = true;
				for(size_t i = 0; i < MAX_TEXTURE_COUNT; ++i)
				{
					if(a->textures[i] != b->textures[i])
					{
						equals = false;
						break;
					}
				}
			}
			
			return equals;
		}
	};
};

#endif
