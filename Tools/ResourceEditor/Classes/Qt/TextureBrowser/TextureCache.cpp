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


#include "TextureCache.h"
#include "TextureConvertor.h"
#include "Main/mainwindow.h"

#include <QPainter>

#include <QDebug>

TextureCache::TextureCache()
{
    qRegisterMetaType<QList<QImage>>();

    curThumbnailWeight = 0;
	curOriginalWeight = 0;
    for(int i = 0 ; i < DAVA::GPU_DEVICE_COUNT; ++i)
    {
        curConvertedWeight[i] = 0;
    }
    
    new TextureConvertor();
    
	QObject::connect(TextureConvertor::Instance(), SIGNAL(ReadyThumbnail(const DAVA::TextureDescriptor *, const TextureInfo &)), this, SLOT(ReadyThumbnail(const DAVA::TextureDescriptor *, const TextureInfo &)));
	QObject::connect(TextureConvertor::Instance(), SIGNAL(ReadyOriginal(const DAVA::TextureDescriptor *, const TextureInfo &)), this, SLOT(ReadyOriginal(const DAVA::TextureDescriptor *, const TextureInfo &)));
    QObject::connect(TextureConvertor::Instance(), SIGNAL(ReadyConverted(const DAVA::TextureDescriptor *, const DAVA::eGPUFamily, const TextureInfo &)), this, SLOT(ReadyConverted(const DAVA::TextureDescriptor *, const DAVA::eGPUFamily, const TextureInfo &)));
}

TextureCache::~TextureCache()
{
    TextureConvertor::Instance()->Release();
}

void TextureCache::getThumbnail( const DAVA::TextureDescriptor *descriptor, QObject *object, const QString& slotName, const QVariant& userData )
{
    Q_ASSERT( descriptor );
    Q_ASSERT( object );

	if(descriptor->pathname.IsEmpty()) 
		return;

    const DAVA::FilePath key = TextureDescriptor::GetDescriptorPathname( descriptor->GetSourceTexturePathname() );
    CacheMap::const_iterator i = cacheThumbnail.find( key );
    if ( i != cacheThumbnail.end() )
    {
        CacheRequest request(  key );
        request.registerObserver( object, slotName, userData );
        request.invoke( i->second.info.images );
    }
    else
    {
        QPointer< CacheRequest > request;
        RequestPool::iterator i = poolThumbnail.find( key );
        if ( i == poolThumbnail.end() )
        {
            request = new CacheRequest( key );
            request->registerObserver( object, slotName, userData );
            poolThumbnail[key] = request;
            TextureConvertor::Instance()->GetThumbnail( descriptor );
        }
        else
        {
            request = i.value();
            request->registerObserver( object, slotName, userData );
        }
    }
}

QList<QImage> TextureCache::getThumbnail(const DAVA::TextureDescriptor *descriptor)
{
    if(NULL == descriptor)
        return QList<QImage>();
    
    const DAVA::FilePath & path = descriptor->pathname;
	if(cacheThumbnail.find(path) != cacheThumbnail.end())
	{
		// update weight for this cached
		cacheThumbnail[path].weight = curThumbnailWeight++;
        return cacheThumbnail[path].info.images;
	}
    
	return QList<QImage>();
}

QList<QImage> TextureCache::getOriginal(const DAVA::TextureDescriptor *descriptor)
{
    if(NULL == descriptor)
        return QList<QImage>();

    const DAVA::FilePath & path = descriptor->pathname;
	if(cacheOriginal.find(path) != cacheOriginal.end())
	{
		// update weight for this cached
		cacheOriginal[path].weight = curOriginalWeight++;
		return cacheOriginal[path].info.images;
	}

	return QList<QImage>();
}

QList<QImage> TextureCache::getConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu)
{
    if(NULL == descriptor)
        return QList<QImage>();

    const DAVA::FilePath & path = descriptor->pathname;
	if((gpu >= 0 && gpu < DAVA::GPU_DEVICE_COUNT) && (cacheConverted[gpu].find(path) != cacheConverted[gpu].end()))
	{
		// update weight for this cached
		cacheConverted[gpu][path].weight = curConvertedWeight[gpu]++;
		return cacheConverted[gpu][path].info.images;
	}

	return QList<QImage>();
}

void TextureCache::ClearCache()
{
    clearInsteadThumbnails();
    cacheThumbnail.clear();
}

void TextureCache::ReadyThumbnail(const DAVA::TextureDescriptor *descriptor, const TextureInfo & image)
{
	setThumbnail(descriptor, image);
}

void TextureCache::ReadyOriginal(const DAVA::TextureDescriptor *descriptor, const TextureInfo & image)
{
    setOriginal(descriptor, image);
    setThumbnail(descriptor, image);
}

void TextureCache::ReadyConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu, const TextureInfo & image)
{
    setConverted(descriptor, gpu, image);
}

void TextureCache::setThumbnail(const DAVA::TextureDescriptor *descriptor, const TextureInfo &images)
{
	if(NULL != descriptor)
	{
		TextureInfo info;
		info.dataSize = images.dataSize;
		info.fileSize = images.fileSize;

        info.images.reserve(images.images.size());
		for(int i = 0; i < images.images.size(); ++i)
		{
            QImage img(THUMBNAIL_SIZE, THUMBNAIL_SIZE, QImage::Format_ARGB32);
            img.fill(QColor(Qt::white));
            
            QPainter painter(&img);
            
            QSize imageSize = images.images[i].rect().size();
            imageSize.scale(THUMBNAIL_SIZE, THUMBNAIL_SIZE, Qt::KeepAspectRatio);
            int x = (THUMBNAIL_SIZE - imageSize.width()) / 2;
            int y = (THUMBNAIL_SIZE - imageSize.height()) / 2;
            painter.drawImage(QRect(QPoint(x, y), imageSize), images.images[i]);
            
            painter.end();

            info.images.push_back(img);
		}
        
        const DAVA::FilePath & path = descriptor->pathname;

        cacheThumbnail[path] = CacheEntity(info, curThumbnailWeight++);
        ClearCacheTail(cacheThumbnail, curThumbnailWeight, maxThumbnailCount);

        emit ThumbnailLoaded(descriptor, info);

        RequestPool::iterator i = poolThumbnail.find( path );
        if ( i != poolThumbnail.end() && i.value() )
        {
            i.value()->invoke( info.images );
            delete i.value();
            poolThumbnail.remove( path );
        }
    }
}

void TextureCache::setOriginal(const DAVA::TextureDescriptor *descriptor, const TextureInfo & images)
{
	if(NULL != descriptor)
	{
		cacheOriginal[descriptor->pathname] = CacheEntity(images, curOriginalWeight++);
        ClearCacheTail(cacheOriginal, curOriginalWeight, maxOrigCount);
        
        emit OriginalLoaded(descriptor, images);
	}
}

void TextureCache::setConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu, const TextureInfo & images)
{
	if( NULL != descriptor && 
		gpu >= 0 && gpu < DAVA::GPU_DEVICE_COUNT)
	{
		cacheConverted[gpu][descriptor->pathname] = CacheEntity(images, curConvertedWeight[gpu]++);
        ClearCacheTail(cacheConverted[gpu], curConvertedWeight[gpu], maxConvertedCount);
        
        emit ConvertedLoaded(descriptor, gpu, images);
	}
}

void TextureCache::ClearCacheTail(DAVA::Map<const DAVA::FilePath, CacheEntity> & cache, const size_t currentWeight, const size_t maxWeight)
{
    if(cache.size() > maxWeight)
    {
        size_t weightToRemove = currentWeight - maxWeight;
 
        DAVA::Map<const DAVA::FilePath, CacheEntity>::iterator it = cache.begin();
        while(it != cache.end())
        {
            if(it->second.weight < weightToRemove)
            {
                cache.erase(it++);
            }
            else
            {
                ++it;
            }
        }
    }
}

void TextureCache::clearInsteadThumbnails()
{
	cacheOriginal.clear();
	for(int i = 0; i < DAVA::GPU_DEVICE_COUNT; ++i)
	{
		cacheConverted[i].clear();
	}
}

void TextureCache::clearThumbnail(const DAVA::TextureDescriptor *descriptor)
{
    RemoveFromCache(cacheThumbnail, descriptor);
}

void TextureCache::clearOriginal(const DAVA::TextureDescriptor *descriptor)
{
    RemoveFromCache(cacheOriginal, descriptor);
}

void TextureCache::clearConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu)
{
	if(gpu >= 0 && gpu < DAVA::GPU_DEVICE_COUNT)
	{
        RemoveFromCache(cacheConverted[gpu], descriptor);
	}
}

void TextureCache::RemoveFromCache(DAVA::Map<const DAVA::FilePath, CacheEntity> & cache, const DAVA::TextureDescriptor *descriptor)
{
    if(descriptor)
    {
        DAVA::Map<const DAVA::FilePath, CacheEntity>::iterator found = cache.find(descriptor->pathname);
        if(found != cache.end())
        {
            cache.erase(found);
        }
    }
}


DAVA::uint32 TextureCache::getThumbnailSize(const DAVA::TextureDescriptor *descriptor)
{
	if(NULL == descriptor)
		return 0;

	DAVA::Map<const DAVA::FilePath, CacheEntity>::const_iterator it = cacheThumbnail.find(descriptor->pathname);
	if(it != cacheThumbnail.end())
	{
		return it->second.info.dataSize;
	}

	return 0;
}

DAVA::uint32 TextureCache::getOriginalSize(const DAVA::TextureDescriptor *descriptor)
{
	if(NULL == descriptor)
		return 0;

	DAVA::Map<const DAVA::FilePath, CacheEntity>::const_iterator it = cacheOriginal.find(descriptor->pathname);
	if(it != cacheOriginal.end())
	{
		return it->second.info.dataSize;
	}

	return 0;
}

DAVA::uint32 TextureCache::getOriginalFileSize(const DAVA::TextureDescriptor *descriptor)
{
	if(NULL == descriptor)
		return 0;

	DAVA::Map<const DAVA::FilePath, CacheEntity>::const_iterator it = cacheOriginal.find(descriptor->pathname);
	if(it != cacheOriginal.end())
	{
		return it->second.info.fileSize;
	}

	return 0;
}

QSize TextureCache::getOriginalImageSize(const DAVA::TextureDescriptor *descriptor) const
{
	if(NULL == descriptor)
		return QSize();

	DAVA::Map<const DAVA::FilePath, CacheEntity>::const_iterator it = cacheOriginal.find(descriptor->pathname);
	if(it != cacheOriginal.end())
	{
		return it->second.info.imageSize;
	}

	return QSize();
}



DAVA::uint32 TextureCache::getConvertedSize(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu)
{
	if(NULL == descriptor)
		return 0;

	DAVA::Map<const DAVA::FilePath, CacheEntity>::const_iterator it = cacheConverted[gpu].find(descriptor->pathname);
	if(it != cacheConverted[gpu].end())
	{
		return it->second.info.dataSize;
	}

	return 0;
}

DAVA::uint32 TextureCache::getConvertedFileSize(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu)
{
	if(NULL == descriptor)
		return 0;

	DAVA::Map<const DAVA::FilePath, CacheEntity>::const_iterator it = cacheConverted[gpu].find(descriptor->pathname);
	if(it != cacheConverted[gpu].end())
	{
		return it->second.info.fileSize;
	}

	return 0;
}

QSize TextureCache::getConvertedImageSize(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu) const
{
	if(NULL == descriptor)
		return QSize();

	DAVA::Map<const DAVA::FilePath, CacheEntity>::const_iterator it = cacheConverted[gpu].find(descriptor->pathname);
	if(it != cacheConverted[gpu].end())
	{
		return it->second.info.imageSize;
	}

	return QSize();
}



