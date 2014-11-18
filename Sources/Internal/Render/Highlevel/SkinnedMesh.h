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


#ifndef __DAVAENGINE_SKINNED_MESH_H__
#define	__DAVAENGINE_SKINNED_MESH_H__

#include "Base/BaseTypes.h"
#include "Animation/AnimatedObject.h"
#include "Base/BaseMath.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA    
{

class PolygonGroup;
class RenderBatch;
class ShadowVolume;
class NMaterial;

class SkinnedMesh : public RenderObject
{

public:
    SkinnedMesh();
    
    
    virtual RenderObject * Clone(RenderObject *newObject);

    
    virtual void RecalcBoundingBox(){}    
    virtual void BindDynamicParameters(Camera * camera);
    
    inline void SetObjectSpaceBoundingBox(const AABBox3& box);
    inline void SetJointsPtr(Vector4 *positionPtr, Vector4 *quaternoinPtr, int32 count);
protected:
    Vector4 *positionArray;
    Vector4 *quaternionArray;
    int32 jointsCount;
};


inline void SkinnedMesh::SetJointsPtr(Vector4 *positionPtr, Vector4 *quaternoinPtr, int32 count)
{
    positionArray = positionPtr;
    quaternionArray = quaternoinPtr;
    jointsCount = count;
}

inline void SkinnedMesh::SetObjectSpaceBoundingBox(const AABBox3& box)
{
    bbox = box;
}

}//ns

#endif