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


#include "RulerToolLandscape.h"

#include "LandscapeRenderer.h"
#include "EditorHeightmap.h"

using namespace DAVA;

RulerToolLandscape::RulerToolLandscape()
    : EditorLandscape()
{
    // RETURN TO THIS CODE LATER
    //SetName(String("Landscape_RulerTool"));

    rulerSprite = Sprite::CreateAsRenderTarget((float32)TEXTURE_TILE_FULL_SIZE, (float32)TEXTURE_TILE_FULL_SIZE, DAVA::FORMAT_RGBA8888);
//    rulerSprite->GetTexture()->GenerateMipmaps();
}

RulerToolLandscape::~RulerToolLandscape()
{
    SafeRelease(rulerSprite);
}


void RulerToolLandscape::SetPoints(const DAVA::List<DAVA::Vector3> &points)
{
    EditorHeightmap *editorHeightmap = dynamic_cast<EditorHeightmap *>(heightmap);
    if(editorHeightmap)
    {
        HeihghtmapUpdated(Rect(0, 0, (float32)editorHeightmap->Size() - 1.f, (float32)editorHeightmap->Size() - 1.f));
    }

    
    Texture *targetTexture = rulerSprite->GetTexture();
    
    RenderManager::Instance()->SetRenderTarget(rulerSprite);

    Rect drawRect(0, 0, (float32)targetTexture->GetWidth(), (float32)targetTexture->GetHeight());
    RenderManager::Instance()->ClipPush();
    RenderManager::Instance()->ClipRect(drawRect);

    DrawFullTiledTexture(targetTexture, drawRect);
    
    if(1 < points.size())
    {
        Color red(1.0f, 0.0f, 0.0f, 1.0f);
        RenderManager::Instance()->SetColor(red);

        AABBox3 boundingBox = nestedLandscape->GetBoundingBox();
        Vector3 landSize = boundingBox.max - boundingBox.min;
        Vector3 offsetPoint = boundingBox.min;
        
        float32 koef = (float32)targetTexture->GetWidth() / landSize.x;
        
        List<Vector3>::const_iterator it = points.begin();
        List<Vector3>::const_iterator endIt = points.end();

        Vector3 startPoint = *it;
        ++it;
        for( ; it != endIt; ++it)
        {
            
            Vector3 endPoint = *it;

            Vector3 startPosition = (startPoint - offsetPoint) * koef;
            Vector3 endPosition = (endPoint - offsetPoint) * koef;
            RenderHelper::Instance()->DrawLine(Vector2(startPosition.x, startPosition.y), Vector2(endPosition.x, endPosition.y));
            
            startPoint = endPoint;
        }
        

        RenderManager::Instance()->ResetColor();
    }
    

    RenderManager::Instance()->ClipPop();
    
    RenderManager::Instance()->RestoreRenderTarget();
    
    if(parentLandscape)
    {
        parentLandscape->HeihghtmapUpdated(Rect(0, 0, (float32)editorHeightmap->Size() - 1.f, (float32)editorHeightmap->Size() - 1.f));
    }
}


void RulerToolLandscape::SetDisplayedTexture()
{
    SetTexture(Landscape::TEXTURE_TILE_FULL, rulerSprite->GetTexture());
}

Texture * RulerToolLandscape::GetDisplayedTexture()
{
    return rulerSprite->GetTexture();
}
