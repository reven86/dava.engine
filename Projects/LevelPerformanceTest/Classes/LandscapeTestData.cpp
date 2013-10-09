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


#include "LandscapeTestData.h"

using namespace DAVA;

void LandscapeTestData::AddStatItem(const FpsStatItem &item)
{
	stat.push_back(item);
}

void LandscapeTestData::SetLandscapeRect(const Rect &rect)
{
	landscapeRect = rect;
}

const Rect& LandscapeTestData::GetLandscapeRect() const
{
	return landscapeRect;
}

uint32 LandscapeTestData::GetItemCount() const
{
	return stat.size();
}

const FpsStatItem& LandscapeTestData::GetItem(uint32 index) const
{
	DVASSERT(index < stat.size());
	return stat[index];
}

void LandscapeTestData::Clear()
{
	stat.clear();
}

Rect LandscapeTestData::TranslateRect(const Rect &rect, const Rect& destPlane) const
{
	DVASSERT(landscapeRect.dx != 0 && landscapeRect.dy !=0);
	
	Vector2 origPlaneSize = landscapeRect.GetSize();
	Vector2 destPlaneSize = destPlane.GetSize();

	Vector2 scale(destPlaneSize.x / origPlaneSize.x,
				   destPlaneSize.y / origPlaneSize.y);

	Vector2 relPos = rect.GetPosition() - landscapeRect.GetPosition();

	Vector2 newRelPos(relPos.x * scale.x,
					  relPos.y * scale.y);

	Vector2 newPos = newRelPos + destPlane.GetPosition();

	Vector2 newSize(rect.GetSize().x * scale.x,
					rect.GetSize().y * scale.y);

	return Rect(newPos, newSize);
}

Vector2 LandscapeTestData::TranslatePoint(const Vector2& point, const DAVA::Rect& destPlane) const
{
	DVASSERT(landscapeRect.dx != 0 && landscapeRect.dy !=0);
	
	Vector2 origPlaneSize = landscapeRect.GetSize();
	Vector2 destPlaneSize = destPlane.GetSize();

	Vector2 scale(destPlaneSize.x / origPlaneSize.x,
				  destPlaneSize.y / origPlaneSize.y);
	
	Vector2 relPos = point - landscapeRect.GetPosition();

	Vector2 newRelPos(relPos.x * scale.x,
					  relPos.y * scale.y);

	Vector2 newPos = newRelPos + destPlane.GetPosition();

	return newPos;
}

void LandscapeTestData::SetTextureMemorySize(uint32 size)
{
	textureMemorySize = size;
}

void LandscapeTestData::SetSceneFilePath(const FilePath & path)
{
	sceneFilePath = path;
}

uint32 LandscapeTestData::GetTextureMemorySize() const
{
	return textureMemorySize;
}

const FilePath & LandscapeTestData::GetSceneFilePath() const
{
	return sceneFilePath;
}

void LandscapeTestData::SetTexturesFilesSize(uint32 size)
{
	textureFilesSize = size;
}

uint32 LandscapeTestData::GetTexturesFilesSize() const
{
	return textureFilesSize;
}