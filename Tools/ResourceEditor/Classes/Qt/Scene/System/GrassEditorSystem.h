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


#ifndef __RESOURCEEDITORQT__GRASSEDITORSYSTEM__
#define __RESOURCEEDITORQT__GRASSEDITORSYSTEM__

#include "LandscapeEditorSystem.h"
#include "LandscapeEditorDrawSystem.h"
#include "Render/Highlevel/Vegetation/VegetationRenderObject.h"

class Command2;

using namespace DAVA;

class GrassEditorSystem: public LandscapeEditorSystem
{
public:
    enum BrushMode
    {
        BRUSH_REPLACE       = 0x0,
        BRUSH_ADD_HEIGHT    = 0x1,
        BRUSH_ADD_DENSITY   = 0x2,
        BRUSH_ADD           = BRUSH_ADD_HEIGHT | BRUSH_ADD_DENSITY
    };

    enum BrushAffectType
    {
        AFFECT_NONE = 0x0,
        AFFECT_HEIGHT = 0x1,
        AFFECT_DENSITY = 0x2,
        AFFECT_ALL = 0xFF
    };

	GrassEditorSystem(Scene* scene);
	virtual ~GrassEditorSystem();

	void Update(DAVA::float32 timeElapsed);
	virtual void Input(DAVA::UIEvent *event);
    void ProcessCommand(const Command2 *command, bool redo);

    bool EnableGrassEdit(bool enable);

    void SetLayerVisible(uint8 layer, bool visible);
    bool IsLayerVisible(uint8 layer) const;

    void SetCurrentLayer(uint8 layer);
    uint8 GetCurrentLayer() const;

    void SetBrushHeight(uint8 height);
    uint8 GetBrushHeight() const;

    void SetBrushDensity(uint8 density);
    uint8 GetBrushDensity() const;

    void SetBrushMode(int mode);
    int GetBrushMode() const;

    void SetBrushAffect(int affect);
    int GetBrushAffect() const;

    DAVA::VegetationRenderObject *GetCurrentVegetationObject() const;

    static DAVA::Rect2i GetAffectedImageRect(DAVA::AABBox2 &area);
    
protected:
    bool inDrawState;

    DAVA::Vector2 curCursorPos;
    DAVA::Vector2 lastDrawPos;
    DAVA::AABBox2 affectedArea;
    
    int curBrushMode;
    int curBrushAffect;

    uint8 curHeight;
    uint8 curDensity;
    uint8 curLayer;

    DAVA::VegetationMap *vegetationMap;
    DAVA::VegetationMap *vegetationMapCopy;
    DAVA::VegetationRenderObject *curVegetation; 

    void UpdateCursorPos();
    void DrawGrass(DAVA::Vector2 pos);
    void DrawGrassEnd();
    void BuildGrassCopy(DAVA::AABBox2 area = DAVA::AABBox2());

    DAVA::VegetationRenderObject* SearchVegetation(DAVA::Entity *entity) const;
};

#endif /* defined(__RESOURCEEDITORQT__GRASSEDITORSYSTEM__) */
