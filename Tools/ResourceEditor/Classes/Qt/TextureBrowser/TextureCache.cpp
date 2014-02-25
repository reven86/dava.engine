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



#include "TextureBrowser/TextureCache.h"

DAVA::Vector<QImage> TextureCache::getOriginal(const DAVA::TextureDescriptor *descriptor)
{
	DAVA::Vector<QImage> images;

	if(NULL != descriptor && cacheOriginal.contains(descriptor))
	{
		// update weight for this cached
		cacheOriginal[descriptor].weight = curOriginalWeight++;

		images = cacheOriginal[descriptor].images;
	}

	return images;
}

DAVA::Vector<QImage> TextureCache::getConverted(const DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu)
{
	DAVA::Vector<QImage> images;

	if( NULL != descriptor &&
		gpu > DAVA::GPU_UNKNOWN && gpu < DAVA::GPU_FAMILY_COUNT &&
		cacheConverted[gpu].contains(descriptor))
	{
		// update weight for this cached
		cacheConverted[gpu][descriptor].weight = curConvertedWeight[gpu]++;

		images = cacheConverted[gpu][descriptor].images;
	}

	return images;
}

void TextureCache::setOriginal(const DAVA::TextureDescriptor *descriptor, DAVA::Vector<QImage>& images)
{
	if(NULL != descriptor)
	{
		DAVA::Vector<QImage> tmpImages;
		for(size_t i = 0; i < images.size(); ++i)
		{
			tmpImages.push_back(images[i]);
		}
		
		cacheOriginal[descriptor] = CacheEntity(tmpImages, curOriginalWeight++);

		// reached max count
		if(cacheOriginal.size() > maxOrigCount)
		{
			int weightToRemove = curOriginalWeight - maxOrigCount;
			
			QMapIterator<const DAVA::TextureDescriptor*, CacheEntity> iter(cacheOriginal);
			while(iter.hasNext())
			{
				iter.next();
				if(iter.value().weight < weightToRemove)
				{
					cacheOriginal.remove(iter.key());
				}
			}
		}
	}
}

void TextureCache::setConverted(const DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu, DAVA::Vector<QImage>& images)
{
	if( NULL != descriptor && 
		gpu > DAVA::GPU_UNKNOWN && gpu < DAVA::GPU_FAMILY_COUNT)
	{
		DAVA::Vector<QImage> tmpImages;
		for(int i = 0; i < (int)images.size(); ++i)
		{
			tmpImages.push_back(images[i]);
		}

		cacheConverted[gpu][descriptor] = CacheEntity(tmpImages, curConvertedWeight[gpu]++);

		// reached max count
		if(cacheConverted[gpu].size() > maxConvertedCount)
		{
			int weightToRemove = curConvertedWeight[gpu] - maxConvertedCount;

			QMapIterator<const DAVA::TextureDescriptor*, CacheEntity> iter(cacheConverted[gpu]);
			while(iter.hasNext())
			{
				iter.next();
				if(iter.value().weight < weightToRemove)
				{
					cacheConverted[gpu].remove(iter.key());
				}
			}
		}
	}
}

void TextureCache::clearAll()
{
	cacheOriginal.clear();
	for(int i = DAVA::GPU_UNKNOWN + 1; i < DAVA::GPU_FAMILY_COUNT; ++i)
	{
		cacheConverted[i].clear();
	}
}

void TextureCache::clearOriginal(const DAVA::TextureDescriptor *descriptor)
{
	cacheOriginal.remove(descriptor);
}

void TextureCache::clearConverted(const DAVA::TextureDescriptor *descriptor, DAVA::eGPUFamily gpu)
{
	if(gpu > DAVA::GPU_UNKNOWN && gpu < DAVA::GPU_FAMILY_COUNT)
	{
		cacheConverted[gpu].remove(descriptor);
	}
}
