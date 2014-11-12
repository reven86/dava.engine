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


#ifndef __DAVAENGINE_SPEEDTREE_OBJECT_H__
#define __DAVAENGINE_SPEEDTREE_OBJECT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Render/Highlevel/Mesh.h"

namespace DAVA 
{

class SpeedTreeUpdateSystem;
class SpeedTreeObject: public RenderObject
{
public:

    SpeedTreeObject();
    virtual ~SpeedTreeObject();

    virtual void RecalcBoundingBox();
    virtual RenderObject * Clone(RenderObject *newObject);
    virtual void Save(KeyedArchive *archive, SerializationContext *serializationContext);
    virtual void Load(KeyedArchive *archive, SerializationContext *serializationContext);

    static bool IsTreeLeafBatch(RenderBatch * batch);

    virtual void BindDynamicParameters(Camera * camera);

    void SetSphericalHarmonics(const Vector<Vector3> & coeffs);
    const Vector<Vector3> & GetSphericalHarmonics() const;

    //Interpolate between globally smoothed (0.0) and locally smoothed (1.0) leafs lighting
    void SetLightSmoothing(const float32 & smooth);
    const float32 & GetLightSmoothing() const;

protected:
    static const FastName FLAG_WIND_ANIMATION;

    AABBox3 CalcBBoxForSpeedTreeGeometry(RenderBatch * rb);

    void SetTreeAnimationParams(const Vector2 & trunkOscillationParams, const Vector2 & leafOscillationParams);
    void UpdateAnimationFlag(int32 maxAnimatedLod);

    Vector2 trunkOscillation;
    Vector2 leafOscillation;

    Vector<Vector3> sphericalHarmonics;
    float32 lightSmoothing;

public:
    INTROSPECTION_EXTEND(SpeedTreeObject, RenderObject,
        PROPERTY("lightSmoothing", "Light Smoothing", GetLightSmoothing, SetLightSmoothing, I_SAVE | I_EDIT | I_VIEW)
        );

friend class SpeedTreeUpdateSystem;
};

};

#endif // __DAVAENGINE_SPEEDTREE_OBJECT_H__
