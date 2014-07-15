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
Vector<Vector2> Sprite::clippedTexCoords;
Vector<Vector2> Sprite::clippedVertices;
bool Sprite::spriteClipping = true;

Mutex Sprite::spriteMapMutex;

Sprite::DrawState::DrawState()
{
    Reset();
    
    renderState = RenderState::RENDERSTATE_2D_BLEND;
    shader = RenderManager::TEXTURE_MUL_FLAT_COLOR;
    //RenderManager::Instance()->RetainRenderState(renderState);
    //shader = SafeRetain(RenderManager::TEXTURE_MUL_FLAT_COLOR);
}

RenderDataObject* Sprite::spriteRenderObject = NULL;
RenderDataStream* Sprite::vertexStream = NULL;
RenderDataStream* Sprite::texCoordStream = NULL;

Sprite::Sprite()
{
	textures = 0;
	textureNames = 0;
	frameTextureIndex = 0;
	textureCount = 0;

	frameVertices = 0;
	texCoords = 0;
	rectsAndOffsets = 0;
//	originalVertices = 0;

	size.dx = 24;
	size.dy = 24;
//	originalSize = size;
	frameCount = 0;
	//frame = 0;

	isPreparedForTiling = false;

	modification = 0;
	flags = 0;
	resourceSizeIndex = 0;

	clipPolygon = 0;

	resourceToVirtualFactor = 1.0f;
	resourceToPhysicalFactor = 1.0f;

	if(spriteRenderObject == NULL)
	{
		spriteRenderObject = new RenderDataObject();
		vertexStream = spriteRenderObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 0, 0);
		texCoordStream  = spriteRenderObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, 0);
	}
	else
	{
		spriteRenderObject->Retain();
	}


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
	String::size_type pos = spriteName.GetAbsolutePathname().find(Core::Instance()->GetResourceFolder(Core::Instance()->GetBaseResourceIndex()));
	if(String::npos != pos)
	{
		String pathname = spriteName.GetAbsolutePathname();

		String subStrPath = pathname.substr(0, pos);
		String resFolder = Core::Instance()->GetResourceFolder(Core::Instance()->GetDesirableResourceIndex());
		String footer = pathname.substr(pos + Core::Instance()->GetResourceFolder(Core::Instance()->GetBaseResourceIndex()).length());

		return pathname.substr(0, pos)
						+ Core::Instance()->GetResourceFolder(Core::Instance()->GetDesirableResourceIndex())
						+ pathname.substr(pos + Core::Instance()->GetResourceFolder(Core::Instance()->GetBaseResourceIndex()).length());
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

//	uint64 timeSpriteRead = SystemTimer::Instance()->AbsoluteMS();

	type = SPRITE_FROM_FILE;
    const FilePath& pathName = file->GetFilename();

	char tempBuf[1024];
	file->ReadLine(tempBuf, 1024);
	sscanf(tempBuf, "%d", &textureCount);
	textures = new Texture*[textureCount];
	textureNames = new FilePath[textureCount];
//	timeSpriteRead = SystemTimer::Instance()->AbsoluteMS() - timeSpriteRead;

	char textureCharName[128];
	for (int32 k = 0; k < textureCount; ++k)
	{
		file->ReadLine(tempBuf, 1024);
		sscanf(tempBuf, "%s", textureCharName);

		FilePath tp = pathName.GetDirectory() + String(textureCharName);
//		Logger::FrameworkDebug("Opening texture: %s", tp.c_str());
        Texture* testTexture = Texture::CreateFromFile(tp);
		textures[k] = testTexture;
		textureNames[k] = tp;
		DVASSERT_MSG(textures[k], "ERROR: Texture loading failed"/* + pathName*/);
	}
	
	RegisterTextureStates();

	resourceToVirtualFactor = Core::Instance()->GetResourceToVirtualFactor(resourceSizeIndex);
	resourceToPhysicalFactor = Core::Instance()->GetResourceToPhysicalFactor(resourceSizeIndex);

//	uint64 timeSpriteRead2 = SystemTimer::Instance()->AbsoluteMS();

	int32 width, height;
	file->ReadLine(tempBuf, 1024);
	sscanf(tempBuf, "%d %d", &width, &height);
	size.dx = (float32)width;
	size.dy = (float32)height;
//	originalSize = size;
	size.dx *= resourceToVirtualFactor;
	size.dy *= resourceToVirtualFactor;
	file->ReadLine(tempBuf, 1024);
	sscanf(tempBuf, "%d", &frameCount);

	texCoords = new GLfloat*[frameCount];
	frameVertices = new GLfloat*[frameCount];
//	originalVertices = new float32*[frameCount];
	rectsAndOffsets = new float32*[frameCount];
	frameTextureIndex = new int32[frameCount];


	for (int32 i = 0; i < frameCount; i++)
	{
		frameVertices[i] = new GLfloat[8];
//		originalVertices[i] = new float32[4];
		texCoords[i] = new GLfloat[8];
		rectsAndOffsets[i] = new GLfloat[6];

		int32 x, y, dx,dy, xOff, yOff;

		file->ReadLine(tempBuf, 1024);
		sscanf(tempBuf, "%d %d %d %d %d %d %d", &x, &y, &dx, &dy, &xOff, &yOff, &frameTextureIndex[i]);

		rectsAndOffsets[i][0] = (float32)x;
		rectsAndOffsets[i][1] = (float32)y;
//		rectsAndOffsets[i][2] = (float32)dx;
//		rectsAndOffsets[i][3] = (float32)dy;
//		rectsAndOffsets[i][4] = (float32)xOff;
//		rectsAndOffsets[i][5] = (float32)yOff;
//		originalVertices[i][0] = (float32)dx;
//		originalVertices[i][1] = (float32)dy;
//		originalVertices[i][2] = (float32)xOff;
//		originalVertices[i][3] = (float32)yOff;

		frameVertices[i][0] = (float32)xOff;
		frameVertices[i][1] = (float32)yOff;
		frameVertices[i][2] = (float32)(xOff + dx);
		frameVertices[i][3] = (float32)yOff;
		frameVertices[i][4] = (float32)xOff;
		frameVertices[i][5] = (float32)(yOff + dy);
		frameVertices[i][6] = (float32)(xOff + dx);
		frameVertices[i][7] = (float32)(yOff + dy);

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


		rectsAndOffsets[i][2] = dx * resourceToVirtualFactor;
		rectsAndOffsets[i][3] = dy * resourceToVirtualFactor;
		rectsAndOffsets[i][4] = xOff * resourceToVirtualFactor;
		rectsAndOffsets[i][5] = yOff * resourceToVirtualFactor;

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

		frameVertices[i][0] *= resourceToVirtualFactor;
		frameVertices[i][1] *= resourceToVirtualFactor;
		frameVertices[i][2] *= resourceToVirtualFactor;
		frameVertices[i][3] *= resourceToVirtualFactor;
		frameVertices[i][4] *= resourceToVirtualFactor;
		frameVertices[i][5] *= resourceToVirtualFactor;
		frameVertices[i][6] *= resourceToVirtualFactor;
		frameVertices[i][7] *= resourceToVirtualFactor;
	}
    
    char frameName[128];
    frameNames.clear();

	for (int32 i = 0; i < frameCount; i++)
	{
    	String actualFrameName;
        
		if (file->ReadLine(tempBuf, 1024) == 0)
        {
        	actualFrameName = "";
        }
        else
        {
			sscanf(tempBuf, "%s", frameName);
            actualFrameName = String(frameName);
        }
        
		frameNames.push_back(actualFrameName);
       // Logger::FrameworkDebug("Sprite::InitFromFile - Loaded frame name: %s", String(actualFrameName).c_str());
    }

//	Logger::FrameworkDebug("Frames created: %d", spr->frameCount);
	//	center.x = width / 2;
	//	center.y = height / 2;

	defaultPivotPoint.x = 0;
	defaultPivotPoint.y = 0;

//	timeSpriteRead2 = SystemTimer::Instance()->AbsoluteMS() - timeSpriteRead2;
//  Logger::FrameworkDebug("Sprite: %s time:%lld", relativePathname.c_str(), timeSpriteRead2 + timeSpriteRead);

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
	if (!contentScaleIncluded)
	{
		sprWidth = sprWidth * Core::GetVirtualToPhysicalFactor();
		sprHeight = sprHeight * Core::GetVirtualToPhysicalFactor();
	}

	Texture *t = Texture::CreateFBO((int32)ceilf(sprWidth), (int32)ceilf(sprHeight), textureFormat, Texture::DEPTH_NONE);

	this->InitFromTexture(t, 0, 0, sprWidth, sprHeight, -1, -1, true);

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
        float32 sprWidth = (float32)width;
		float32 sprHeight = (float32)height;
        if(inVirtualSpace)
        {
            sprWidth *= Core::GetPhysicalToVirtualFactor();
            sprHeight *= Core::GetPhysicalToVirtualFactor();
        }
        
        sprite = Sprite::CreateFromTexture(texture, 0, 0, sprWidth, sprHeight, contentScaleIncluded);
        
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
	if (!contentScaleIncluded)
	{
		xOffset = (int32)(Core::GetVirtualToPhysicalFactor() * xOffset);
		yOffset = (int32)(Core::GetVirtualToPhysicalFactor() * yOffset);
	}
	else
	{
		sprWidth = Core::GetPhysicalToVirtualFactor() * sprWidth;
		sprHeight = Core::GetPhysicalToVirtualFactor() * sprHeight;
	}

	resourceToPhysicalFactor = Core::GetVirtualToPhysicalFactor();
	resourceToVirtualFactor = Core::GetPhysicalToVirtualFactor();
	resourceSizeIndex = Core::Instance()->GetDesirableResourceIndex();

	this->type = SPRITE_FROM_TEXTURE;
	this->textureCount = 1;
	this->textures = new Texture*[this->textureCount];
	this->textureNames = new FilePath[this->textureCount];


	this->textures[0] = SafeRetain(fromTexture);
	if(this->textures[0])
	{
		this->textureNames[0] = this->textures[0]->GetPathname();
	}

//	int32 width = sprWidth;
	this->size.dx = (float32)sprWidth;

//	int32 height = sprHeight;
	this->size.dy = (float32)sprHeight;

//	Logger::FrameworkDebug("Init from texture: %.4fx%.4f", sprWidth, sprWidth);

//	this->originalSize = this->size;
	this->defaultPivotPoint.x = 0;
	this->defaultPivotPoint.y = 0;
	this->frameCount = 1;

	this->texCoords = new GLfloat*[this->frameCount];
	this->frameVertices = new GLfloat*[this->frameCount];
//	this->originalVertices = new GLfloat*[this->frameCount];
	this->rectsAndOffsets = new GLfloat*[this->frameCount];
	this->frameTextureIndex = new int32[this->frameCount];

	for (int i = 0;	i < this->frameCount; i++)
	{
		this->frameVertices[i] = new GLfloat[8];
//		this->originalVertices[i] = new GLfloat[4];
		this->texCoords[i] = new GLfloat[8];
		this->rectsAndOffsets[i] = new GLfloat[6];
		this->frameTextureIndex[i] = 0;

		float32 x, y, dx,dy, xOff, yOff;
		x = (float32)xOffset;
		y = (float32)yOffset;
		dx = sprWidth * Core::GetVirtualToPhysicalFactor();
		dy = sprHeight * Core::GetVirtualToPhysicalFactor();
		xOff = 0;
		yOff = 0;

		this->rectsAndOffsets[i][0] = (float32)x;
		this->rectsAndOffsets[i][1] = (float32)y;
		this->rectsAndOffsets[i][2] = sprWidth;
		this->rectsAndOffsets[i][3] = sprHeight;
		this->rectsAndOffsets[i][4] = (float32)xOff;
		this->rectsAndOffsets[i][5] = (float32)yOff;

//		this->originalVertices[i][0] = (float32)dx;
//		this->originalVertices[i][1] = (float32)dy;
//		this->originalVertices[i][2] = (float32)xOff;
//		this->originalVertices[i][3] = (float32)yOff;

		this->frameVertices[i][0] = (float32)xOff;
		this->frameVertices[i][1] = (float32)yOff;
		this->frameVertices[i][2] = (float32)xOff + sprWidth;
		this->frameVertices[i][3] = (float32)yOff;
		this->frameVertices[i][4] = (float32)xOff;
		this->frameVertices[i][5] = (float32)(yOff + sprHeight);
		this->frameVertices[i][6] = (float32)(xOff + sprWidth);
		this->frameVertices[i][7] = (float32)(yOff + sprHeight);


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
	this->relativePathname = spriteName.IsEmpty() ? Format("FBO sprite %d", fboCounter) : spriteName;

    spriteMapMutex.Lock();
	spriteMap[FILEPATH_MAP_KEY(this->relativePathname)] = this;
    spriteMapMutex.Unlock();

	fboCounter++;
	this->Reset();
	
	RegisterTextureStates();
}

void Sprite::PrepareForTiling()
{
	if(!isPreparedForTiling)
	{
		for (int i = 0;	i < this->frameCount; i++)
		{
			this->texCoords[i][0] += (1.0f/this->textures[this->frameTextureIndex[i]]->width); // x
			this->texCoords[i][1] += (1.0f/this->textures[this->frameTextureIndex[i]]->height); // y
			this->texCoords[i][2] -= (2.0f/this->textures[this->frameTextureIndex[i]]->width); // x+dx
			this->texCoords[i][3] += (1.0f/this->textures[this->frameTextureIndex[i]]->height); // y
			this->texCoords[i][4] += (1.0f/this->textures[this->frameTextureIndex[i]]->width); // x
			this->texCoords[i][5] -= (2.0f/this->textures[this->frameTextureIndex[i]]->height); // y+dy
			this->texCoords[i][6] -= (2.0f/this->textures[this->frameTextureIndex[i]]->width); // x+dx
			this->texCoords[i][7] -= (2.0f/this->textures[this->frameTextureIndex[i]]->height); // y+dy
		}
		isPreparedForTiling = true;
	}
}

void Sprite::SetOffsetsForFrame(int frame, float32 xOff, float32 yOff)
{
	DVASSERT(frame < frameCount);

	rectsAndOffsets[frame][4] = xOff;
	rectsAndOffsets[frame][5] = yOff;

//	originalVertices[frame][2] = xOff / Core::Instance()->GetResourceToVirtualFactor(resourceSizeIndex);
//	originalVertices[frame][3] = yOff / Core::Instance()->GetResourceToVirtualFactor(resourceSizeIndex);

	frameVertices[frame][0] = xOff;
	frameVertices[frame][1] = yOff;
	frameVertices[frame][2] = xOff + rectsAndOffsets[frame][2];
	frameVertices[frame][3] = yOff;
	frameVertices[frame][4] = xOff;
	frameVertices[frame][5] = yOff + rectsAndOffsets[frame][3];
	frameVertices[frame][6] = xOff + rectsAndOffsets[frame][2];
	frameVertices[frame][7] = yOff + rectsAndOffsets[frame][3];
}

int32 Sprite::Release()
{
	if(GetRetainCount() == 1)
	{
        spriteMapMutex.Lock();
		//SafeRelease(spriteRenderObject);
		spriteMap.erase(FILEPATH_MAP_KEY(relativePathname));
        spriteMapMutex.Unlock();
	}

	return BaseObject::Release();
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
			//			SafeDeleteArray(originalVertices[i]);
			SafeDeleteArray(texCoords[i]);
			SafeDeleteArray(rectsAndOffsets[i]);
		}
	}
	//	if(maxCollisionPoints != 0)
	//	{
	//		for (int i = 0;	i < frameCount; i++)
	//		{
	//			[collision[i] release];
	//		}
	//		SAFE_DELETE_ARRAY(collision);
	//	}
	SafeDeleteArray(frameVertices);
	//	SafeDeleteArray(originalVertices);
	SafeDeleteArray(texCoords);
	SafeDeleteArray(rectsAndOffsets);
	SafeDeleteArray(frameTextureIndex);
	textureCount = 0;
}

Sprite::~Sprite()
{
//	Logger::FrameworkDebug("Removing sprite");
	Clear();

	if(spriteRenderObject)
	{
		if(spriteRenderObject->GetRetainCount() == 1)
		{
		    SafeRelease(spriteRenderObject);
		}
		else
		{
		    spriteRenderObject->Release();
		}
	}
}

Texture* Sprite::GetTexture()
{
	return textures[0];
}

Texture* Sprite::GetTexture(int32 frameNumber)
{
//	DVASSERT(frameNumber > -1 && frameNumber < frameCount);
	frame = Clamp(frameNumber, 0, frameCount - 1);
	return textures[frameTextureIndex[frame]];
}
	
UniqueHandle Sprite::GetTextureHandle(int32 frameNumber)
{
	frame = Clamp(frameNumber, 0, frameCount - 1);
	return textureHandles[frameTextureIndex[frame]];
}

float32 *Sprite::GetTextureVerts(int32 frame)
{
//	DVASSERT(frame > -1 && frame < frameCount);
	uint32 frameNumber = Clamp(frame, 0, frameCount - 1);
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

void Sprite::SetFrame(int32 frm)
{
	frame = Max(0, Min(frm, frameCount - 1));
}

int32 Sprite::GetFrameByName(const String& frameName)
{
    for (int32 i = 0; i < frameCount; i++)
	{
    	if (frameNames[i].compare(frameName) == 0)
        	return i;
    }
    
    return 0;
}

/*void Sprite::SetPivotPoint(float32 x, float32 y)
{
	pivotPoint.x = x;
	pivotPoint.y = y;
}

void Sprite::SetPivotPoint(const Vector2 &newPivotPoint)
{
	pivotPoint = newPivotPoint;
}

void Sprite::SetPosition(float32 x, float32 y)
{
	drawCoord.x = x;
	drawCoord.y = y;
}

void Sprite::SetPosition(const Vector2 &drawPos)
{
	drawCoord = drawPos;
}


void Sprite::SetAngle(float32 angleInRadians)
{
	rotateAngle = angleInRadians;
	if(angleInRadians != 0)
	{
		flags = flags | EST_ROTATE;
	}
	else
	{
		ResetAngle();
	}
}

void Sprite::SetScale(float32 xScale, float32 yScale)
{
	if(xScale != 1.f || yScale != 1.f)
	{
		scale.x = xScale;
		scale.y = yScale;

		flags = flags | EST_SCALE;
	}
	else
	{
		ResetScale();
	}
}

void Sprite::SetScale(const Vector2 &newScale)
{
	if(newScale.x != 1.f || newScale.y != 1.f)
	{
		scale = newScale;
		flags = flags | EST_SCALE;
	}
	else
	{
		ResetScale();
	}
}

void Sprite::SetScaleSize(float32 width, float32 height)
{
	if(width != size.dx || height != size.dy)
	{
		scale.x = width / size.dx;
		scale.y = height / size.dy;
		flags = flags | EST_SCALE;
	}
	else
	{
		ResetScale();
	}
}

void Sprite::SetScaleSize(const Vector2 &drawSize)
{
	SetScaleSize(drawSize.x, drawSize.y);
}
*/

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
	//drawCoord.x = 0;
	//drawCoord.y = 0;
	//frame = 0;
	flags = 0;
	//rotateAngle = 0.0f;
	modification = 0;
	//SetScale(1.0f, 1.0f);
	//ResetPivotPoint();
	clipPolygon = 0;
}

void Sprite::ResetModification()
{
    flags = flags & ~EST_MODIFICATION;
}


/*void Sprite::ResetPivotPoint()
{
	pivotPoint = defaultPivotPoint;
}

void Sprite::ResetAngle()
{
	flags = flags & ~EST_ROTATE;
}


void Sprite::ResetScale()
{
	scale.x = 1.f;
	scale.y = 1.f;
	flags = flags & ~EST_SCALE;
}
*/

inline void Sprite::PrepareSpriteRenderData(Sprite::DrawState * state)
{
    DVASSERT(state);
    
	float32 x, y;
    float32 scaleX = 1.0f;
    float32 scaleY = 1.0f;

    flags = 0;
    if (state->flags != 0)
    {
        flags |= EST_MODIFICATION;
    }
    
    if(state->scale.x != 1.f || state->scale.y != 1.f)
    {
        flags |= EST_SCALE;
        scaleX = state->scale.x;
        scaleY = state->scale.y;
    }
    
    if(state->angle != 0.f) flags |= EST_ROTATE;
    
    frame = Max(0, Min(state->frame, frameCount - 1));
    
    x = state->position.x - state->pivotPoint.x * state->scale.x;
    y = state->position.y - state->pivotPoint.y * state->scale.y;
    
	if(flags & EST_MODIFICATION)
	{
		if((state->flags & (ESM_HFLIP | ESM_VFLIP)) == (ESM_HFLIP | ESM_VFLIP))
		{//HFLIP|VFLIP
			if(flags & EST_SCALE)
			{//SCALE
				x += (size.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2) * scaleX;
				y += (size.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2) * scaleY;
				if(!state || !state->usePerPixelAccuracy || (flags & EST_ROTATE))
				{
					tempVertices[2] = tempVertices[6] = frameVertices[frame][0] * scaleX + x;//x2 do not change this sequence. This is because of the cache reason
					tempVertices[1] = tempVertices[3] = frameVertices[frame][5] * scaleY + y;//y1
					tempVertices[0] = tempVertices[4] = frameVertices[frame][2] * scaleX + x;//x1
					tempVertices[5] = tempVertices[7] = frameVertices[frame][1] * scaleY + y;//y2
				}
				else
				{
#if !defined (NEW_PPA)
					tempVertices[2] = tempVertices[6] = floorf((frameVertices[frame][0] * scaleX + x) * Core::GetVirtualToPhysicalFactor() + 0.5f);//x2
					tempVertices[5] = tempVertices[7] = floorf((frameVertices[frame][1] * scaleY + y) * Core::GetVirtualToPhysicalFactor() + 0.5f);//y2
					tempVertices[0] = tempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) * scaleX * Core::GetVirtualToPhysicalFactor() + tempVertices[2];//x1
					tempVertices[1] = tempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) * scaleY * Core::GetVirtualToPhysicalFactor() + tempVertices[5];//y1

					RenderManager::Instance()->SetPhysicalViewScale();
#else
					tempVertices[2] = tempVertices[6] = floorf((frameVertices[frame][0] * scaleX + x) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//x2
					tempVertices[5] = tempVertices[7] = floorf((frameVertices[frame][1] * scaleY + y) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//y2
					tempVertices[0] = tempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) * scaleX + tempVertices[2];//x1
					tempVertices[1] = tempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) * scaleY + tempVertices[5];//y1
#endif
				}
			}
			else
			{//NOT SCALE
				x += (size.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2);
				y += (size.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2);
				if(!state || !state->usePerPixelAccuracy || (flags & EST_ROTATE))
				{
					tempVertices[2] = tempVertices[6] = frameVertices[frame][0] + x;//x2 do not change this sequence. This is because of the cache reason
					tempVertices[1] = tempVertices[3] = frameVertices[frame][5] + y;//y1
					tempVertices[0] = tempVertices[4] = frameVertices[frame][2] + x;//x1
					tempVertices[5] = tempVertices[7] = frameVertices[frame][1] + y;//y2
				}
				else
				{
#if !defined (NEW_PPA)
					tempVertices[2] = tempVertices[6] = floorf((frameVertices[frame][0] + x) * Core::GetVirtualToPhysicalFactor() + 0.5f);//x2
					tempVertices[5] = tempVertices[7] = floorf((frameVertices[frame][1] + y) * Core::GetVirtualToPhysicalFactor() + 0.5f);//y2
					tempVertices[0] = tempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) * Core::GetVirtualToPhysicalFactor() + tempVertices[2];//x1
					tempVertices[1] = tempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) * Core::GetVirtualToPhysicalFactor() + tempVertices[5];//y1

					RenderManager::Instance()->SetPhysicalViewScale();
#else
					tempVertices[2] = tempVertices[6] = floorf((frameVertices[frame][0] + x) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//x2
					tempVertices[5] = tempVertices[7] = floorf((frameVertices[frame][1] + y) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//y2
					tempVertices[0] = tempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) + tempVertices[2];//x1
					tempVertices[1] = tempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) + tempVertices[5];//y1
#endif
				}
			}
		}
		else
		{
			if(state->flags & ESM_HFLIP)
			{//HFLIP
				if(flags & EST_SCALE)
				{//SCALE
					x += (size.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2) * scaleX;
					if(!state || !state->usePerPixelAccuracy || (flags & EST_ROTATE))
					{
						tempVertices[0] = tempVertices[4] = frameVertices[frame][2] * scaleX + x;//x1
						tempVertices[5] = tempVertices[7] = frameVertices[frame][5] * scaleY + y;//y2
						tempVertices[1] = tempVertices[3] = frameVertices[frame][1] * scaleX + y;//y1 //WEIRD: maybe scaleY should be used?
						tempVertices[2] = tempVertices[6] = frameVertices[frame][0] * scaleX + x;//x2
					}
					else
					{
#if !defined (NEW_PPA)
						tempVertices[2] = tempVertices[6] = floorf((frameVertices[frame][0] * scaleX + x) * Core::GetVirtualToPhysicalFactor() + 0.5f);//x2
						tempVertices[0] = tempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) * scale.x * Core::GetVirtualToPhysicalFactor() + tempVertices[2];//x1
						tempVertices[1] = tempVertices[3] = floorf((frameVertices[frame][1] * scaleY + y) * Core::GetVirtualToPhysicalFactor() + 0.5f);//y1
						tempVertices[5] = tempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) * scale.y * Core::GetVirtualToPhysicalFactor() + tempVertices[1];//y2

						RenderManager::Instance()->SetPhysicalViewScale();
#else
						tempVertices[2] = tempVertices[6] = floorf((frameVertices[frame][0] * scaleX + x) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//x2
						tempVertices[0] = tempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) * scaleX + tempVertices[2];//x1
						tempVertices[1] = tempVertices[3] = floorf((frameVertices[frame][1] * scaleY + y) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//y1
						tempVertices[5] = tempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) * scaleY + tempVertices[1];//y2
#endif
					}
				}
				else
				{//NOT SCALE
					x += (size.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2);
					if(!state || !state->usePerPixelAccuracy || (flags & EST_ROTATE))
					{
						tempVertices[0] = tempVertices[4] = frameVertices[frame][2] + x;//x1
						tempVertices[5] = tempVertices[7] = frameVertices[frame][5] + y;//y2
						tempVertices[1] = tempVertices[3] = frameVertices[frame][1] + y;//y1
						tempVertices[2] = tempVertices[6] = frameVertices[frame][0] + x;//x2
					}
					else
					{
#if !defined (NEW_PPA)
						tempVertices[2] = tempVertices[6] = floorf((frameVertices[frame][0] + x) * Core::GetVirtualToPhysicalFactor() + 0.5f);//x2
						tempVertices[0] = tempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) * Core::GetVirtualToPhysicalFactor() + tempVertices[2];//x1
						tempVertices[1] = tempVertices[3] = floorf((frameVertices[frame][1] + y) * Core::GetVirtualToPhysicalFactor() + 0.5f);//y1
						tempVertices[5] = tempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) * Core::GetVirtualToPhysicalFactor() + tempVertices[1];//y2

						RenderManager::Instance()->SetPhysicalViewScale();
#else
						tempVertices[2] = tempVertices[6] = floorf((frameVertices[frame][0] + x) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//x2
						tempVertices[0] = tempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) + tempVertices[2];//x1
						tempVertices[1] = tempVertices[3] = floorf((frameVertices[frame][1] + y) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//y1
						tempVertices[5] = tempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) + tempVertices[1];//y2
#endif
					}
				}
			}
			else
			{//VFLIP
				if(flags & EST_SCALE)
				{//SCALE
					y += (size.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2) * scaleY;
					if(!state || !state->usePerPixelAccuracy || (flags & EST_ROTATE))
					{
						tempVertices[0] = tempVertices[4] = frameVertices[frame][0] * scaleX + x;//x1
						tempVertices[5] = tempVertices[7] = frameVertices[frame][1] * scaleY + y;//y2
						tempVertices[1] = tempVertices[3] = frameVertices[frame][5] * scaleY + y;//y1
						tempVertices[2] = tempVertices[6] = frameVertices[frame][2] * scaleX + x;//x2
					}
					else
					{
#if !defined (NEW_PPA)
						tempVertices[0] = tempVertices[4] = floorf((frameVertices[frame][0] * scale.x + x) * Core::GetVirtualToPhysicalFactor() + 0.5f);//x1
						tempVertices[5] = tempVertices[7] = floorf((frameVertices[frame][1] * scale.y + y) * Core::GetVirtualToPhysicalFactor() + 0.5f);//y2
						tempVertices[2] = tempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) * scale.x * Core::GetVirtualToPhysicalFactor() + tempVertices[0];//x2
						tempVertices[1] = tempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) * scale.y * Core::GetVirtualToPhysicalFactor() + tempVertices[5];//y1

						RenderManager::Instance()->SetPhysicalViewScale();
#else
						tempVertices[0] = tempVertices[4] = floorf((frameVertices[frame][0] * scaleX + x) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//x1
						tempVertices[5] = tempVertices[7] = floorf((frameVertices[frame][1] * scaleY + y) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//y2
						tempVertices[2] = tempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) * scaleX + tempVertices[0];//x2
						tempVertices[1] = tempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) * scaleY + tempVertices[5];//y1
#endif
					}
				}
				else
				{//NOT SCALE
					y += (size.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2);
					if(!state || !state->usePerPixelAccuracy || (flags & EST_ROTATE))
					{
						tempVertices[0] = tempVertices[4] = frameVertices[frame][0] + x;//x1
						tempVertices[5] = tempVertices[7] = frameVertices[frame][1] + y;//y2
						tempVertices[1] = tempVertices[3] = frameVertices[frame][5] + y;//y1
						tempVertices[2] = tempVertices[6] = frameVertices[frame][2] + x;//x2
					}
					else
					{
#if !defined (NEW_PPA)
						tempVertices[0] = tempVertices[4] = floorf((frameVertices[frame][0] + x) * Core::GetVirtualToPhysicalFactor() + 0.5f);//x1
						tempVertices[5] = tempVertices[7] = floorf((frameVertices[frame][1] + y) * Core::GetVirtualToPhysicalFactor() + 0.5f);//y2
						tempVertices[2] = tempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) * Core::GetVirtualToPhysicalFactor() + tempVertices[0];//x2
						tempVertices[1] = tempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) * Core::GetVirtualToPhysicalFactor() + tempVertices[5];//y1

						RenderManager::Instance()->SetPhysicalViewScale();
#else
						tempVertices[0] = tempVertices[4] = floorf((frameVertices[frame][0] + x) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//x1
						tempVertices[5] = tempVertices[7] = floorf((frameVertices[frame][1] + y) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//y2
						tempVertices[2] = tempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) + tempVertices[0];//x2
						tempVertices[1] = tempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) + tempVertices[5];//y1
#endif
					}
				}
			}
		}

	}
	else
	{//NO MODIFERS
		if(flags & EST_SCALE)
		{//SCALE
			if(!state || !state->usePerPixelAccuracy || (flags & EST_ROTATE))
			{
				tempVertices[0] = tempVertices[4] = frameVertices[frame][0] * scaleX + x;//x1
				tempVertices[5] = tempVertices[7] = frameVertices[frame][5] * scaleY + y;//y2
				tempVertices[1] = tempVertices[3] = frameVertices[frame][1] * scaleY + y;//y1
				tempVertices[2] = tempVertices[6] = frameVertices[frame][2] * scaleX + x;//x2 do not change this sequence. This is because of the cache reason
			}
			else
			{
#if !defined (NEW_PPA)
				tempVertices[0] = tempVertices[4] = floorf((frameVertices[frame][0] * scale.x + x) * Core::GetVirtualToPhysicalFactor() + 0.5f);//x1
				tempVertices[1] = tempVertices[3] = floorf((frameVertices[frame][1] * scale.y + y) * Core::GetVirtualToPhysicalFactor() + 0.5f);//y1
				tempVertices[2] = tempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) * scale.x * Core::GetVirtualToPhysicalFactor() + tempVertices[0];//x2
				tempVertices[5] = tempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) * scale.y * Core::GetVirtualToPhysicalFactor() + tempVertices[1];//y2

				RenderManager::Instance()->SetPhysicalViewScale();
#else
				tempVertices[0] = tempVertices[4] = floorf((frameVertices[frame][0] * scaleX + x) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//x1
				tempVertices[1] = tempVertices[3] = floorf((frameVertices[frame][1] * scaleY + y) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//y1
				tempVertices[2] = tempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) * scaleX + tempVertices[0];//x2
				tempVertices[5] = tempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) * scaleY + tempVertices[1];//y2
#endif
			}
		}
		else
		{//NOT SCALE
			if(!state || !state->usePerPixelAccuracy || (flags & EST_ROTATE))
			{
				tempVertices[0] = tempVertices[4] = frameVertices[frame][0] + x;//x1
				tempVertices[5] = tempVertices[7] = frameVertices[frame][5] + y;//y2
				tempVertices[1] = tempVertices[3] = frameVertices[frame][1] + y;//y1
				tempVertices[2] = tempVertices[6] = frameVertices[frame][2] + x;//x2 do not change this sequence. This is because of the cache reason
			}
			else
			{
#if !defined (NEW_PPA)
				tempVertices[0] = tempVertices[4] = floorf((frameVertices[frame][0] + x) * Core::GetVirtualToPhysicalFactor() + 0.5f);//x1
				tempVertices[1] = tempVertices[3] = floorf((frameVertices[frame][1] + y) * Core::GetVirtualToPhysicalFactor() + 0.5f);//y1
				tempVertices[2] = tempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) * Core::GetVirtualToPhysicalFactor() + tempVertices[0];//x2
				tempVertices[5] = tempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) * Core::GetVirtualToPhysicalFactor() + tempVertices[1];//y2

				RenderManager::Instance()->SetPhysicalViewScale();
#else
				tempVertices[0] = tempVertices[4] = floorf((frameVertices[frame][0] + x) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//x1
				tempVertices[1] = tempVertices[3] = floorf((frameVertices[frame][1] + y) * Core::GetVirtualToPhysicalFactor() + 0.5f) * Core::GetPhysicalToVirtualFactor();//y1
				tempVertices[2] = tempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) + tempVertices[0];//x2
				tempVertices[5] = tempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) + tempVertices[1];//y2
#endif
			}

		}

	}

	if(!clipPolygon)
	{
		if(flags & EST_ROTATE)
		{
			//SLOW CODE
			//			glPushMatrix();
			//			glTranslatef(drawCoord.x, drawCoord.y, 0);
			//			glRotatef(RadToDeg(rotateAngle), 0.0f, 0.0f, 1.0f);
			//			glTranslatef(-drawCoord.x, -drawCoord.y, 0);
			//			RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
			//			glPopMatrix();

			// Optimized code
			float32 sinA = sinf(state->angle);
			float32 cosA = cosf(state->angle);
			for(int32 k = 0; k < 4; ++k)
			{
				float32 x = tempVertices[(k << 1)] - state->position.x;
				float32 y = tempVertices[(k << 1) + 1] - state->position.y;

				float32 nx = (x) * cosA  - (y) * sinA + state->position.x;
				float32 ny = (x) * sinA  + (y) * cosA + state->position.y;

				tempVertices[(k << 1)] = nx;
				tempVertices[(k << 1) + 1] = ny;
			}
		}

		vertexStream->Set(TYPE_FLOAT, 2, 0, tempVertices);
		texCoordStream->Set(TYPE_FLOAT, 2, 0, texCoords[frame]);
		primitiveToDraw = PRIMITIVETYPE_TRIANGLESTRIP;
		vertexCount = 4;
	}
	else
	{
		clippedVertices.clear();
        clippedVertices.reserve(clipPolygon->GetPointCount());
		clippedTexCoords.clear();
        clippedTexCoords.reserve(clipPolygon->GetPointCount());
        
		Texture * t = GetTexture(frame);
		float32 adjWidth = 1.f / t->width / resourceToVirtualFactor;
		float32 adjHeight = 1.f / t->height / resourceToVirtualFactor;

		if( flags & EST_SCALE )
		{
			for(int32 i = 0; i < clipPolygon->GetPointCount(); ++i)
			{
				const Vector2 &point = clipPolygon->GetPoints()[i];
				clippedVertices.push_back( Vector2( point.x*scaleX + x, point.y*scaleY + y ) );
			}
		}
		else
		{
			Vector2 pos(x, y);
			for(int32 i = 0; i < clipPolygon->GetPointCount(); ++i)
			{
				const Vector2 &point = clipPolygon->GetPoints()[i];
				clippedVertices.push_back( point + pos );
			}
		}

		for( int32 i = 0; i < clipPolygon->GetPointCount(); ++i )
		{
			const Vector2 &point = clipPolygon->GetPoints()[i];

			Vector2 texCoord( ( point.x - frameVertices[frame][0] ) * adjWidth
							, ( point.y - frameVertices[frame][1] ) * adjHeight );

			clippedTexCoords.push_back( Vector2( texCoords[frame][0] + texCoord.x
											   , texCoords[frame][1] + texCoord.y ) );
		}

		vertexStream->Set(TYPE_FLOAT, 2, 0, &clippedVertices.front());
		texCoordStream->Set(TYPE_FLOAT, 2, 0, &clippedTexCoords.front());
		primitiveToDraw = PRIMITIVETYPE_TRIANGLEFAN;
		vertexCount = clipPolygon->pointCount;
	}

	DVASSERT(vertexStream->pointer != 0);
	DVASSERT(texCoordStream->pointer != 0);
}

/*void Sprite::Draw()
{
	if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
	{
		return;
	}
    
    RENDERER_UPDATE_STATS(spriteDrawCount++);

    PrepareSpriteRenderData(0);

    if( clipPolygon )
    {
        RenderManager::Instance()->ClipPush();
        Rect clipRect;
        if( flags & EST_SCALE )
        {
            float32 x = drawCoord.x - pivotPoint.x * scale.x;
            float32 y = drawCoord.y - pivotPoint.y * scale.y;
            clipRect = Rect( GetRectOffsetValueForFrame( frame, X_OFFSET_TO_ACTIVE ) * scale.x + x
                           , GetRectOffsetValueForFrame( frame, Y_OFFSET_TO_ACTIVE ) * scale.y + y
                           , GetRectOffsetValueForFrame( frame, ACTIVE_WIDTH  ) * scale.x
                           , GetRectOffsetValueForFrame( frame, ACTIVE_HEIGHT ) * scale.y );
        }
        else
        {
            float32 x = drawCoord.x - pivotPoint.x;
            float32 y = drawCoord.y - pivotPoint.y;
            clipRect = Rect( GetRectOffsetValueForFrame( frame, X_OFFSET_TO_ACTIVE ) + x
                           , GetRectOffsetValueForFrame( frame, Y_OFFSET_TO_ACTIVE ) + y
                           , GetRectOffsetValueForFrame( frame, ACTIVE_WIDTH )
                           , GetRectOffsetValueForFrame( frame, ACTIVE_HEIGHT ) );
        }

        RenderManager::Instance()->ClipRect( clipRect );
    }

	RenderManager::Instance()->SetTextureState(textureHandles[frameTextureIndex[frame]]);
    RenderManager::Instance()->SetRenderData(spriteRenderObject);
    RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
    RenderManager::Instance()->DrawArrays(primitiveToDraw, 0, vertexCount);

    if( clipPolygon )
    {
        RenderManager::Instance()->ClipPop();
    }

	Reset();
}
*/

void Sprite::Draw(DrawState * state)
{
	// DF-2897 - Do not draw sprite if its position is beyond screen
	if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW) ||
		(!IsSpriteOnScreen(state) && spriteClipping))
	{
		return;
	}
    
    RENDERER_UPDATE_STATS(spriteDrawCount++);

#if !defined (NEW_PPA)
	if (state->usePerPixelAccuracy)
		RenderManager::Instance()->PushMappingMatrix();
#endif

	PrepareSpriteRenderData(state);

	if( clipPolygon )
	{
		RenderManager::Instance()->ClipPush();
		Rect clipRect;
		if( flags & EST_SCALE )
		{
			float32 x = state->position.x - state->pivotPoint.x * state->scale.x;
			float32 y = state->position.y - state->pivotPoint.y * state->scale.y;
			clipRect = Rect( GetRectOffsetValueForFrame( state->frame, X_OFFSET_TO_ACTIVE ) * state->scale.x + x
						   , GetRectOffsetValueForFrame( state->frame, Y_OFFSET_TO_ACTIVE ) * state->scale.y + y
						   , GetRectOffsetValueForFrame( state->frame, ACTIVE_WIDTH  ) * state->scale.x
						   , GetRectOffsetValueForFrame( state->frame, ACTIVE_HEIGHT ) * state->scale.y );
		}
		else
		{
			float32 x = state->position.x - state->pivotPoint.x;
			float32 y = state->position.y - state->pivotPoint.y;
			clipRect = Rect( GetRectOffsetValueForFrame( state->frame, X_OFFSET_TO_ACTIVE ) + x
						   , GetRectOffsetValueForFrame( state->frame, Y_OFFSET_TO_ACTIVE ) + y
						   , GetRectOffsetValueForFrame( state->frame, ACTIVE_WIDTH )
						   , GetRectOffsetValueForFrame( state->frame, ACTIVE_HEIGHT ) );
		}

		RenderManager::Instance()->ClipRect( clipRect );
	}

    RenderManager::Instance()->SetRenderState(state->renderState);
	RenderManager::Instance()->SetTextureState(textureHandles[frameTextureIndex[frame]]);
	RenderManager::Instance()->SetRenderData(spriteRenderObject);
    RenderManager::Instance()->SetRenderEffect(state->shader);
 	RenderManager::Instance()->DrawArrays(primitiveToDraw, 0, vertexCount);

	if( clipPolygon )
	{
		RenderManager::Instance()->ClipPop();
	}

#if !defined (NEW_PPA)
	if (state->usePerPixelAccuracy)
		RenderManager::Instance()->PopMappingMatrix();
#endif

}

void Sprite::BeginBatching()
{


}

void Sprite::EndBatching()
{


}


void Sprite::DrawPoints(Vector2 *verticies, Vector2 *textureCoordinates, DrawState* drawState)
{
	GLfloat tempCoord[8];
	Memcpy(tempCoord, texCoords[frame], sizeof(float32)*8);


	texCoords[frame][0] = textureCoordinates[0].x;
	texCoords[frame][1] = textureCoordinates[0].y;
	texCoords[frame][2] = textureCoordinates[1].x;
	texCoords[frame][3] = textureCoordinates[1].y;
	texCoords[frame][4] = textureCoordinates[2].x;
	texCoords[frame][5] = textureCoordinates[2].y;
	texCoords[frame][6] = textureCoordinates[3].x;
	texCoords[frame][7] = textureCoordinates[3].y;
	DrawPoints(verticies, drawState);

	Memcpy(texCoords[frame], tempCoord, sizeof(float32)*8);
}


void Sprite::DrawPoints(Vector2 *verticies, DrawState* drawState)
{
	if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
	{
		return;
	}
    
    RENDERER_UPDATE_STATS(spriteDrawCount++);

	float32 x = drawState->position.x;
	float32 y = drawState->position.y;
	if(flags & EST_SCALE)
	{
		x -= drawState->pivotPoint.x * drawState->scale.x;
		y -= drawState->pivotPoint.y * drawState->scale.y;
	}
	else
	{
		x -= drawState->pivotPoint.x;
		y -= drawState->pivotPoint.y;
	}

	if (!textures)
	{
		RenderManager::Instance()->SetColor(1.0f, 0.0f, 1.0f, 1.0f);
		RenderHelper::Instance()->FillRect(Rect(drawState->position.x - 12, drawState->position.y - 12, 24.0f, 24.0f), drawState->GetRenderState());
		RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		return;
	}

	if(flags & EST_MODIFICATION)
	{
		if((modification & (ESM_HFLIP | ESM_VFLIP)) == (ESM_HFLIP | ESM_VFLIP))
		{
			if(flags & EST_SCALE)
			{
				tempVertices[0] = verticies[3].x * drawState->scale.x + x;
				tempVertices[1] = verticies[3].y * drawState->scale.y + y;
				tempVertices[2] = verticies[2].x * drawState->scale.x + x;
				tempVertices[3] = verticies[2].y * drawState->scale.y + y;
				tempVertices[4] = verticies[1].x * drawState->scale.x + x;
				tempVertices[5] = verticies[1].y * drawState->scale.y + y;
				tempVertices[6] = verticies[0].x * drawState->scale.x + x;
				tempVertices[7] = verticies[0].y * drawState->scale.y + y;
			}
			else
			{
				tempVertices[0] = verticies[3].x + x;
				tempVertices[1] = verticies[3].y + y;
				tempVertices[2] = verticies[2].x + x;
				tempVertices[3] = verticies[2].y + y;
				tempVertices[4] = verticies[1].x + x;
				tempVertices[5] = verticies[1].y + y;
				tempVertices[6] = verticies[0].x + x;
				tempVertices[7] = verticies[0].y + y;
			}
		}
		else
		{
			if(modification & ESM_HFLIP)
			{
				if(flags & EST_SCALE)
				{
					tempVertices[0] = verticies[1].x * drawState->scale.x + x;
					tempVertices[1] = verticies[1].y * drawState->scale.y + y;
					tempVertices[2] = verticies[0].x * drawState->scale.x + x;
					tempVertices[3] = verticies[0].y * drawState->scale.y + y;
					tempVertices[4] = verticies[3].x * drawState->scale.x + x;
					tempVertices[5] = verticies[3].y * drawState->scale.y + y;
					tempVertices[6] = verticies[2].x * drawState->scale.x + x;
					tempVertices[7] = verticies[2].y * drawState->scale.y + y;
				}
				else
				{
					tempVertices[0] = verticies[1].x + x;
					tempVertices[1] = verticies[1].y + y;
					tempVertices[2] = verticies[0].x + x;
					tempVertices[3] = verticies[0].y + y;
					tempVertices[4] = verticies[3].x + x;
					tempVertices[5] = verticies[3].y + y;
					tempVertices[6] = verticies[2].x + x;
					tempVertices[7] = verticies[2].y + y;
				}
			}
			else
			{
				if(flags & EST_SCALE)
				{
					tempVertices[0] = verticies[2].x * drawState->scale.x + x;
					tempVertices[1] = verticies[2].y * drawState->scale.y + y;
					tempVertices[2] = verticies[3].x * drawState->scale.x + x;
					tempVertices[3] = verticies[3].y * drawState->scale.y + y;
					tempVertices[4] = verticies[0].x * drawState->scale.x + x;
					tempVertices[5] = verticies[0].y * drawState->scale.y + y;
					tempVertices[6] = verticies[1].x * drawState->scale.x + x;
					tempVertices[7] = verticies[1].y * drawState->scale.y + y;
				}
				else
				{
					tempVertices[0] = verticies[2].x + x;
					tempVertices[1] = verticies[2].y + y;
					tempVertices[2] = verticies[3].x + x;
					tempVertices[3] = verticies[3].y + y;
					tempVertices[4] = verticies[0].x + x;
					tempVertices[5] = verticies[0].y + y;
					tempVertices[6] = verticies[1].x + x;
					tempVertices[7] = verticies[1].y + y;
				}
			}
		}
	}
	else
	{
		if(flags & EST_SCALE)
		{
			tempVertices[0] = verticies[0].x * drawState->scale.x + x;
			tempVertices[1] = verticies[0].y * drawState->scale.y + y;
			tempVertices[2] = verticies[1].x * drawState->scale.x + x;
			tempVertices[3] = verticies[1].y * drawState->scale.y + y;
			tempVertices[4] = verticies[2].x * drawState->scale.x + x;
			tempVertices[5] = verticies[2].y * drawState->scale.y + y;
			tempVertices[6] = verticies[3].x * drawState->scale.x + x;
			tempVertices[7] = verticies[3].y * drawState->scale.y + y;
		}
		else
		{
			tempVertices[0] = verticies[0].x + x;
			tempVertices[1] = verticies[0].y + y;
			tempVertices[2] = verticies[1].x + x;
			tempVertices[3] = verticies[1].y + y;
			tempVertices[4] = verticies[2].x + x;
			tempVertices[5] = verticies[2].y + y;
			tempVertices[6] = verticies[3].x + x;
			tempVertices[7] = verticies[3].y + y;
		}

	}

	vertexStream->Set(TYPE_FLOAT, 2, 0, tempVertices);
	texCoordStream->Set(TYPE_FLOAT, 2, 0, texCoords[frame]);

	primitiveToDraw = PRIMITIVETYPE_TRIANGLESTRIP;
	vertexCount = 4;

	if(flags & EST_ROTATE)
	{
        // Optimized code
        float32 sinA = sinf(drawState->angle);
        float32 cosA = cosf(drawState->angle);
        for(int32 k = 0; k < 4; ++k)
        {
            float32 x = tempVertices[(k << 1)] - drawState->position.x;
            float32 y = tempVertices[(k << 1) + 1] - drawState->position.y;
            
            float32 nx = (x) * cosA  - (y) * sinA + drawState->position.x;
            float32 ny = (x) * sinA  + (y) * cosA + drawState->position.y;
            
            tempVertices[(k << 1)] = nx;
            tempVertices[(k << 1) + 1] = ny;
        }
    }	

    //RenderManager::Instance()->SetTexture(textures[frameTextureIndex[frame]]);
	RenderManager::Instance()->SetTextureState(textureHandles[frameTextureIndex[frame]]);
	RenderManager::Instance()->SetRenderData(spriteRenderObject);
	RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
	RenderManager::Instance()->DrawArrays(primitiveToDraw, 0, vertexCount);
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

	int pos = (int)pathname.find(Core::Instance()->GetResourceFolder(Core::Instance()->GetBaseResourceIndex()));
	String scaledName = pathname.substr(0, pos) + Core::Instance()->GetResourceFolder(Core::Instance()->GetDesirableResourceIndex()) + pathname.substr(pos + Core::Instance()->GetResourceFolder(Core::Instance()->GetBaseResourceIndex()).length());

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
//	Logger::FrameworkDebug("erasing from sprite from map");
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
	frame = 0;

	modification = 0;
	flags = 0;
	resourceSizeIndex = 0;

	clipPolygon = 0;

	resourceToVirtualFactor = 1.0f;

    resourceToPhysicalFactor = 1.0f;
    
    
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
		if (sp->type == SPRITE_FROM_FILE && Core::Instance()->GetDesirableResourceIndex() != sp->GetResourceSizeIndex())
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
	float32 virtualToPhysicalFactor = Core::Instance()->GetVirtualToPhysicalFactor();
	float32 resourceToVirtualFactor = Core::Instance()->GetResourceToVirtualFactor(GetResourceSizeIndex());

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
			SetFrame(frame);
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

        resourceSizeIndex = Core::Instance()->GetBaseResourceIndex();
    }
    else
    {
        resourceSizeIndex = Core::Instance()->GetDesirableResourceIndex();
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

void Sprite::SetSpriteClipping(bool clipping)
{
	spriteClipping = clipping;
}

bool Sprite::IsSpriteOnScreen(DrawState * state)
{
	// DF-2897 - Calculate real size of texture and it's position
	float32 realWidth = GetWidth() * state->scale.x;
	float32 realHeight = GetHeight() * state->scale.y;
	float32 xPosition = state->position.x - (state->pivotPoint.x * state->scale.x);
	float32 yPosition = state->position.y - (state->pivotPoint.y * state->scale.y);

	float32 screenWidth = Core::Instance()->GetVirtualScreenWidth();
 	float32 screenHeight = Core::Instance()->GetVirtualScreenHeight();

	// If texuture has negative coordinates - do not draw it
	// If texture is outside the screen - do not draw it
	if ((xPosition + realWidth) < 0 || (yPosition + realHeight) < 0 ||
			xPosition >=  screenWidth || yPosition >= screenHeight)
	{
		return false;
	}
	
	return true;
}

};
