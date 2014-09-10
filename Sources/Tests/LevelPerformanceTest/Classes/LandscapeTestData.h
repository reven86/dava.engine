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


#ifndef __LEVEL_PERFORMANCE_TEST_LANDSCAPE_TEST_DATA_H__
#define __LEVEL_PERFORMANCE_TEST_LANDSCAPE_TEST_DATA_H__

#include "DAVAEngine.h"

#define SECTORS_COUNT 8

struct FpsStatItem
{
    DAVA::float32 avFps[SECTORS_COUNT];
	DAVA::Rect rect;
	
	FpsStatItem()
	{
	}
};

class LandscapeTestData
{
private:
	DAVA::Rect landscapeRect;
    DAVA::Vector<FpsStatItem> stat;

	DAVA::uint32 textureMemorySize;
	DAVA::uint32 textureFilesSize;
	DAVA::FilePath sceneFilePath;

public:
	void SetLandscapeRect(const DAVA::Rect& rect);
	void AddStatItem(const FpsStatItem& item);

	void SetTextureMemorySize(DAVA::uint32 size);
	void SetTexturesFilesSize(DAVA::uint32 size);
	void SetSceneFilePath(const DAVA::FilePath & path);
	DAVA::uint32 GetTextureMemorySize() const;
	DAVA::uint32 GetTexturesFilesSize() const;
	const DAVA::FilePath & GetSceneFilePath() const;

	const DAVA::Rect& GetLandscapeRect() const;
	DAVA::uint32 GetItemCount() const;
	const FpsStatItem& GetItem(DAVA::uint32 index) const;

	void Clear();
	DAVA::Rect TranslateRect(const DAVA::Rect& rect, const DAVA::Rect& destPlane) const;
	DAVA::Vector2 TranslatePoint(const DAVA::Vector2& point, const DAVA::Rect& destPlane) const;
};

#endif
