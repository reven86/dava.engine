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


#ifndef __TEXTURE_CACHE_H__
#define __TEXTURE_CACHE_H__

#include "Base/Singleton.h"
#include "Render/TextureDescriptor.h"
#include "FileSystem/FilePath.h"

#include "TextureInfo.h"
#include "CacheRequest.h"

#include <QPointer>
#include <QImage>
#include <QMap>

class TextureCache : public QObject, public DAVA::Singleton<TextureCache>
{
	Q_OBJECT

private:
	struct CacheEntity
	{
		CacheEntity()
        : weight(0)
		{ }
        
		CacheEntity(const TextureInfo & _info, const size_t _weight)
        : info(_info), weight(_weight)
		{ }
        
		TextureInfo info;
		size_t weight;
	};

    typedef DAVA::Map<const DAVA::FilePath, CacheEntity> CacheMap;
    typedef QMap< DAVA::FilePath, QPointer< CacheRequest > > RequestPool;
    
public:
    
    TextureCache();
    ~TextureCache();

    static const int THUMBNAIL_SIZE = 64;
    
	DAVA::uint32 getThumbnailSize(const DAVA::TextureDescriptor *descriptor);
    DAVA::uint32 getOriginalSize(const DAVA::TextureDescriptor *descriptor);
    DAVA::uint32 getOriginalFileSize(const DAVA::TextureDescriptor *descriptor);
	QSize getOriginalImageSize(const DAVA::TextureDescriptor *descriptor) const;
    DAVA::uint32 getConvertedSize(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu);
    DAVA::uint32 getConvertedFileSize(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu);
	QSize getConvertedImageSize(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu) const;

    QList<QImage> getThumbnail(const DAVA::TextureDescriptor *descriptor);
	QList<QImage> getOriginal(const DAVA::TextureDescriptor *descriptor);
	QList<QImage> getConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu);

    void getThumbnail( const DAVA::TextureDescriptor *descriptor, QObject *object, const QString& slotName, const QVariant& userData = QVariant() );

	void clearInsteadThumbnails();
	void clearConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu);
	void clearOriginal(const DAVA::TextureDescriptor *descriptor);
	void clearThumbnail(const DAVA::TextureDescriptor *descriptor);

signals:
    void ThumbnailLoaded(const DAVA::TextureDescriptor *descriptor, const TextureInfo & image);
    void OriginalLoaded(const DAVA::TextureDescriptor *descriptor, const TextureInfo & image);
    void ConvertedLoaded(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu, const TextureInfo & image);
    
protected slots:
    
    void ClearCache();
    
	void ReadyThumbnail(const DAVA::TextureDescriptor *descriptor, const TextureInfo & image);
    void ReadyOriginal(const DAVA::TextureDescriptor *descriptor, const TextureInfo & image);
	void ReadyConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu, const TextureInfo & image);

protected:
    
    void setThumbnail(const DAVA::TextureDescriptor *descriptor, const TextureInfo & images);
	void setOriginal(const DAVA::TextureDescriptor *descriptor, const TextureInfo & images);
	void setConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu, const TextureInfo & images);

    void ClearCacheTail(DAVA::Map<const DAVA::FilePath, CacheEntity> & cache, const size_t currentWeight, const size_t maxWeight);
    void RemoveFromCache(DAVA::Map<const DAVA::FilePath, CacheEntity> & cache, const DAVA::TextureDescriptor *descriptor);
    
private:

	size_t curThumbnailWeight;
	size_t curOriginalWeight;
	size_t curConvertedWeight[DAVA::GPU_DEVICE_COUNT];

    static const size_t maxThumbnailCount = 100;
	static const size_t maxOrigCount = 1;
	static const size_t maxConvertedCount = 1; // per gpu

    RequestPool poolThumbnail;

    CacheMap cacheThumbnail;
	CacheMap cacheOriginal;
	CacheMap cacheConverted[DAVA::GPU_DEVICE_COUNT];
};

#endif // __TEXTURE_CACHE_H__
