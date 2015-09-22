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


#include "Render/2D/Sprite.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Utils/StringFormat.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Core/Core.h"
#include "Render/Shader.h"
#include "Render/RenderHelper.h"
#include "FileSystem/LocalizationSystem.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "Render/TextureDescriptor.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/Image/ImageConvert.h"

#define NEW_PPA

namespace DAVA
{
#ifdef USE_FILEPATH_IN_MAP
    using SpriteMap = Map<FilePath, Sprite *>;
#else //#ifdef USE_FILEPATH_IN_MAP
    using SpriteMap = Map<String, Sprite *>;
#endif //#ifdef USE_FILEPATH_IN_MAP
    SpriteMap spriteMap;

static int32 fboCounter = 0;

Mutex Sprite::spriteMapMutex;

namespace SpriteUtils
{
ScopedPtr<Image> PrepareImageToCreateSprite(Image* srcImage)
{
    if (srcImage->GetPixelFormat() == PixelFormat::FORMAT_RGB888)
    {
        ScopedPtr<Image> image8888(Image::Create(srcImage->GetWidth(), srcImage->GetHeight(), FORMAT_RGBA8888));
        ImageConvert::ConvertImageDirect(srcImage, image8888.get());
        return image8888;
    }
    return ScopedPtr<Image>(SafeRetain(srcImage));
}
}

Sprite::DrawState::DrawState()
{
    Reset();
    
    material = RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL;    
}

Sprite::Sprite()
{
	textures = 0;
	textureNames = 0;
	frameTextureIndex = 0;
	textureCount = 0;

	frameVertices = 0;
	texCoords = 0;
	rectsAndOffsets = 0;

	size.dx = 24;
	size.dy = 24;
	frameCount = 0;

    isPreparedForTiling = false;
    textureInVirtualSpace = false;

	modification = 0;
	flags = 0;
	resourceSizeIndex = 0;

	clipPolygon = 0;

	defaultPivotPoint = Vector2(0.0f, 0.0f);
}

Sprite* Sprite::PureCreate(const FilePath & spriteName, Sprite* forPointer)
{
	if(spriteName.IsEmpty() || spriteName.GetType() == FilePath::PATH_IN_MEMORY)
		return NULL;

	Sprite *cachedSprite = GetSpriteFromMap(spriteName);
	if(cachedSprite)
	{
		return cachedSprite;
	}

    int32 resourceSizeIndex = 0;
    File* spriteFile = GetSpriteFile(spriteName, resourceSizeIndex);
    if (!spriteFile)
    {
        return NULL;
    }

    Sprite * spr = forPointer;
	if (!spr)
	{
		spr = new Sprite();
	}

	spr->resourceSizeIndex = resourceSizeIndex;
    spr->relativePathname = spriteName;

    spr->InitFromFile(spriteFile);
	SafeRelease(spriteFile);

    spriteMapMutex.Lock();
	spriteMap[FILEPATH_MAP_KEY(spr->relativePathname)] = spr;
    spriteMapMutex.Unlock();

	spr->Reset();
	return spr;
}

Sprite* Sprite::GetSpriteFromMap(const FilePath &pathname)
{
    Sprite * ret = NULL;

    spriteMapMutex.Lock();

	SpriteMap::iterator it = spriteMap.find(FILEPATH_MAP_KEY(pathname));
	if (it != spriteMap.end())
	{
		Sprite *spr = it->second;
		spr->Retain();
		ret = spr;
	}
    spriteMapMutex.Unlock();

	return ret;
}

FilePath Sprite::GetScaledName(const FilePath &spriteName)
{
    String pathname;
    if(FilePath::PATH_IN_RESOURCES == spriteName.GetType())
        pathname = spriteName.GetFrameworkPath();//as we can have several res folders we should work with 'FrameworkPath' instead of 'AbsolutePathname'
    else
        pathname = spriteName.GetAbsolutePathname();

    VirtualCoordinatesSystem * virtualCoordsSystem = VirtualCoordinatesSystem::Instance();
    const String baseGfxFolderName = virtualCoordsSystem->GetResourceFolder(virtualCoordsSystem->GetBaseResourceIndex());
    String::size_type pos = pathname.find(baseGfxFolderName);
	if(String::npos != pos)
	{
        const String &desirableGfxFolderName = virtualCoordsSystem->GetResourceFolder(virtualCoordsSystem->GetDesirableResourceIndex());
        pathname.replace(pos, baseGfxFolderName.length(), desirableGfxFolderName);
		return pathname;
	}

	return spriteName;
}

File * Sprite::LoadLocalizedFile(const FilePath & spritePathname, FilePath & texturePath)
{
	FilePath localizedScaledPath(spritePathname);
	localizedScaledPath.ReplaceDirectory(spritePathname.GetDirectory() + (LocalizationSystem::Instance()->GetCurrentLocale() + "/"));

	texturePath = FilePath();
	File * fp = File::Create(localizedScaledPath, File::READ|File::OPEN);
	if(fp)
	{
		texturePath = localizedScaledPath;
	}
	else
	{
		fp = File::Create(spritePathname, File::READ|File::OPEN);
		if(fp)
		{
			texturePath = spritePathname;
		}
	}

	return fp;
}

void Sprite::InitFromFile(File *file)
{
	bool usedForScale = false;//Думаю, после исправлений в конвертере, эта магия больше не нужна. Но переменную пока оставлю.

	type = SPRITE_FROM_FILE;
    const FilePath& pathName = file->GetFilename();

	char tempBuf[1024];
	file->ReadLine(tempBuf, 1024);
	sscanf(tempBuf, "%d", &textureCount);
	textures = new Texture*[textureCount];
	textureNames = new FilePath[textureCount];

	char textureCharName[128];
	for (int32 k = 0; k < textureCount; ++k)
	{
		file->ReadLine(tempBuf, 1024);
		sscanf(tempBuf, "%s", textureCharName);

		FilePath tp = pathName.GetDirectory() + String(textureCharName);
        Texture* testTexture = Texture::CreateFromFile(tp);
		textures[k] = testTexture;
		textureNames[k] = tp;
		DVASSERT_MSG(textures[k], "ERROR: Texture loading failed"/* + pathName*/);
	}
	
	RegisterTextureStates();

	int32 width, height;
	file->ReadLine(tempBuf, 1024);
	sscanf(tempBuf, "%d %d", &width, &height);
    size = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtual(Vector2((float32)width, (float32)height), resourceSizeIndex);
    
	file->ReadLine(tempBuf, 1024);
	sscanf(tempBuf, "%d", &frameCount);

	texCoords = new float32*[frameCount];
	frameVertices = new float32*[frameCount];
	rectsAndOffsets = new float32*[frameCount];
	frameTextureIndex = new int32[frameCount];

	frameNames.resize(frameCount);
	for (int32 i = 0; i < frameCount; i++)
	{
		frameVertices[i] = new float32[8];
		texCoords[i] = new float32[8];
		rectsAndOffsets[i] = new float32[6];
    	char frameName[128] = {0};
    	
		int32 x, y, dx, dy, xOff, yOff;

		file->ReadLine(tempBuf, 1024);
		sscanf(tempBuf, "%d %d %d %d %d %d %d %s", &x, &y, &dx, &dy, &xOff, &yOff, &frameTextureIndex[i], frameName);
		frameNames[i] = (*frameName == '\0') ? FastName() : FastName(frameName);
        
        Rect rect = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtual(Rect((float32)xOff, (float32)yOff, (float32)dx, (float32)dy), resourceSizeIndex);

		rectsAndOffsets[i][0] = (float32)x;
        rectsAndOffsets[i][1] = (float32)y;
        rectsAndOffsets[i][2] = rect.dx;
        rectsAndOffsets[i][3] = rect.dy;
        rectsAndOffsets[i][4] = rect.x;
        rectsAndOffsets[i][5] = rect.y;
        
        frameVertices[i][0] = rect.x;
        frameVertices[i][1] = rect.y;
        frameVertices[i][2] = rect.x + rect.dx;
        frameVertices[i][3] = rect.y;
        frameVertices[i][4] = rect.x;
        frameVertices[i][5] = rect.y + rect.dy;
        frameVertices[i][6] = rect.x + rect.dx;
        frameVertices[i][7] = rect.y + rect.dy;

		float32 xof = 0;
		float32 yof = 0;
		if (usedForScale)
		{
			xof = 0.15f + (0.45f - 0.15f) * (dx * 0.01f);
			yof = 0.15f + (0.45f - 0.15f) * (dy * 0.01f);
			if(xof > 0.45f)
			{
				xof = 0.45f;
			}
			if(yof > 0.45f)
			{
				yof = 0.45f;
			}
		}

		dx += x;
		dy += y;

		texCoords[i][0] = ((float32)x + xof) / textures[frameTextureIndex[i]]->width;
        texCoords[i][1] = ((float32)y + yof) / textures[frameTextureIndex[i]]->height;
        texCoords[i][2] = ((float32)dx - xof) / textures[frameTextureIndex[i]]->width;
        texCoords[i][3] = ((float32)y + yof) / textures[frameTextureIndex[i]]->height;
        texCoords[i][4] = ((float32)x + xof) / textures[frameTextureIndex[i]]->width;
        texCoords[i][5] = ((float32)dy - yof) / textures[frameTextureIndex[i]]->height;
        texCoords[i][6] = ((float32)dx - xof) / textures[frameTextureIndex[i]]->width;
        texCoords[i][7] = ((float32)dy - yof) / textures[frameTextureIndex[i]]->height;
	}
	defaultPivotPoint.x = 0;
	defaultPivotPoint.y = 0;
}


Sprite* Sprite::Create(const FilePath &spriteName)
{
	Sprite * spr = PureCreate(spriteName, NULL);
	if (!spr)
	{
		Texture *pinkTexture = Texture::CreatePink();
		spr = CreateFromTexture(pinkTexture, 0, 0, 16, 16, 16.f, 16.f, spriteName);
		spr->type = SPRITE_FROM_FILE;

		pinkTexture->Release();
	}
	return spr;
}

Sprite* Sprite::CreateFromTexture(Texture *fromTexture, int32 xOffset, int32 yOffset, float32 sprWidth, float32 sprHeight, bool contentScaleIncluded)
{
	DVASSERT(fromTexture);
	Sprite *spr = new Sprite();
	DVASSERT_MSG(spr, "Render Target Sprite Creation failed");
	spr->InitFromTexture(fromTexture, xOffset, yOffset, sprWidth, sprHeight, -1, -1, contentScaleIncluded);
	return spr;
}

Sprite * Sprite::CreateFromTexture(Texture *fromTexture, int32 textureRegionOffsetX, int32 textureRegionOffsetY, int32 textureRegionWidth, int32 textureRegionHeigth, float32 sprWidth, float32 sprHeight, const FilePath &spriteName /* = FilePath()*/)
{
	DVASSERT(fromTexture);
	Sprite *spr = new Sprite();
	DVASSERT_MSG(spr, "Render Target Sprite Creation failed");
    spr->InitFromTexture(fromTexture, textureRegionOffsetX, textureRegionOffsetY, sprWidth, sprHeight, textureRegionWidth, textureRegionHeigth, false, spriteName);
	return spr;
}

Sprite* Sprite::CreateFromImage(Image* image, bool contentScaleIncluded /* = false*/, bool inVirtualSpace /* = false */)
{
    ScopedPtr<Image> srcImage = SpriteUtils::PrepareImageToCreateSprite(image);
    image = srcImage.get();

    uint32 width = image->GetWidth();
    uint32 height = image->GetHeight();

    ScopedPtr<Image> squareImage(ImageSystem::Instance()->EnsurePowerOf2Image(image));
    ScopedPtr<Texture> texture(Texture::CreateFromData(squareImage, false));

    Sprite* sprite = nullptr;
    if (texture)
    {
        Vector2 sprSize((float32)width, (float32)height);
        if(inVirtualSpace)
        {
            sprSize = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtual(sprSize);
        }
        
        sprite = Sprite::CreateFromTexture(texture, 0, 0, sprSize.x, sprSize.y, contentScaleIncluded);
        
        if(inVirtualSpace)
        {
            sprite->ConvertToVirtualSize();
        }
    }

    return sprite;
}

Sprite* Sprite::CreateFromSourceData(const uint8* data, uint32 size, bool contentScaleIncluded /* = false*/, bool inVirtualSpace /* = false */)
{
    if (data == nullptr || size == 0)
    {
        return nullptr;
    }

    ScopedPtr<DynamicMemoryFile> file(DynamicMemoryFile::Create(data, size, File::OPEN | File::READ));
    DVASSERT(file);

    Vector<Image*> images;
    ImageSystem::Instance()->Load(file, images);
    if (images.size() == 0)
    {
        return nullptr;
    }

    Sprite* sprite = CreateFromImage(images[0], contentScaleIncluded, inVirtualSpace);
    
    for_each(images.begin(), images.end(), SafeRelease<Image>);

    return sprite;
}

String Sprite::GetPathString( const Sprite *sprite )
{
    if (nullptr == sprite)
        return String();

    FilePath path(sprite->GetRelativePathname());

    String pathName;
    if (!path.IsEmpty())
    {
        path.TruncateExtension();
        pathName = path.GetFrameworkPath();
    }
    return pathName;
}

Sprite* Sprite::CreateFromSourceFile(const FilePath& path, bool contentScaleIncluded /* = false*/, bool inVirtualSpace /* = false */)
{
    Sprite* sprite = GetSpriteFromMap(path);
    if (sprite != nullptr)
    {
        return sprite;
    }
    
    Vector<Image*> images;
    ImageSystem::Instance()->Load(path, images);
    if (images.size() == 0)
    {
        return nullptr;
    }

    sprite = CreateFromImage(images[0], contentScaleIncluded, inVirtualSpace);
    if (sprite)
    {
        sprite->SetRelativePathname(path);
    }

    for_each(images.begin(), images.end(), SafeRelease<Image>);

    return sprite;
}

void Sprite::InitFromTexture(Texture *fromTexture, int32 xOffset, int32 yOffset, float32 sprWidth, float32 sprHeight, int32 targetWidth, int32 targetHeight, bool contentScaleIncluded, const FilePath &spriteName /* = FilePath() */)
{
    Clear();
    
    Vector2 offset((float32)xOffset, (float32)yOffset);
    size = Vector2(sprWidth, sprHeight);
	if (!contentScaleIncluded)
	{
        offset = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(offset);
	}
	else
	{
        size = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtual(size);
	}

	resourceSizeIndex = VirtualCoordinatesSystem::Instance()->GetBaseResourceIndex();

    type = SPRITE_FROM_TEXTURE;
    textureCount = 1;
    textures = new Texture*[textureCount];
    textureNames = new FilePath[textureCount];
    textureInVirtualSpace = contentScaleIncluded;


    textures[0] = SafeRetain(fromTexture);
    if(textures[0])
    {
        textureNames[0] = textures[0]->GetPathname();
    }

    defaultPivotPoint.x = 0;
	defaultPivotPoint.y = 0;
	frameCount = 1;

    texCoords = new float32*[frameCount];
    frameVertices = new float32*[frameCount];
    rectsAndOffsets = new float32*[frameCount];
    frameTextureIndex = new int32[frameCount];

    for (int i = 0;	i < frameCount; i++)
    {
        frameVertices[i] = new float32[8];
        texCoords[i] = new float32[8];
        rectsAndOffsets[i] = new float32[6];
        frameTextureIndex[i] = 0;

		float32 x, y, dx,dy, xOff, yOff;
		x = offset.x;
		y = offset.y;
        dx = (targetWidth == -1) ? VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(size.x) : (float32)targetWidth;
        dy = (targetHeight == -1) ? VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY(size.y) : (float32)targetHeight;
		xOff = 0;
		yOff = 0;

        float32* rectAndOffset = rectsAndOffsets[i];
        rectAndOffset[0] = x;
        rectAndOffset[1] = y;
        rectAndOffset[2] = size.x;
        rectAndOffset[3] = size.y;
        rectAndOffset[4] = xOff;
        rectAndOffset[5] = yOff;


        float32* frameVerts = frameVertices[i];
        frameVerts[0] = xOff;
        frameVerts[1] = yOff;
        frameVerts[2] = xOff + size.x;
        frameVerts[3] = yOff;
        frameVerts[4] = xOff;
        frameVerts[5] = (yOff + size.y);
        frameVerts[6] = (xOff + size.x);
        frameVerts[7] = (yOff + size.y);

        dx += x;
        dy += y;

        int32 frameIndex = frameTextureIndex[i];
        Texture* texture = textures[frameIndex];
        float32* texCoord = texCoords[i];
        
        texCoord[0] = x / texture->width;
        texCoord[1] = y / texture->height;
        texCoord[2] = dx / texture->width;
        texCoord[3] = y / texture->height;
        texCoord[4] = x / texture->width;
        texCoord[5] = dy / texture->height;
        texCoord[6] = dx / texture->width;
        texCoord[7] = dy / texture->height;
	}

	// DF-1984 - Set available sprite relative path name here. Use FBO sprite name only if sprite name is empty.
    if (relativePathname.IsEmpty())
    {
        relativePathname = spriteName.IsEmpty() ? Format("FBO sprite %d", fboCounter) : spriteName;
    }

    spriteMapMutex.Lock();
    spriteMap[FILEPATH_MAP_KEY(relativePathname)] = this;
    spriteMapMutex.Unlock();

    fboCounter++;
    Reset();

    RegisterTextureStates();
}

void Sprite::SetOffsetsForFrame(int frame, float32 xOff, float32 yOff)
{
	DVASSERT(frame < frameCount);

	rectsAndOffsets[frame][4] = xOff;
	rectsAndOffsets[frame][5] = yOff;

	frameVertices[frame][0] = xOff;
	frameVertices[frame][1] = yOff;
	frameVertices[frame][2] = xOff + rectsAndOffsets[frame][2];
	frameVertices[frame][3] = yOff;
	frameVertices[frame][4] = xOff;
	frameVertices[frame][5] = yOff + rectsAndOffsets[frame][3];
	frameVertices[frame][6] = xOff + rectsAndOffsets[frame][2];
	frameVertices[frame][7] = yOff + rectsAndOffsets[frame][3];
}

void Sprite::Clear()
{
	UnregisterTextureStates();
	for (int32 k = 0; k < textureCount; ++k)
	{
		SafeRelease(textures[k]);
	}
	SafeDeleteArray(textures);
	SafeDeleteArray(textureNames);

	if (frameVertices != 0)
	{
		for (int i = 0;	i < frameCount; i++)
		{
			SafeDeleteArray(frameVertices[i]);
			SafeDeleteArray(texCoords[i]);
			SafeDeleteArray(rectsAndOffsets[i]);
		}
	}
    
	SafeDeleteArray(frameVertices);
	SafeDeleteArray(texCoords);
	SafeDeleteArray(rectsAndOffsets);
	SafeDeleteArray(frameTextureIndex);
	textureCount = 0;
}

Sprite::~Sprite()
{
    spriteMapMutex.Lock();
    spriteMap.erase(FILEPATH_MAP_KEY(relativePathname));
    spriteMapMutex.Unlock();
	Clear();
}

Texture* Sprite::GetTexture() const
{
	return textures[0];
}

Texture* Sprite::GetTexture(int32 frameNumber) const
{
	frameNumber = Clamp(frameNumber, 0, frameCount - 1);
	return textures[frameTextureIndex[frameNumber]];
}
	
rhi::HTextureSet Sprite::GetTextureHandle(int32 frameNumber) const
{
	frameNumber = Clamp(frameNumber, 0, frameCount - 1);
	return textureHandles[frameTextureIndex[frameNumber]];
}

float32 *Sprite::GetTextureVerts(int32 frameNumber)
{
	frameNumber = Clamp(frameNumber, 0, frameCount - 1);
	return texCoords[frameNumber];
}

int32 Sprite::GetFrameCount() const
{
	return frameCount;
}

float32 Sprite::GetWidth() const
{
	return size.dx;
}

float32 Sprite::GetHeight() const
{
	return size.dy;
}

const Vector2 &Sprite::GetSize() const
{
	return size;
}

const Vector2 &Sprite::GetDefaultPivotPoint() const
{
	return defaultPivotPoint;
}

void Sprite::SetDefaultPivotPoint(float32 x, float32 y)
{
    defaultPivotPoint.x = x;
    defaultPivotPoint.y = y;
}
    
void Sprite::SetDefaultPivotPoint(const Vector2 &newPivotPoint)
{
    defaultPivotPoint = newPivotPoint;
}

int32 Sprite::GetFrameByName(const FastName& frameName) const
{
	if (!frameName.IsValid())
    {
		return INVALID_FRAME_INDEX;
    }
    
    for (int32 i = 0; i < frameCount; i++)
	{
    	if (frameNames[i] == frameName)
        	return i;
    }
    
    return INVALID_FRAME_INDEX;
}

void Sprite::SetModification(int32 modif)
{
	modification = modif;
	if(modif != 0)
	{
		flags = flags | EST_MODIFICATION;
	}
	else
	{
		ResetModification();
	}
}

void Sprite::Reset()
{
	flags = 0;
	modification = 0;
	clipPolygon = 0;
}

void Sprite::ResetModification()
{
    flags = flags & ~EST_MODIFICATION;
}

float32 Sprite::GetRectOffsetValueForFrame(int32 frame, eRectsAndOffsets valueType) const
{
	int32 clampedFrame = Clamp(frame, 0, frameCount - 1);
	return rectsAndOffsets[clampedFrame][valueType];
}

const float32 * Sprite::GetFrameVerticesForFrame( int32 frame ) const
{
	int32 clampedFrame = Clamp(frame, 0, frameCount - 1);
	return frameVertices[clampedFrame];
}

const float32 * Sprite::GetTextureCoordsForFrame( int32 frame ) const
{
	int32 clampedFrame = Clamp(frame, 0, frameCount - 1);
	return texCoords[clampedFrame];
}

void Sprite::PrepareForNewSize()
{
    if(relativePathname.IsEmpty()) return;
    
	String pathname = relativePathname.GetAbsolutePathname();

	int pos = (int)pathname.find(VirtualCoordinatesSystem::Instance()->GetResourceFolder(VirtualCoordinatesSystem::Instance()->GetBaseResourceIndex()));
	String scaledName = pathname.substr(0, pos) + VirtualCoordinatesSystem::Instance()->GetResourceFolder(VirtualCoordinatesSystem::Instance()->GetDesirableResourceIndex()) + pathname.substr(pos + VirtualCoordinatesSystem::Instance()->GetResourceFolder(VirtualCoordinatesSystem::Instance()->GetBaseResourceIndex()).length());

	Logger::FrameworkDebug("Seraching for file: %s", scaledName.c_str());


	File *fp = File::Create(scaledName, File::READ|File::OPEN);

	if (!fp)
	{
		Logger::FrameworkDebug("Can't find file: %s", scaledName.c_str());
		return;
	}
	SafeRelease(fp);

	Vector2 tempPivotPoint = defaultPivotPoint;

	Clear();
    
    spriteMapMutex.Lock();
	spriteMap.erase(FILEPATH_MAP_KEY(relativePathname));
    spriteMapMutex.Unlock();

	textures = 0;
	textureNames = 0;

	frameTextureIndex = 0;
	textureCount = 0;

	frameVertices = 0;
	texCoords = 0;
	rectsAndOffsets = 0;

	size.dx = 24;
	size.dy = 24;
	frameCount = 0;

	modification = 0;
	flags = 0;
	resourceSizeIndex = 0;

	clipPolygon = 0;
    
	PureCreate(pathname.substr(0, pathname.length() - 4), this);
//TODO: следующая строка кода написада здесь только до тех времен
//		пока defaultPivotPoint не начнет задаваться прямо в спрайте,
//		но возможно это навсегда.
	defaultPivotPoint = tempPivotPoint;
}

void Sprite::ValidateForSize()
{
	Logger::FrameworkDebug("--------------- Sprites validation for new resolution ----------------");
	List<Sprite*> spritesToReload;

    spriteMapMutex.Lock();
	for(SpriteMap::iterator it = spriteMap.begin(); it != spriteMap.end(); ++it)
	{
		Sprite *sp = it->second;
		if (sp->type == SPRITE_FROM_FILE && VirtualCoordinatesSystem::Instance()->GetDesirableResourceIndex() != sp->GetResourceSizeIndex())
		{
			spritesToReload.push_back(sp);
		}
	}
    spriteMapMutex.Unlock();

	for(List<Sprite*>::iterator it = spritesToReload.begin(); it != spritesToReload.end(); ++it)
	{
		(*it)->PrepareForNewSize();
	}
	Logger::FrameworkDebug("----------- Sprites validation for new resolution DONE  --------------");
//	Texture::DumpTextures();
}


void Sprite::DumpSprites()
{
	Logger::FrameworkDebug("============================================================");
	Logger::FrameworkDebug("--------------- Currently allocated sprites ----------------");

    spriteMapMutex.Lock();
    uint32 spritesCount = static_cast<uint32>(spriteMap.size()); 
	for(SpriteMap::iterator it = spriteMap.begin(); it != spriteMap.end(); ++it)
	{
		Sprite *sp = it->second; //[spriteDict objectForKey:[txKeys objectAtIndex:i]];
		Logger::FrameworkDebug("name:%s count:%d size(%.0f x %.0f)", sp->relativePathname.GetAbsolutePathname().c_str(), sp->GetRetainCount(), sp->size.dx, sp->size.dy);
	}
    spriteMapMutex.Unlock();

    Logger::FrameworkDebug("Total spritesCount: %d", spritesCount);
    Logger::FrameworkDebug("============================================================");
}

void Sprite::SetClipPolygon(Polygon2 * _clipPolygon)
{
	clipPolygon = _clipPolygon;
}

void Sprite::ConvertToVirtualSize()
{
    frameVertices[0][0] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualX(frameVertices[0][0], resourceSizeIndex);
    frameVertices[0][1] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualY(frameVertices[0][1], resourceSizeIndex);
    frameVertices[0][2] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualX(frameVertices[0][2], resourceSizeIndex);
    frameVertices[0][3] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualY(frameVertices[0][3], resourceSizeIndex);
    frameVertices[0][4] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualX(frameVertices[0][4], resourceSizeIndex);
    frameVertices[0][5] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualY(frameVertices[0][5], resourceSizeIndex);
    frameVertices[0][6] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualX(frameVertices[0][6], resourceSizeIndex);
    frameVertices[0][7] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualY(frameVertices[0][7], resourceSizeIndex);

    frameVertices[0][0] = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(frameVertices[0][0]);
    frameVertices[0][1] = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY(frameVertices[0][1]);
    frameVertices[0][2] = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(frameVertices[0][2]);
    frameVertices[0][3] = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY(frameVertices[0][3]);
    frameVertices[0][4] = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(frameVertices[0][4]);
    frameVertices[0][5] = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY(frameVertices[0][5]);
    frameVertices[0][6] = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(frameVertices[0][6]);
    frameVertices[0][7] = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY(frameVertices[0][7]);

    texCoords[0][0] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualX(texCoords[0][0], resourceSizeIndex);
    texCoords[0][1] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualY(texCoords[0][1], resourceSizeIndex);
    texCoords[0][2] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualX(texCoords[0][2], resourceSizeIndex);
    texCoords[0][3] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualY(texCoords[0][3], resourceSizeIndex);
    texCoords[0][4] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualX(texCoords[0][4], resourceSizeIndex);
    texCoords[0][5] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualY(texCoords[0][5], resourceSizeIndex);
    texCoords[0][6] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualX(texCoords[0][6], resourceSizeIndex);
    texCoords[0][7] = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualY(texCoords[0][7], resourceSizeIndex);
}

const FilePath & Sprite::GetRelativePathname() const
{
	return relativePathname;
}

void Sprite::DrawState::BuildStateFromParentAndLocal(const Sprite::DrawState &parentState, const Sprite::DrawState &localState)
{
	position.x = parentState.position.x + localState.position.x * parentState.scale.x;
	position.y = parentState.position.y + localState.position.y * parentState.scale.y;
	if(parentState.angle != 0)
	{
		float tmpX = position.x;
		position.x = (tmpX - parentState.position.x) * parentState.cosA  + (parentState.position.y - position.y) * parentState.sinA + parentState.position.x;
		position.y = (tmpX - parentState.position.x) * parentState.sinA  + (position.y - parentState.position.y) * parentState.cosA + parentState.position.y;
	}
	scale.x = localState.scale.x * parentState.scale.x;
	scale.y = localState.scale.y * parentState.scale.y;
	angle = localState.angle + parentState.angle;
	if(angle != precomputedAngle)	// compute precomputed angle and store values
	{
		precomputedAngle = angle;
		if(precomputedAngle != parentState.angle)
		{
			cosA = cosf(precomputedAngle);
			sinA = sinf(precomputedAngle);
		}
		else
		{
			cosA = parentState.cosA;
			sinA = parentState.sinA;
		}
	}
	pivotPoint.x = localState.pivotPoint.x;
	pivotPoint.y = localState.pivotPoint.y;

	frame = localState.frame;
}

void Sprite::ReloadSprites()
{
	for(SpriteMap::iterator it = spriteMap.begin(); it != spriteMap.end(); ++it)
	{
        (it->second)->Reload();
	}
}

void Sprite::Reload()
{
	if(type == SPRITE_FROM_FILE)
	{
        ReloadExistingTextures();
        Clear();

        File *fp = GetSpriteFile(relativePathname, resourceSizeIndex);
		if(fp)
		{
			InitFromFile(fp);
			SafeRelease(fp);
		}
		else
		{
			Logger::Warning("Unable to reload sprite %s", relativePathname.GetAbsolutePathname().c_str());

			Texture *pinkTexture = Texture::CreatePink();
			InitFromTexture(pinkTexture, 0, 0, 16.0f, 16.0f, 16, 16, false, relativePathname);
			pinkTexture->Release();

			type = SPRITE_FROM_FILE;
		}
	}
}
    
File* Sprite::GetSpriteFile(const FilePath & spriteName, int32& resourceSizeIndex)
{
    FilePath pathName = FilePath::CreateWithNewExtension(spriteName, ".txt");
    FilePath scaledPath = GetScaledName(pathName);

    FilePath texturePath;
    File * fp = LoadLocalizedFile(scaledPath, texturePath);
    if (!fp)
    {
        fp = LoadLocalizedFile(pathName, texturePath);
        if(!fp)
        {
            Logger::Instance()->Warning("Failed to open sprite file: %s", pathName.GetAbsolutePathname().c_str());
            return NULL;
        }

        resourceSizeIndex = VirtualCoordinatesSystem::Instance()->GetBaseResourceIndex();
    }
    else
    {
        resourceSizeIndex = VirtualCoordinatesSystem::Instance()->GetDesirableResourceIndex();
    }

    return fp;
}

void Sprite::RegisterTextureStates()
{
	textureHandles.resize(textureCount);
	for(int32 i = 0; i < textureCount; ++i)
    {
        if(textures[i])
        {
            rhi::TextureSetDescriptor descriptor;
            descriptor.fragmentTextureCount = 1;
            descriptor.fragmentTexture[0] = textures[i]->handle;            			
            textureHandles[i] = rhi::AcquireTextureSet(descriptor);
		}
	}
}

void Sprite::UnregisterTextureStates()
{

	for(int32 i = 0; i < textureCount; ++i)
    {
		if(textureHandles[i] != rhi::InvalidHandle)
		{
            rhi::ReleaseTextureSet(textureHandles[i]);			
		}
	}

}

void Sprite::ReloadExistingTextures()
{
    //this function need to be sure that textures really would reload
    for(int32 i = 0; i < textureCount; ++i)
    {
        if(textures[i] && !textures[i]->GetPathname().IsEmpty())
        {
            if(textures[i]->GetPathname().Exists())
            {
                textures[i]->Reload();
            }
        }
        else
        {
            Logger::Error("[Sprite::ReloadSpriteTextures] Something strange with texture_%d", i);
        }
    }
}
    
void Sprite::SetRelativePathname(const FilePath& path)
{
    spriteMapMutex.Lock();
    spriteMap.erase(FILEPATH_MAP_KEY(relativePathname));
    relativePathname = path;
    spriteMap[FILEPATH_MAP_KEY(this->relativePathname)] = this;
    spriteMapMutex.Unlock();
    GetTexture()->SetPathname(path);
}

};
