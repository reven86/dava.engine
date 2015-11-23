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


#include "TexturePacker/DefinitionFile.h"
#include "TexturePacker/PngImage.h"
#include <sys/types.h>
#include <sys/stat.h>

#include "FileSystem/FileSystem.h"
#include "FramePathHelper.h"

namespace DAVA
{


DefinitionFile::DefinitionFile()
:	frameCount(0)
,	spriteWidth(0)
,	spriteHeight(0)
,	frameRects(0)
{
}

DefinitionFile::~DefinitionFile()
{
	SafeDeleteArray(frameRects);
}

void DefinitionFile::LoadPNG(const FilePath & _filename, const FilePath & pathToProcess)
{
    DVASSERT(pathToProcess.IsDirectoryPathname());
	
    String nameWithoutExt = _filename.GetBasename();
    FilePath corespondingPngImage = FilePath::CreateWithNewExtension(_filename, ".png");

	filename = pathToProcess + (nameWithoutExt + String(".txt"));
	frameCount = 1;

	PngImageExt image;
	image.Read(corespondingPngImage);
	spriteWidth = image.GetWidth();
	spriteHeight = image.GetHeight();

	frameRects = new Rect2i[1];
	frameRects[0].x = 0;
	frameRects[0].y = 0;
	frameRects[0].dx = spriteWidth;
	frameRects[0].dy = spriteHeight;
    
    frameNames.resize(frameCount);

	FilePath fileWrite = FramePathHelper::GetFramePathAbsolute(pathToProcess, nameWithoutExt, 0);
	FileSystem::Instance()->CopyFile(_filename, fileWrite);
}

bool DefinitionFile::LoadPNGDef(const FilePath& _filename, const FilePath& pathToProcess)
{
    DVASSERT(pathToProcess.IsDirectoryPathname());

    Logger::FrameworkDebug("* Load PNG Definition: %s", _filename.GetAbsolutePathname().c_str());
	
	FILE * fp = fopen(_filename.GetAbsolutePathname().c_str(), "rt");
	fscanf(fp, "%d", &frameCount);

	String nameWithoutExt = _filename.GetBasename();
	FilePath corespondingPngImage = _filename.GetDirectory() +  (nameWithoutExt + String(".png"));

	filename = pathToProcess + (nameWithoutExt + String(".txt"));
	
	PngImageExt image;
	image.Read(corespondingPngImage);
	spriteWidth = image.GetWidth() / frameCount;
	spriteHeight = image.GetHeight();
	
	Logger::FrameworkDebug("* frameCount: %d spriteWidth: %d spriteHeight: %d", frameCount, spriteWidth, spriteHeight);

	frameRects = new Rect2i[frameCount];
    frameNames.resize(frameCount);
	for (int k = 0; k < frameCount; ++k)
	{
		PngImageExt frameX;
		frameX.Create(spriteWidth, spriteHeight);
		frameX.DrawImage(0, 0, &image, Rect2i(k * spriteWidth, 0, spriteWidth, spriteHeight));
		
		
		Rect2i reducedRect;
		frameX.FindNonOpaqueRect(reducedRect);
		Logger::FrameworkDebug("%s - reduced_rect(%d %d %d %d)", nameWithoutExt.c_str(), reducedRect.x, reducedRect.y, reducedRect.dx, reducedRect.dy);
		
		PngImageExt frameX2;
		frameX2.Create(reducedRect.dx, reducedRect.dy);
		frameX2.DrawImage(0, 0, &frameX, reducedRect);
		
		FilePath fileWrite = FramePathHelper::GetFramePathAbsolute(pathToProcess, nameWithoutExt, k);
		frameX2.Write(fileWrite);		
	
		frameRects[k].x = reducedRect.x;
		frameRects[k].y = reducedRect.y;
		frameRects[k].dx = reducedRect.dx;
		frameRects[k].dy = reducedRect.dy;
	}
	

	fclose(fp);
	return true;
}

bool DefinitionFile::Load(const FilePath& _filename)
{
	filename = _filename;
	FILE * fp = fopen(filename.GetAbsolutePathname().c_str(), "rt");
	if (!fp)
	{
		Logger::Error("*** ERROR: Can't open definition file: %s",filename.GetAbsolutePathname().c_str());
		return false;
	}
	fscanf(fp, "%d %d", &spriteWidth, &spriteHeight);
	fscanf(fp, "%d", &frameCount);
	
	frameRects = new Rect2i[frameCount];
	
	for (int i = 0; i < frameCount; ++i)
	{
        char frameName[128];
		fscanf(fp, "%d %d %d %d %s\n", &frameRects[i].x, &frameRects[i].y, &frameRects[i].dx, &frameRects[i].dy, frameName);
		Logger::FrameworkDebug("[DefinitionFile] frame: %d w: %d h: %d", i, frameRects[i].dx, frameRects[i].dy);
        frameNames[i] = String(frameName);
	}
	
	while(1)
	{
		char tmpString[512];
		fgets(tmpString, sizeof(tmpString), fp);
		pathsInfo.push_back(tmpString);
		printf("str: %s\n", tmpString);
		if (feof(fp))break;
	}
	
	
	fclose(fp);
	Logger::FrameworkDebug("Loaded definition: %s frames: %d",filename.GetAbsolutePathname().c_str(), frameCount);
	
	return true;
}


DAVA::Size2i DefinitionFile::GetFrameSize(int frame) const
{
	return Size2i(frameRects[frame].dx, frameRects[frame].dy);
}


int DefinitionFile::GetFrameWidth(int frame) const
{
	return frameRects[frame].dx;
}

int DefinitionFile::GetFrameHeight(int frame) const
{
	return frameRects[frame].dy;
}


};

