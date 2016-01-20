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

#include "VisibilityCheckComponent.h"
#include "Scene3D/Entity.h"
#include "Render/Texture.h"
#include "Utils/Random.h"

using namespace DAVA;

REGISTER_CLASS(VisibilityCheckComponent)

VisibilityCheckComponent::VisibilityCheckComponent()
{
}

Component* VisibilityCheckComponent::Clone(Entity* toEntity)
{
    auto result = new VisibilityCheckComponent();
    result->SetEntity(toEntity);

    result->color = color;
    result->radius = radius;
    result->distanceBetweenPoints = distanceBetweenPoints;
    result->upAngle = upAngle;
    result->downAngle = downAngle;
    result->verticalVariance = verticalVariance;
    result->maximumDistance = maximumDistance;
    result->heightAboveLandscape = heightAboveLandscape;
    result->isEnabled = isEnabled;
    result->shouldNormalizeColor = shouldNormalizeColor;
    result->shouldPlaceOnLandscape = shouldPlaceOnLandscape;
    result->shouldRebuildPointSet = true;
    result->isValid = false;

    return result;
}

float32 VisibilityCheckComponent::GetRadius() const
{
    return radius;
}

void VisibilityCheckComponent::SetRadius(float32 r)
{
    radius = r;
    shouldRebuildPointSet = true;
    Invalidate();
}

float32 VisibilityCheckComponent::GetDistanceBetweenPoints() const
{
    return distanceBetweenPoints;
}

void VisibilityCheckComponent::SetDistanceBetweenPoints(float32 d)
{
    distanceBetweenPoints = d;
    shouldRebuildPointSet = true;
    Invalidate();
}

const Color& VisibilityCheckComponent::GetColor() const
{
    return color;
}

void VisibilityCheckComponent::SetColor(const Color& clr)
{
    color = clr;
    color.a = 1.0f;
    Invalidate();
}

bool VisibilityCheckComponent::ShouldRebuildPoints() const
{
    return shouldRebuildPointSet;
}

bool VisibilityCheckComponent::IsValid() const
{
    return isValid;
}

void VisibilityCheckComponent::SetValid()
{
    isValid = true;
    shouldRebuildPointSet = false;
}

void VisibilityCheckComponent::Invalidate()
{
    isValid = false;
}

float32 VisibilityCheckComponent::GetUpAngle() const
{
    return upAngle;
}

void VisibilityCheckComponent::SetUpAngle(float32 value)
{
    upAngle = std::max(0.0f, std::min(90.0f, value));
    Invalidate();
}

float32 VisibilityCheckComponent::GetDownAngle() const
{
    return downAngle;
}

void VisibilityCheckComponent::SetDownAngle(float32 value)
{
    downAngle = std::max(0.0f, std::min(90.0f, value));
    Invalidate();
}

bool VisibilityCheckComponent::IsEnabled() const
{
    return isEnabled;
}

void VisibilityCheckComponent::SetEnabled(bool value)
{
    isEnabled = value;
    shouldRebuildPointSet = true;
    Invalidate();
}

bool VisibilityCheckComponent::ShouldNormalizeColor() const
{
    return shouldNormalizeColor;
}

void VisibilityCheckComponent::SetShouldNormalizeColor(bool value)
{
    shouldNormalizeColor = value;
}

float32 VisibilityCheckComponent::GetVerticalVariance() const
{
    return verticalVariance;
}

void VisibilityCheckComponent::SetVerticalVariance(float32 value)
{
    verticalVariance = std::max(0.0f, value);
    shouldRebuildPointSet = true;
    Invalidate();
}

float32 VisibilityCheckComponent::GetMaximumDistance() const
{
    return maximumDistance;
}

void VisibilityCheckComponent::SetMaximumDistance(float32 value)
{
    maximumDistance = std::max(0.0f, value);
}

bool VisibilityCheckComponent::ShouldPlaceOnLandscape() const
{
    return shouldPlaceOnLandscape;
}

void VisibilityCheckComponent::SetShouldPlaceOnLandscape(bool value)
{
    shouldPlaceOnLandscape = value;
    Invalidate();
}

float32 VisibilityCheckComponent::GetHeightAboveLandscape() const
{
    return heightAboveLandscape;
}

void VisibilityCheckComponent::SetHeightAboveLandscape(float32 value)
{
    heightAboveLandscape = DAVA::Max(0.0f, value);
    Invalidate();
}

void VisibilityCheckComponent::Serialize(DAVA::KeyedArchive* archive, DAVA::SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    archive->SetColor("vsc.color", color);
    archive->SetFloat("vsc.radius", radius);
    archive->SetFloat("vsc.distanceBetweenPoints", distanceBetweenPoints);
    archive->SetFloat("vsc.upAngle", upAngle);
    archive->SetFloat("vsc.downAngle", downAngle);
    archive->SetFloat("vsc.verticalVariance", verticalVariance);
    archive->SetFloat("vsc.maximumDistance", maximumDistance);
    archive->SetFloat("vsc.heightAboveLandscape", heightAboveLandscape);
    archive->SetBool("vsc.isEnabled", isEnabled);
    archive->SetBool("vsc.shouldNormalizeColor", shouldNormalizeColor);
    archive->SetBool("vsc.shouldPlaceOnLandscape", shouldPlaceOnLandscape);
}

void VisibilityCheckComponent::Deserialize(DAVA::KeyedArchive* archive, DAVA::SerializationContext* serializationContext)
{
    color = archive->GetColor("vsc.color");
    radius = archive->GetFloat("vsc.radius");
    distanceBetweenPoints = archive->GetFloat("vsc.distanceBetweenPoints");
    upAngle = archive->GetFloat("vsc.upAngle");
    downAngle = archive->GetFloat("vsc.downAngle");
    verticalVariance = archive->GetFloat("vsc.verticalVariance");
    maximumDistance = archive->GetFloat("vsc.maximumDistance");
    heightAboveLandscape = archive->GetFloat("vsc.heightAboveLandscape");
    isEnabled = archive->GetBool("vsc.isEnabled");
    shouldNormalizeColor = archive->GetBool("vsc.shouldNormalizeColor");
    shouldPlaceOnLandscape = archive->GetBool("vsc.shouldPlaceOnLandscape");
    shouldRebuildPointSet = true;
    Invalidate();
}
