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

#ifndef __VisibilityCheckComponent_h__
#define __VisibilityCheckComponent_h__

#include "Math/Color.h"
#include "Entity/Component.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class VisibilityCheckComponent : public Component
{
public:
    IMPLEMENT_COMPONENT_TYPE(VISIBILITY_CHECK_COMPONENT);

public:
    VisibilityCheckComponent();
    Component* Clone(Entity* toEntity) override;

    float GetRadius() const;
    void SetRadius(float);

    float GetDistanceBetweenPoints() const;
    void SetDistanceBetweenPoints(float);

    Color GetColor() const;
    void SetColor(const Color&);

    bool IsPointSetValid() const;
    void InvalidatePointSet();
    void BuildPointSet();

    const Vector<Vector3>& GetPoints() const;

private:
    Vector<Vector3> points;
    Color color;
    float radius = 5.0f;
    float distanceBetweenPoints = 2.0f;
    bool shouldBuildPointSet = true;

public:
    INTROSPECTION_EXTEND(VisibilityCheckComponent, Component,
                         PROPERTY("radius", "Radius", GetRadius, SetRadius, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("distanceBetweenPoints", "Distance between points", GetDistanceBetweenPoints, SetDistanceBetweenPoints, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("color", "Color", GetColor, SetColor, I_SAVE | I_VIEW | I_EDIT))
};
}

#endif
