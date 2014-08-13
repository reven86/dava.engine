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


#include "Render/2D/RenderSystem2D/VirtualCoordinatesTransformSystem.h"
#include "UI/UIControlSystem.h"
#include "Render/2D/TextBlock.h"
#include "Render/2D/Sprite.h"

namespace DAVA
{

VirtualCoordinatesTransformSystem::VirtualCoordinatesTransformSystem()
{
    fixedProportions = true;
    enabledReloadResourceOnResize = false;
    needTorecalculateMultipliers = false;
    
    desirableIndex = 0;
    
    virtualToPhysical = 0.f;
	physicalToVirtual = 0.f;
    
    physicalScreenHeight = 0;
    physicalScreenWidth = 0;
    virtualScreenHeight = 0;
    virtualScreenWidth = 0;
	requestedVirtualScreenWidth = 0;
	requestedVirtualScreenHeight = 0;
    
    EnableReloadResourceOnResize(true);
}

void VirtualCoordinatesTransformSystem::CalculateScaleMultipliers()
{
    needTorecalculateMultipliers = false;
    
    virtualScreenWidth = requestedVirtualScreenWidth;
    virtualScreenHeight = requestedVirtualScreenHeight;
    
    float32 w, h;
    w = (float32)virtualScreenWidth / (float32)physicalScreenWidth;
    h = (float32)virtualScreenHeight / (float32)physicalScreenHeight;
    drawOffset.x = drawOffset.y = 0;
    float32 desD = 10000.0f;
    if(w > h)
    {
        physicalToVirtual = w;
        virtualToPhysical = (float32)physicalScreenWidth / (float32)virtualScreenWidth;
        if (fixedProportions)
        {
            drawOffset.y = 0.5f * ((float32)physicalScreenHeight - (float32)virtualScreenHeight * virtualToPhysical);
        }
        else
        {
            virtualScreenHeight = physicalScreenHeight * physicalToVirtual;
        }
        for (int i = 0; i < (int)allowedSizes.size(); i++)
        {
            allowedSizes[i].toVirtual = (float32)virtualScreenWidth / (float32)allowedSizes[i].width;
            allowedSizes[i].toPhysical = (float32)physicalScreenWidth / (float32)allowedSizes[i].width;
            if (fabs(allowedSizes[i].toPhysical - 1.0f) < desD)
            {
                desD = fabsf(allowedSizes[i].toPhysical - 1.0f);
                desirableIndex = i;
            }
        }
    }
    else
    {
        physicalToVirtual = h;
        virtualToPhysical = (float32)physicalScreenHeight / (float32)virtualScreenHeight;
        if (fixedProportions)
        {
            drawOffset.x = 0.5f * ((float32)physicalScreenWidth - (float32)virtualScreenWidth * virtualToPhysical);
        }
        else
        {
            virtualScreenWidth = physicalScreenWidth * physicalToVirtual;
        }
        for (int i = 0; i < (int)allowedSizes.size(); i++)
        {
            allowedSizes[i].toVirtual = (float32)virtualScreenHeight / (float32)allowedSizes[i].height;
            allowedSizes[i].toPhysical = (float32)physicalScreenHeight / (float32)allowedSizes[i].height;
            if (fabs(allowedSizes[i].toPhysical - 1.0f) < desD)
            {
                desD = fabsf(allowedSizes[i].toPhysical - 1.0f);
                desirableIndex = i;
            }
        }
    }
    
    drawOffset.y = floorf(drawOffset.y);
    drawOffset.x = floorf(drawOffset.x);
    
    UIControlSystem::Instance()->CalculateScaleMultipliers();
    
    if(enabledReloadResourceOnResize)
    {
        Sprite::ValidateForSize();
        TextBlock::ScreenResolutionChanged();
    }
}
    
bool VirtualCoordinatesTransformSystem::NeedToRecalculateMultipliers()
{
    return needTorecalculateMultipliers;
}

void VirtualCoordinatesTransformSystem::EnableReloadResourceOnResize(bool enable)
{
    enabledReloadResourceOnResize = enable;
}
    
Vector2 VirtualCoordinatesTransformSystem::ConvertPhysicalToVirtual(const Vector2 & vector)
{
    return vector * physicalToVirtual;
}
Vector2 VirtualCoordinatesTransformSystem::ConvertVirtualToPhysical(const Vector2 & vector)
{
    return vector * virtualToPhysical;
}
Vector2 VirtualCoordinatesTransformSystem::ConvertResourceToVirtual(const Vector2 & vector, DAVA::int32 resourceIndex)
{
    DVASSERT(resourceIndex < (int32)allowedSizes.size());
    return vector * allowedSizes[resourceIndex].toVirtual;
}
Vector2 VirtualCoordinatesTransformSystem::ConvertResourceToPhysical(const Vector2 & vector, DAVA::int32 resourceIndex)
{
    DVASSERT(resourceIndex < (int32)allowedSizes.size());
    return vector * allowedSizes[resourceIndex].toPhysical;
}
Rect VirtualCoordinatesTransformSystem::ConvertPhysicalToVirtual(const Rect & rect)
{
    return ConvertRect(rect, physicalToVirtual);
}
Rect VirtualCoordinatesTransformSystem::ConvertVirtualToPhysical(const Rect & rect)
{
    return ConvertRect(rect, virtualToPhysical);
}
Rect VirtualCoordinatesTransformSystem::ConvertResourceToVirtual(const Rect & rect, int32 resourceIndex)
{
    DVASSERT(resourceIndex < (int32)allowedSizes.size());
    return ConvertRect(rect, allowedSizes[resourceIndex].toVirtual);
}
Rect VirtualCoordinatesTransformSystem::ConvertResourceToPhysical(const Rect & rect, int32 resourceIndex)
{
    DVASSERT(resourceIndex < (int32)allowedSizes.size());
    return ConvertRect(rect, allowedSizes[resourceIndex].toPhysical);
}
Rect VirtualCoordinatesTransformSystem::ConvertRect(const Rect & rect, float32 factor)
{
    Rect newRect(rect);
    newRect.x *= factor;
    newRect.y *= factor;
    newRect.dx *= factor;
    newRect.dy *= factor;
    
    return newRect;
}
const String& VirtualCoordinatesTransformSystem::GetResourceFolder(int32 resourceIndex)
{
    DVASSERT(resourceIndex < (int32)allowedSizes.size());
    return allowedSizes[resourceIndex].folderName;
}
int32 VirtualCoordinatesTransformSystem::GetDesirableResourceIndex()
{
    return desirableIndex;
}
int32 VirtualCoordinatesTransformSystem::GetBaseResourceIndex()
{
    return 0;
}
    
float32 VirtualCoordinatesTransformSystem::GetVirtualToPhysicalFactor()
{
    return virtualToPhysical;
}

float32 VirtualCoordinatesTransformSystem::GetPhysicalToVirtualFactor()
{
    return physicalToVirtual;
}
    
float32 VirtualCoordinatesTransformSystem::GetResourceToVirtualFactor(DAVA::int32 resourceIndex)
{
    DVASSERT(resourceIndex < (int32)allowedSizes.size());
    return allowedSizes[resourceIndex].toVirtual;
}
    
float32 VirtualCoordinatesTransformSystem::GetResourceToPhysicalFactor(DAVA::int32 resourceIndex)
{
    DVASSERT(resourceIndex < (int32)allowedSizes.size());
    return allowedSizes[resourceIndex].toPhysical;
}
    
const Vector2 & VirtualCoordinatesTransformSystem::GetPhysicalDrawOffset()
{
    return drawOffset;
}

void VirtualCoordinatesTransformSystem::SetPhysicalScreenSize(int32 width, int32 height)
{
    physicalScreenWidth = width;
    physicalScreenHeight = height;
    needTorecalculateMultipliers = true;
}

void VirtualCoordinatesTransformSystem::SetVirtualScreenSize(int32 width, int32 height)
{
    requestedVirtualScreenWidth = virtualScreenWidth = (float32)width;
    requestedVirtualScreenHeight = virtualScreenHeight = (float32)height;
    needTorecalculateMultipliers = true;
}

void VirtualCoordinatesTransformSystem::SetProportionsIsFixed( bool needFixed )
{
    fixedProportions = needFixed;
    needTorecalculateMultipliers = true;
}

void VirtualCoordinatesTransformSystem::RegisterAvailableResourceSize(int32 width, int32 height, const String &resourcesFolderName)
{
    VirtualCoordinatesTransformSystem::AvailableSize newSize;
    newSize.width = width;
    newSize.height = height;
    newSize.folderName = resourcesFolderName;
    
    allowedSizes.push_back(newSize);
}

void VirtualCoordinatesTransformSystem::UnregisterAllAvailableResourceSizes()
{
    allowedSizes.clear();
}

int32 VirtualCoordinatesTransformSystem::GetPhysicalScreenWidth()
{
    return physicalScreenWidth;
}
int32 VirtualCoordinatesTransformSystem::GetPhysicalScreenHeight()
{
    return physicalScreenHeight;
}

int32 VirtualCoordinatesTransformSystem::GetVirtualScreenWidth()
{
    return virtualScreenWidth;
}
int32 VirtualCoordinatesTransformSystem::GetVirtualScreenHeight()
{
    return virtualScreenHeight;
}

int32 VirtualCoordinatesTransformSystem::GetVirtualScreenXMin()
{
    return -(int32)Round(drawOffset.x * physicalToVirtual);
}

int32 VirtualCoordinatesTransformSystem::GetVirtualScreenXMax()
{
    return (int32)Round((float32)(physicalScreenWidth - drawOffset.x) * physicalToVirtual);
}

int32 VirtualCoordinatesTransformSystem::GetVirtualScreenYMin()
{
    return -(int32)Round(drawOffset.y * physicalToVirtual);
}

int32 VirtualCoordinatesTransformSystem::GetVirtualScreenYMax()
{
    return (int32)Round((float32)(physicalScreenHeight - drawOffset.y) * physicalToVirtual);
}
    
};
