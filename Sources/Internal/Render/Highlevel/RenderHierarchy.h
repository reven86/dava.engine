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


#ifndef __DAVAENGINE_RENDER_HIERARCHY_H__
#define	__DAVAENGINE_RENDER_HIERARCHY_H__

#include "Base/BaseTypes.h"
#include "Base/HashMap.h"
#include "Base/FastNameMap.h"
#include "Render/UniqueStateSet.h"
//#include "Render/Highlevel/LayerSetUniqueHandler.h"
#include "Base/BaseMath.h"

namespace DAVA
{
class RenderObject;
class Camera;
class RenderHierarchy
{
public:    
	virtual ~RenderHierarchy(){};
    
    virtual void AddRenderObject(RenderObject * renderObject) = 0;
    virtual void RemoveRenderObject(RenderObject * renderObject) = 0;
	virtual void ObjectUpdated(RenderObject * renderObject) = 0;
    virtual void Clip(Camera* camera, Vector<RenderObject*>& visibilityArray, uint32 visibilityCriteria) = 0;

    virtual void GetAllObjectsInBBox(const AABBox3& bbox, Vector<RenderObject*>& visibilityArray) = 0;

    virtual void Initialize(){};
	virtual void Update(){};
	virtual void DebugDraw(const Matrix4 & cameraMatrix){};
};

class LinearRenderHierarchy : public RenderHierarchy
{
	virtual void AddRenderObject(RenderObject * renderObject);
	virtual void RemoveRenderObject(RenderObject * renderObject);
	virtual void ObjectUpdated(RenderObject * renderObject);
    virtual void Clip(Camera* camera, Vector<RenderObject*>& visibilityArray, uint32 visibilityCriteria);
    virtual void GetAllObjectsInBBox(const AABBox3& bbox, Vector<RenderObject*>& visibilityArray);

private:
    Vector<RenderObject*> renderObjectArray;
};
    
} // ns

#endif	/* __DAVAENGINE_RENDER_HIERARCHY_H__ */

