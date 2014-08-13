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


#ifndef __DAVAENGINE_VIRTUAL_COORDINATES_SYSTEM_H__
#define __DAVAENGINE_VIRTUAL_COORDINATES_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Singleton.h"

namespace DAVA
{
    
class VirtualCoordinatesTransformSystem : public Singleton<VirtualCoordinatesTransformSystem>
{
    struct AvailableSize
	{
		AvailableSize()
        : width(0)
        , height(0)
        , toVirtual(0)
        , toPhysical(0)
		{
            
		}
		int32 width;
		int32 height;
		String folderName;
		float32 toVirtual;
		float32 toPhysical;
	};
    
public:
    VirtualCoordinatesTransformSystem();
    virtual ~VirtualCoordinatesTransformSystem() {};

    void SetVirtualScreenSize(int32 width, int32 height);
    void SetPhysicalScreenSize(int32 width, int32 height);
    
    int32 GetVirtualScreenWidth();
    int32 GetVirtualScreenHeight();
    int32 GetPhysicalScreenHeight();
    int32 GetPhysicalScreenWidth();
    
    int32 GetRequestedVirtualScreenWidth();
    int32 GetRequestedVirtualScreenHeight();
	int32 GetVirtualScreenXMin();
	int32 GetVirtualScreenXMax();
	int32 GetVirtualScreenYMin();
	int32 GetVirtualScreenYMax();
    
    float32 GetVirtualToPhysicalFactor();
    float32 GetPhysicalToVirtualFactor();
    Vector2 ConvertPhysicalToVirtual(const Vector2 & vector);
    Vector2 ConvertVirtualToPhysical(const Vector2 & vector);
    Vector2 ConvertResourceToVirtual(const Vector2 & vector, DAVA::int32 resourceIndex);
    Vector2 ConvertResourceToPhysical(const Vector2 & vector, DAVA::int32 resourceIndex);

    float32 GetResourceToVirtualFactor(DAVA::int32 resourceIndex);
    float32 GetResourceToPhysicalFactor(DAVA::int32 resourceIndex);
    Rect ConvertPhysicalToVirtual(const Rect & rect);
    Rect ConvertVirtualToPhysical(const Rect & rect);
    Rect ConvertResourceToVirtual(const Rect & rect, int32 resourceIndex);
    Rect ConvertResourceToPhysical(const Rect & rect, int32 resourceIndex);
    
    void SetProportionsIsFixed(bool needFixed);
	void RegisterAvailableResourceSize(int32 width, int32 height, const String &resourcesFolderName);
	void UnregisterAllAvailableResourceSizes();
	
	const Vector2 & GetPhysicalDrawOffset();
	
	const String & GetResourceFolder(int32 resourceIndex);
	int32 GetDesirableResourceIndex();
	int32 GetBaseResourceIndex();
    
	void CalculateScaleMultipliers();
	bool NeedToRecalculateMultipliers();
    void EnableReloadResourceOnResize(bool enable);
    
private:
    Rect ConvertRect(const Rect & rect, float32 factor);
    
    Vector<AvailableSize> allowedSizes;
    
	int32 physicalScreenWidth;
	int32 physicalScreenHeight;
	int32 virtualScreenWidth;
	int32 virtualScreenHeight;
	int32 requestedVirtualScreenWidth;
	int32 requestedVirtualScreenHeight;
	
	float32 virtualToPhysical;
    float32 physicalToVirtual;
	Vector2 drawOffset;
    
	int desirableIndex;
	bool fixedProportions;
    
	bool needTorecalculateMultipliers;
    bool enabledReloadResourceOnResize;
};
    
class VirtualCoordinates
{
public:
    static int32 GetVirtualScreenWidth()
    { return VirtualCoordinatesTransformSystem::Instance()->GetVirtualScreenWidth(); }
    
    static int32 GetVirtualScreenHeight()
    { return VirtualCoordinatesTransformSystem::Instance()->GetVirtualScreenHeight(); }
    
    static int32 GetPhysicalScreenHeight()
    { return VirtualCoordinatesTransformSystem::Instance()->GetPhysicalScreenHeight(); }
    
    static int32 GetPhysicalScreenWidth()
    { return VirtualCoordinatesTransformSystem::Instance()->GetPhysicalScreenWidth(); }
    
    
    static float32 GetVirtualToPhysicalFactor()
    { return VirtualCoordinatesTransformSystem::Instance()->GetVirtualToPhysicalFactor(); }
    
    static float32 GetPhysicalToVirtualFactor()
    { return VirtualCoordinatesTransformSystem::Instance()->GetPhysicalToVirtualFactor(); }
    
    static Vector2 ConvertPhysicalToVirtual(const Vector2 & vector)
    { return VirtualCoordinatesTransformSystem::Instance()->ConvertPhysicalToVirtual(vector); }
    
    static Vector2 ConvertVirtualToPhysical(const Vector2 & vector)
    { return VirtualCoordinatesTransformSystem::Instance()->ConvertVirtualToPhysical(vector); }
    
    
    static Vector2 ConvertResourceToVirtual(const Vector2 & vector, DAVA::int32 resourceIndex)
    { return VirtualCoordinatesTransformSystem::Instance()->ConvertResourceToVirtual(vector, resourceIndex); }
    
    static Vector2 ConvertResourceToPhysical(const Vector2 & vector, DAVA::int32 resourceIndex)
    { return VirtualCoordinatesTransformSystem::Instance()->ConvertResourceToPhysical(vector, resourceIndex); }
    
    static float32 GetResourceToVirtualFactor(DAVA::int32 resourceIndex)
    { return VirtualCoordinatesTransformSystem::Instance()->GetResourceToVirtualFactor(resourceIndex); }
    
    static float32 GetResourceToPhysicalFactor(DAVA::int32 resourceIndex)
    { return VirtualCoordinatesTransformSystem::Instance()->GetResourceToPhysicalFactor(resourceIndex); }
    
    static Rect ConvertPhysicalToVirtual(const Rect & rect)
    { return VirtualCoordinatesTransformSystem::Instance()->ConvertPhysicalToVirtual(rect); }
    
    static Rect ConvertVirtualToPhysical(const Rect & rect)
    { return VirtualCoordinatesTransformSystem::Instance()->ConvertVirtualToPhysical(rect); }
    
    static Rect ConvertResourceToVirtual(const Rect & rect, int32 resourceIndex)
    { return VirtualCoordinatesTransformSystem::Instance()->ConvertResourceToVirtual(rect, resourceIndex); }
    
    static Rect ConvertResourceToPhysical(const Rect & rect, int32 resourceIndex)
    { return VirtualCoordinatesTransformSystem::Instance()->ConvertResourceToPhysical(rect, resourceIndex); }
    
    static int32 GetRequestedVirtualScreenWidth()
    { return VirtualCoordinatesTransformSystem::Instance()->GetRequestedVirtualScreenWidth(); }
    
    static int32 GetRequestedVirtualScreenHeight()
    { return VirtualCoordinatesTransformSystem::Instance()->GetRequestedVirtualScreenHeight(); }
    
    static int32 GetVirtualScreenXMin()
    { return VirtualCoordinatesTransformSystem::Instance()->GetVirtualScreenXMin(); }
    
    static int32 GetVirtualScreenXMax()
    { return VirtualCoordinatesTransformSystem::Instance()->GetVirtualScreenXMax(); }
    
    static int32 GetVirtualScreenYMin()
    { return VirtualCoordinatesTransformSystem::Instance()->GetVirtualScreenYMin(); }
    
    static int32 GetVirtualScreenYMax()
    { return VirtualCoordinatesTransformSystem::Instance()->GetVirtualScreenYMax(); }
};
    
};
#endif