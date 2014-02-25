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





#ifndef __RULER_TOOL_H__
#define __RULER_TOOL_H__

#include "DAVAEngine.h"

class LandscapesController;
class RulerToolLandscape;
class HeightmapNode;
class EditorBodyControl;
class EditorScene;
class RulerTool: public DAVA::BaseObject
{
    static const DAVA::int32 PREDEFINED_SIZE = 10;
    static const DAVA::int32 APPROXIMATION_COUNT = 10;
    static const DAVA::int32 RAY_TRACING_DISTANCE = 1000;
    
public:

    RulerTool(EditorBodyControl *parent);
	virtual ~RulerTool();
    
    bool EnableTool(EditorScene *scene);
    void DisableTool();
    
    bool Input(DAVA::UIEvent * touch);

protected:

    bool GetIntersectionPoint(const DAVA::Vector2 &touchPoint, DAVA::Vector3 &pointOnLandscape);
    
    void SetStartPoint(const DAVA::Vector3 &point);
    void AddPoint(const DAVA::Vector3 &point);

    
    DAVA::Vector3 LandscapePoint(const DAVA::Vector3 &point);
    
    DAVA::float32 GetLength(const DAVA::Vector3 &startPoint, const DAVA::Vector3 &endPoint);
    
    DAVA::List<DAVA::Vector3> linePoints;
    DAVA::float32 length;
    
    EditorScene *editorScene;
//    DAVA::LandscapeNode *landscape;
    DAVA::int32 landscapeSize;
    
    EditorBodyControl *parentControl;
    HeightmapNode *heightmapNode;
    RulerToolLandscape *rulerToolLandscape;
    LandscapesController *landscapesController;
};


#endif //__RULER_TOOL_H__
