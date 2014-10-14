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
#include "Render/RenderManager.h"
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
#include "Render/2D/RenderSystem2D/VirtualCoordinatesSystem.h"

#define NEW_PPA

namespace DAVA
{
#ifdef USE_FILEPATH_IN_MAP
	typedef Map<FilePath, Sprite *> SpriteMap;
#else //#ifdef USE_FILEPATH_IN_MAP
	typedef Map<String, Sprite *> SpriteMap;
#endif //#ifdef USE_FILEPATH_IN_MAP
	SpriteMap spriteMap;

static int32 fboCounter = 0;
//Vector<Vector2> Sprite::clippedTexCoords;
//Vector<Vector2> Sprite::clippedVertices;

Mutex Sprite::spriteMapMutex;

Sprite::DrawState::DrawState()
{
    Reset();
    
    renderState = RenderState::RENDERSTATE_2D_BLEND;
    shader = RenderManager::TEXTURE_MUL_FLAT_COLOR;
    //RenderManager::Instance()->RetainRenderState(renderState);
    //shader = SafeRetain(RenderManager::TEXTURE_MUL_FLAT_COLOR);
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
	//frame = 0;

	isPreparedForTiling = false;

	modification = 0;
	flags = 0;
	resourceSizeIndex = 0;

	clipPolygon = 0;

//	spriteRenderObject = new RenderDataObject();
//	vertexStream = spriteRenderObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 0, 0);
//	texCoordStream  = spriteRenderObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, 0);

	//pivotPoint = Vector2(0.0f, 0.0f);
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
    size = VirtualCoordinates::ConvertResourceToVirtual(Vector2((float32)width, (float32)height), resourceSizeIndex);
    
	file->ReadLine(tempBuf, 1024);
	sscanf(tempBuf, "%d", &frameCount);

	texCoords = new GLfloat*[frameCount];
	frameVertices = new GLfloat*[frameCount];
	rectsAndOffsets = new float32*[frameCount];
	frameTextureIndex = new int32[frameCount];


	for (int32 i = 0; i < frameCount; i++)
	{
		frameVertices[i] = new GLfloat[8];
		texCoords[i] = new GLfloat[8];
		rectsAndOffsets[i] = new GLfloat[6];

		int32 x, y, dx,dy, xOff, yOff;

		file->ReadLine(tempBuf, 1024);
		sscanf(tempBuf, "%d %d %d %d %d %d %d", &x, &y, &dx, &dy, &xOff, &yOff, &frameTextureIndex[i]);

		rectsAndOffsets[i][0] = (float32)x;
		rectsAndOffsets[i][1] = (float32)y;

        Vector2 vx1 = VirtualCoordinates::ConvertResourceToVirtual(Vector2((float32)xOff, (float32)yOff), resourceSizeIndex);
        Vector2 vx2 = VirtualCoordinates::ConvertResourceToVirtual(Vector2((float32)(xOff + dx), (float32)yOff), resourceSizeIndex);
        Vector2 vx3 = VirtualCoordinates::ConvertResourceToVirtual(Vector2((float32)xOff, (float32)yOff + dy), resourceSizeIndex);
        Vector2 vx4 = VirtualCoordinates::ConvertResourceToVirtual(Vector2((float32)(xOff + dx), (float32)(yOff + dy)), resourceSizeIndex);
        
		frameVertices[i][0] = vx1.x;
		frameVertices[i][1] = vx1.y;
		frameVertices[i][2] = vx2.x;
		frameVertices[i][3] = vx2.y;
		frameVertices[i][4] = vx3.x;
		frameVertices[i][5] = vx3.y;
		frameVertices[i][6] = vx4.x;
		frameVertices[i][7] = vx4.y;

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

        Rect rect = VirtualCoordinates::ConvertResourceToVirtual(Rect((float32)dx, (float32)dy, (float32)xOff, (float32)yOff), resourceSizeIndex);
		rectsAndOffsets[i][2] = rect.x;
		rectsAndOffsets[i][3] = rect.y;
		rectsAndOffsets[i][4] = rect.dx;
		rectsAndOffsets[i][5] = rect.dy;

		dx += x;
		dy += y;

		texCoords[i][0] = ((GLfloat)x + xof) / textures[frameTextureIndex[i]]->width;
		texCoords[i][1] = ((GLfloat)y + yof) / textures[frameTextureIndex[i]]->height;
		texCoords[i][2] = ((GLfloat)dx - xof) / textures[frameTextureIndex[i]]->width;
		texCoords[i][3] = ((GLfloat)y + yof) / textures[frameTextureIndex[i]]->height;
		texCoords[i][4] = ((GLfloat)x + xof) / textures[frameTextureIndex[i]]->width;
		texCoords[i][5] = ((GLfloat)dy - yof) / textures[frameTextureIndex[i]]->height;
		texCoords[i][6] = ((GLfloat)dx - xof) / textures[frameTextureIndex[i]]->width;
		texCoords[i][7] = ((GLfloat)dy - yof) / textures[frameTextureIndex[i]]->height;
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
		spr = CreateFromTexture(Vector2(16.f, 16.f), pinkTexture, Vector2(0.f, 0.f), Vector2(16.f, 16.f), spriteName);
		spr->type = SPRITE_FROM_FILE;

		pinkTexture->Release();
	}
	return spr;
}

Sprite* Sprite::CreateAsRenderTarget(float32 sprWidth, float32 sprHeight, PixelFormat textureFormat, bool contentScaleIncluded)
{
	Sprite * sprite = new Sprite();
	sprite->InitAsRenderTarget(sprWidth, sprHeight, textureFormat, contentScaleIncluded);
	return sprite;
}

void Sprite::InitAsRenderTarget(float32 sprWidth, float32 sprHeight, PixelFormat textureFormat, bool contentScaleIncluded)
{
    Vector2 spriteSize(sprWidth, sprHeight);
	if (!contentScaleIncluded)
	{
        spriteSize = VirtualCoordinates::ConvertVirtualToPhysical(spriteSize);
	}

	Texture *t = Texture::CreateFBO((int32)ceilf(spriteSize.x), (int32)ceilf(spriteSize.y), textureFormat, Texture::DEPTH_NONE);

	this->InitFromTexture(t, 0, 0, spriteSize.x, spriteSize.y, -1, -1, true);

	t->Release();

	this->type = SPRITE_RENDER_TARGET;

	// Clear created render target first
	RenderManager::Instance()->SetRenderTarget(this);
	RenderManager::Instance()->ClearWithColor(0, 0, 0, 0);
	RenderManager::Instance()->RestoreRenderTarget();
}

Sprite* Sprite::CreateFromTexture(Texture *fromTexture, int32 xOffset, int32 yOffset, float32 sprWidth, float32 sprHeight, bool contentScaleIncluded)
{
	DVASSERT(fromTexture);
	Sprite *spr = new Sprite();
	DVASSERT_MSG(spr, "Render Target Sprite Creation failed");
	spr->InitFromTexture(fromTexture, xOffset, yOffset, sprWidth, sprHeight, -1, -1, contentScaleIncluded);
	return spr;
}

Sprite * Sprite::CreateFromTexture(const Vector2 & spriteSize, Texture * fromTexture, const Vector2 & textureRegionOffset, const Vector2 & textureRegionSize, const FilePath &spriteName /* = FilePath()*/)
{
	DVASSERT(fromTexture);
	Sprite *spr = new Sprite();
	DVASSERT_MSG(spr, "Render Target Sprite Creation failed");
	spr->InitFromTexture(fromTexture, (int32)textureRegionOffset.x, (int32)textureRegionOffset.y, textureRegionSize.x, textureRegionSize.y, (int32)spriteSize.x, (int32)spriteSize.y, false, spriteName);
	return spr;
}

Sprite* Sprite::CreateFromImage(Image* image, bool contentScaleIncluded /* = false*/, bool inVirtualSpace /* = false */)
{
    uint32 width = image->GetWidth();
    uint32 height = image->GetHeight();
    
    int32 size = (int32)Max(width, height);
    
    Image *img = NULL;
    if(IsPowerOf2(width) && IsPowerOf2(height))
    {
        img = SafeRetain(image);
    }
    else
    {
        EnsurePowerOf2(size);

        img = Image::Create((uint32)size, (uint32)size, image->GetPixelFormat());
        img->InsertImage(image, 0, 0);
    }

    Texture* texture = Texture::CreateFromData(img, false);

    Sprite* sprite = NULL;
    if (texture)
    {
        Vector2 sprSize((float32)width, (float32)height);
        if(inVirtualSpace)
        {
            sprSize = VirtualCoordinates::ConvertPhysicalToVirtual(sprSize);
        }
        
        sprite = Sprite::CreateFromTexture(texture, 0, 0, sprSize.x, sprSize.y, contentScaleIncluded);
        
        if(inVirtualSpace)
        {
            sprite->ConvertToVirtualSize();
        }
    }

    SafeRelease(texture);
    SafeRelease(img);

    return sprite;
}

Sprite* Sprite::CreateFromSourceData(const uint8* data, uint32 size, bool contentScaleIncluded /* = false*/, bool inVirtualSpace /* = false */)
{
    if (data == NULL || size == 0)
    {
        return NULL;
    }

    DynamicMemoryFile* file = DynamicMemoryFile::Create(data, size, File::OPEN | File::READ);
    if (!file)
    {
        return NULL;
    }

    Vector<Image*> images;
    ImageSystem::Instance()->Load(file, images);
    if (images.size() == 0)
    {
        return NULL;
    }

    Sprite* sprite = CreateFromImage(images[0], contentScaleIncluded, inVirtualSpace);
    
    for_each(images.begin(), images.end(), SafeRelease<Image>);
    SafeRelease(file);

    return sprite;
}

String Sprite::GetPathString( const Sprite *sprite )
{
    if (!sprite)
    {
        return "";
    }

    FilePath path(sprite->GetRelativePathname());
    String pathName = "";
    if (!path.IsEmpty())
    {
        path.TruncateExtension();
        pathName = path.GetFrameworkPath();
    }

    return pathName;
}

Sprite* Sprite::CreateFromSourceFile(const FilePath& path, bool contentScaleIncluded /* = false*/, bool inVirtualSpace /* = false */)
{
    Vector<Image*> images;
    ImageSystem::Instance()->Load(path, images);
    if (images.size() == 0)
    {
        return NULL;
    }

    Sprite* sprite = CreateFromImage(images[0], contentScaleIncluded, inVirtualSpace);

    for_each(images.begin(), images.end(), SafeRelease<Image>);

    return sprite;
}

void Sprite::InitFromTexture(Texture *fromTexture, int32 xOffset, int32 yOffset, float32 sprWidth, float32 sprHeight, int32 targetWidth, int32 targetHeight, bool contentScaleIncluded, const FilePath &spriteName /* = FilePath() */)
{
    Vector2 offset((float32)xOffset, (float32)yOffset);
    size = Vector2(sprWidth, sprHeight);
	if (!contentScaleIncluded)
	{
        offset = VirtualCoordinates::ConvertVirtualToPhysical(offset);
	}
	else
	{
        size = VirtualCoordinates::ConvertPhysicalToVirtual(size);
	}

	resourceSizeIndex = VirtualCoordinatesSystem::Instance()->GetBaseResourceIndex();

	this->type = SPRITE_FROM_TEXTURE;
	this->textureCount = 1;
	this->textures = new Texture*[this->textureCount];
	this->textureNames = new FilePath[this->textureCount];


	this->textures[0] = SafeRetain(fromTexture);
	if(this->textures[0])
	{
		this->textureNames[0] = this->textures[0]->GetPathname();
	}

	this->defaultPivotPoint.x = 0;
	this->defaultPivotPoint.y = 0;
	this->frameCount = 1;

	this->texCoords = new GLfloat*[this->frameCount];
	this->frameVertices = new GLfloat*[this->frameCount];
	this->rectsAndOffsets = new GLfloat*[this->frameCount];
	this->frameTextureIndex = new int32[this->frameCount];

	for (int i = 0;	i < this->frameCount; i++)
	{
		this->frameVertices[i] = new GLfloat[8];
		this->texCoords[i] = new GLfloat[8];
		this->rectsAndOffsets[i] = new GLfloat[6];
		this->frameTextureIndex[i] = 0;

		float32 x, y, dx,dy, xOff, yOff;
		x = offset.x;
		y = offset.y;
		dx = size.x * VirtualCoordinates::GetVirtualToPhysicalFactor();
		dy = size.y * VirtualCoordinates::GetVirtualToPhysicalFactor();
		xOff = 0;
		yOff = 0;

		this->rectsAndOffsets[i][0] = (float32)x;
		this->rectsAndOffsets[i][1] = (float32)y;
		this->rectsAndOffsets[i][2] = size.x;
		this->rectsAndOffsets[i][3] = size.y;
		this->rectsAndOffsets[i][4] = (float32)xOff;
		this->rectsAndOffsets[i][5] = (float32)yOff;

		this->frameVertices[i][0] = (float32)xOff;
		this->frameVertices[i][1] = (float32)yOff;
		this->frameVertices[i][2] = (float32)xOff + size.x;
		this->frameVertices[i][3] = (float32)yOff;
		this->frameVertices[i][4] = (float32)xOff;
		this->frameVertices[i][5] = (float32)(yOff + size.y);
		this->frameVertices[i][6] = (float32)(xOff + size.x);
		this->frameVertices[i][7] = (float32)(yOff + size.y);


		dx += x;
		dy += y;

		this->texCoords[i][0] = (GLfloat)x / this->textures[this->frameTextureIndex[i]]->width;
		this->texCoords[i][1] = (GLfloat)y / this->textures[this->frameTextureIndex[i]]->height;
		this->texCoords[i][2] = (GLfloat)dx / this->textures[this->frameTextureIndex[i]]->width;
		this->texCoords[i][3] = (GLfloat)y / this->textures[this->frameTextureIndex[i]]->height;
		this->texCoords[i][4] = (GLfloat)x / this->textures[this->frameTextureIndex[i]]->width;
		this->texCoords[i][5] = (GLfloat)dy / this->textures[this->frameTextureIndex[i]]->height;
		this->texCoords[i][6] = (GLfloat)dx / this->textures[this->frameTextureIndex[i]]->width;
		this->texCoords[i][7] = (GLfloat)dy / this->textures[this->frameTextureIndex[i]]->height;

	}

	// DF-1984 - Set available sprite relative path name here. Use FBO sprite name only if sprite name is empty.
    if (this->relativePathname.IsEmpty())
        this->relativePathname = spriteName.IsEmpty() ? Format("FBO sprite %d", fboCounter) : spriteName;

    spriteMapMutex.Lock();
	spriteMap[FILEPATH_MAP_KEY(this->relativePathname)] = this;
    spriteMapMutex.Unlock();

	fboCounter++;
	this->Reset();
	
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
//    SafeRelease(spriteRenderObject);
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
	
UniqueHandle Sprite::GetTextureHandle(int32 frameNumber) const
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
	for(SpriteMap::iterator it = spriteMap.begin(); it != spriteMap.end(); ++it)
	{
		Sprite *sp = it->second; //[spriteDict objectForKey:[txKeys objectAtIndex:i]];
		Logger::FrameworkDebug("name:%s count:%d size(%.0f x %.0f)", sp->relativePathname.GetAbsolutePathname().c_str(), sp->GetRetainCount(), sp->size.dx, sp->size.dy);
	}
    spriteMapMutex.Unlock();

	Logger::FrameworkDebug("============================================================");
}

void Sprite::SetClipPolygon(Polygon2 * _clipPolygon)
{
	clipPolygon = _clipPolygon;
}

void Sprite::ConvertToVirtualSize()
{
	float32 virtualToPhysicalFactor = VirtualCoordinates::GetVirtualToPhysicalFactor();
	float32 resourceToVirtualFactor = VirtualCoordinates::GetResourceToVirtualFactor(GetResourceSizeIndex());

	frameVertices[0][0] *= virtualToPhysicalFactor * resourceToVirtualFactor;
	frameVertices[0][1] *= virtualToPhysicalFactor * resourceToVirtualFactor;
	frameVertices[0][2] *= virtualToPhysicalFactor * resourceToVirtualFactor;
	frameVertices[0][3] *= virtualToPhysicalFactor * resourceToVirtualFactor;
	frameVertices[0][4] *= virtualToPhysicalFactor * resourceToVirtualFactor;
	frameVertices[0][5] *= virtualToPhysicalFactor * resourceToVirtualFactor;
	frameVertices[0][6] *= virtualToPhysicalFactor * resourceToVirtualFactor;
	frameVertices[0][7] *= virtualToPhysicalFactor * resourceToVirtualFactor;

	texCoords[0][0] *= resourceToVirtualFactor;
	texCoords[0][1] *= resourceToVirtualFactor;
	texCoords[0][2] *= resourceToVirtualFactor;
	texCoords[0][3] *= resourceToVirtualFactor;
	texCoords[0][4] *= resourceToVirtualFactor;
	texCoords[0][5] *= resourceToVirtualFactor;
	texCoords[0][6] *= resourceToVirtualFactor;
	texCoords[0][7] *= resourceToVirtualFactor;
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
//			SetFrame(frame);
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
	textureHandles.resize(textureCount, InvalidUniqueHandle);
	for(int32 i = 0; i < textureCount; ++i)
    {
        if(textures[i])
        {
            //VI: always set "0" texture for each sprite part
			TextureStateData data;
			data.SetTexture(0, textures[i]);
			
			textureHandles[i] = RenderManager::Instance()->CreateTextureState(data);
		}
	}
}

void Sprite::UnregisterTextureStates()
{
	for(int32 i = 0; i < textureCount; ++i)
    {
		if(textureHandles[i] != InvalidUniqueHandle)
		{
			RenderManager::Instance()->ReleaseTextureState(textureHandles[i]);
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

};
