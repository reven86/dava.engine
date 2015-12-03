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

    bool IsEnabled() const;
    void SetEnabled(bool);

    bool ShoouldNormalizeColor() const;
    void SetShoouldNormalizeColor(bool);

    float GetRadius() const;
    void SetRadius(float);

    float GetDistanceBetweenPoints() const;
    void SetDistanceBetweenPoints(float);

    float GetMaximumDistance() const;
    void SetMaximumDistance(float);

    float GetVerticalVariance() const;
    void SetVerticalVariance(float);

    const Color& GetColor() const;
    void SetColor(const Color&);

    float GetUpAngle() const;
    void SetUpAngle(float);

    float GetDownAngle() const;
    void SetDownAngle(float);

    Color GetNormalizedColor() const;

    bool ShouldPlaceOnLandscape() const;
    void SetShouldPlaceOnLandscape(bool);

    float GetHeightAboveLandscape() const;
    void SetHeightAboveLandscape(float);

    bool IsPointSetValid() const;
    void InvalidatePointSet();
    void BuildPointSet();

    const Vector<Vector3>& GetPoints() const;

    bool IsValid() const;
    void SetValid();

private:
    Vector<Vector3> points;
    Color color = Color(1.0f, 0.0f, 0.0f, 1.0f);
    float radius = 20.0f;
    float distanceBetweenPoints = 4.0f;
    float upAngle = 45.0f;
    float downAngle = 45.0f;
    float verticalVariance = 0.0f;
    float maximumDistance = 250.0f;
    float heightAboveLandscape = 3.0f;
    bool shouldBuildPointSet = true;
    bool isValid = false;
    bool isEnabled = true;
    bool shouldNormalizeColor = true;
    bool shouldPlaceOnLandscape = true;

public:
    INTROSPECTION_EXTEND(VisibilityCheckComponent, Component,
                         PROPERTY("Enabled", "Enabled", IsEnabled, SetEnabled, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("Radius", "Radius", GetRadius, SetRadius, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("Distance Between Points", "Minimal distance between points", GetDistanceBetweenPoints, SetDistanceBetweenPoints, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("Maximum Distance", "Maximum distance to check", GetMaximumDistance, SetMaximumDistance, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("Up Angle", "Up Angle", GetUpAngle, SetUpAngle, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("Down Angle", "Down Angle", GetDownAngle, SetDownAngle, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("Vertical Variance", "Vertical Variance", GetVerticalVariance, SetVerticalVariance, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("Color", "Color", GetColor, SetColor, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("Normalize Color", "If enabled scales overlay's color to match current color.", ShoouldNormalizeColor, SetShoouldNormalizeColor, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("Place on Landscape", "Snaps each point to landscape", ShouldPlaceOnLandscape, SetShouldPlaceOnLandscape, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("Height Above the Landscape", "Distance from landscape to each point", GetHeightAboveLandscape, SetHeightAboveLandscape, I_SAVE | I_VIEW | I_EDIT))
};
}

#endif
