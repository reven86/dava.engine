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


#ifndef __EDITOR_LANDSCAPE_NODE_H__
#define __EDITOR_LANDSCAPE_NODE_H__

#include "DAVAEngine.h"

class LandscapeRenderer;
class EditorLandscape : public DAVA::Landscape
{
public:	
	EditorLandscape();
	virtual ~EditorLandscape();
    
    void SetNestedLandscape(DAVA::Landscape *landscapeNode);
    DAVA::Landscape *GetNestedLandscape();

    void SetParentLandscape(EditorLandscape *landscapeNode);
    EditorLandscape *GetParentLandscape();

    void SetRenderer(LandscapeRenderer *renderer);
    
	virtual void Draw(DAVA::Camera * camera);
    virtual void HeihghtmapUpdated(const DAVA::Rect &forRect);
    
    virtual void SetHeightmap(DAVA::Heightmap *height);

    LandscapeRenderer *GetRenderer();
    
    void FlushChanges();
    
	virtual DAVA::Texture * GetTexture(eTextureLevel level);
    virtual void BuildLandscapeFromHeightmapImage(const DAVA::FilePath & heightmapPathname, const DAVA::AABBox3 & landscapeBox);
    virtual void UpdateFullTiledTexture();

	virtual DAVA::RenderObject * Clone(DAVA::RenderObject *newObject);


protected:
    
    void DrawFullTiledTexture(DAVA::Texture *renderTarget, const DAVA::Rect &drawRect);

    void CopyCursorData(DAVA::Landscape *sourceLandscape, DAVA::Landscape *destinationLandscape);
    virtual void SetDisplayedTexture();
    virtual DAVA::Texture * GetDisplayedTexture();

    DAVA::Landscape *nestedLandscape;
    EditorLandscape *parentLandscape;
    
    LandscapeRenderer *landscapeRenderer;
};

    
#endif // __EDITOR_LANDSCAPE_NODE_H__

