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
    
class VirtualCoordinatesSystem : public Singleton<VirtualCoordinatesSystem>
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
    VirtualCoordinatesSystem();
    virtual ~VirtualCoordinatesSystem() {};

    void SetVirtualScreenSize(int32 width, int32 height);
    void SetPhysicalScreenSize(int32 width, int32 height);
    void SetInputScreenAreaSize(int32 width, int32 height);
    
    inline const Size2i & GetVirtualScreenSize() const;
    inline const Size2i & GetRequestedVirtualScreenSize() const;
    inline const Size2i & GetPhysicalScreenSize() const;
    inline const Rect & GetFullVirtualScreenRect() const;
    
    inline Vector2 ConvertPhysicalToVirtual(const Vector2 & vector) const;
    inline Vector2 ConvertVirtualToPhysical(const Vector2 & vector) const;
    inline Vector2 ConvertResourceToVirtual(const Vector2 & vector, DAVA::int32 resourceIndex) const;
    inline Vector2 ConvertResourceToPhysical(const Vector2 & vector, DAVA::int32 resourceIndex) const;
    inline Vector2 ConvertInputToVirtual(const Vector2 & vector) const;
    inline Vector2 ConvertInputToPhysical(const Vector2 & vector) const;

    inline Rect ConvertPhysicalToVirtual(const Rect & rect) const;
    inline Rect ConvertVirtualToPhysical(const Rect & rect) const;
    inline Rect ConvertResourceToVirtual(const Rect & rect, int32 resourceIndex) const;
    inline Rect ConvertResourceToPhysical(const Rect & rect, int32 resourceIndex) const;
    
	inline const String & GetResourceFolder(int32 resourceIndex) const;
	inline int32 GetDesirableResourceIndex() const;
	inline int32 GetBaseResourceIndex() const;
    
	inline bool WasScreenSizeChanged() const;
	void ScreenSizeChanged();
    void EnableReloadResourceOnResize(bool enable);
    
    void SetProportionsIsFixed(bool needFixed);
	void RegisterAvailableResourceSize(int32 width, int32 height, const String &resourcesFolderName);
	void UnregisterAllAvailableResourceSizes();
    
	inline const Vector2 & GetPhysicalDrawOffset() const;
    
	inline Vector2 GetInputOffset() const;
	inline float32 GetInputScaleFactor() const;
    
    //TODO: cut this
    float32 GetVirtualToPhysicalFactor() const;
    float32 GetPhysicalToVirtualFactor() const;
    float32 GetResourceToVirtualFactor(DAVA::int32 resourceIndex) const;
    float32 GetResourceToPhysicalFactor(DAVA::int32 resourceIndex) const;
    
private:
    inline Rect ConvertRect(const Rect & rect, float32 factor) const;
    
    Vector<AvailableSize> allowedSizes;
	
    Size2i virtualScreenSize;
    Size2i requestedVirtualScreenSize;
    Size2i physicalScreenSize;
    Rect fullVirtualScreenRect;
    
	float32 virtualToPhysical;
    float32 physicalToVirtual;
	Vector2 drawOffset;
    
	int desirableIndex;
	bool fixedProportions;
    
	bool wasScreenResized;
    bool enabledReloadResourceOnResize;
    
    Size2i inputAreaSize;
	float32 inputScaleFactor;
	Vector2 inputOffset;
};
    
class ScreenSizes
{
public:
    static const Size2i & GetVirtualScreenSize()
    { return VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize(); }
    
    static const Size2i & GetRequestedVirtualScreenSize()
    { return VirtualCoordinatesSystem::Instance()->GetRequestedVirtualScreenSize(); }
    
    static const Size2i & GetPhysicalScreenSize()
    { return VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize(); }
    
    static const Rect & GetFullVirtualScreenRect()
    { return VirtualCoordinatesSystem::Instance()->GetFullVirtualScreenRect(); }
};
    
class VirtualCoordinates
{
public:
    static float32 GetVirtualToPhysicalFactor()
    { return VirtualCoordinatesSystem::Instance()->GetVirtualToPhysicalFactor(); }
    
    static float32 GetPhysicalToVirtualFactor()
    { return VirtualCoordinatesSystem::Instance()->GetPhysicalToVirtualFactor(); }
    
    static Vector2 ConvertPhysicalToVirtual(const Vector2 & vector)
    { return VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtual(vector); }
    
    static Vector2 ConvertVirtualToPhysical(const Vector2 & vector)
    { return VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(vector); }
    
    
    static Vector2 ConvertResourceToVirtual(const Vector2 & vector, DAVA::int32 resourceIndex)
    { return VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtual(vector, resourceIndex); }
    
    static Vector2 ConvertResourceToPhysical(const Vector2 & vector, DAVA::int32 resourceIndex)
    { return VirtualCoordinatesSystem::Instance()->ConvertResourceToPhysical(vector, resourceIndex); }
    
    static float32 GetResourceToVirtualFactor(DAVA::int32 resourceIndex)
    { return VirtualCoordinatesSystem::Instance()->GetResourceToVirtualFactor(resourceIndex); }
    
    static float32 GetResourceToPhysicalFactor(DAVA::int32 resourceIndex)
    { return VirtualCoordinatesSystem::Instance()->GetResourceToPhysicalFactor(resourceIndex); }
    
    static Rect ConvertPhysicalToVirtual(const Rect & rect)
    { return VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtual(rect); }
    
    static Rect ConvertVirtualToPhysical(const Rect & rect)
    { return VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(rect); }
    
    static Rect ConvertResourceToVirtual(const Rect & rect, int32 resourceIndex)
    { return VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtual(rect, resourceIndex); }
    
    static Rect ConvertResourceToPhysical(const Rect & rect, int32 resourceIndex)
    { return VirtualCoordinatesSystem::Instance()->ConvertResourceToPhysical(rect, resourceIndex); }
    
    
    static Vector2 ConvertInputToVirtual(const Vector2 & point)
    { return VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(point); }
    
    static Vector2 ConvertInputToPhysical(const Vector2 & point)
    { return VirtualCoordinatesSystem::Instance()->ConvertInputToPhysical(point); }
};
    
inline const Size2i & VirtualCoordinatesSystem::GetVirtualScreenSize() const
{
    return virtualScreenSize;
}
    
inline const Size2i & VirtualCoordinatesSystem::GetRequestedVirtualScreenSize() const
{
    return requestedVirtualScreenSize;
}
    
inline const Size2i & VirtualCoordinatesSystem::GetPhysicalScreenSize() const
{
    return physicalScreenSize;
}
    
inline const Rect & VirtualCoordinatesSystem::GetFullVirtualScreenRect() const
{
    return fullVirtualScreenRect;
}

inline Vector2 VirtualCoordinatesSystem::ConvertPhysicalToVirtual(const Vector2 & vector) const
{
    return vector * physicalToVirtual;
}
    
inline Vector2 VirtualCoordinatesSystem::ConvertVirtualToPhysical(const Vector2 & vector) const
{
    return vector * virtualToPhysical;
}
    
inline Vector2 VirtualCoordinatesSystem::ConvertResourceToVirtual(const Vector2 & vector, DAVA::int32 resourceIndex) const
{
    DVASSERT(resourceIndex < (int32)allowedSizes.size());
    return vector * allowedSizes[resourceIndex].toVirtual;
}
    
inline Vector2 VirtualCoordinatesSystem::ConvertResourceToPhysical(const Vector2 & vector, DAVA::int32 resourceIndex) const
{
    DVASSERT(resourceIndex < (int32)allowedSizes.size());
    return vector * allowedSizes[resourceIndex].toPhysical;
}
    
inline Rect VirtualCoordinatesSystem::ConvertPhysicalToVirtual(const Rect & rect) const
{
    return ConvertRect(rect, physicalToVirtual);
}
    
inline Rect VirtualCoordinatesSystem::ConvertVirtualToPhysical(const Rect & rect) const
{
    return ConvertRect(rect, virtualToPhysical);
}
    
inline Rect VirtualCoordinatesSystem::ConvertResourceToVirtual(const Rect & rect, int32 resourceIndex) const
{
    DVASSERT(resourceIndex < (int32)allowedSizes.size());
    return ConvertRect(rect, allowedSizes[resourceIndex].toVirtual);
}
    
inline Rect VirtualCoordinatesSystem::ConvertResourceToPhysical(const Rect & rect, int32 resourceIndex) const
{
    DVASSERT(resourceIndex < (int32)allowedSizes.size());
    return ConvertRect(rect, allowedSizes[resourceIndex].toPhysical);
}
    
inline Rect VirtualCoordinatesSystem::ConvertRect(const Rect & rect, float32 factor) const
{
    Rect newRect(rect);
    newRect.x *= factor;
    newRect.y *= factor;
    newRect.dx *= factor;
    newRect.dy *= factor;
    
    return newRect;
}
    
    
inline Vector2 VirtualCoordinatesSystem::ConvertInputToPhysical(const Vector2 &point) const
{
    Vector2 calcPoint(point);
    
    calcPoint -= inputOffset;
    calcPoint /= inputScaleFactor;
    
    return calcPoint;
}

inline Vector2 VirtualCoordinatesSystem::ConvertInputToVirtual(const Vector2 &point) const
{
    Vector2 calcPoint(point);
    
    calcPoint *= inputScaleFactor;
    calcPoint += inputOffset;
    
    return calcPoint;
}
    
inline const String & VirtualCoordinatesSystem::GetResourceFolder(int32 resourceIndex) const
{
    DVASSERT(resourceIndex < (int32)allowedSizes.size());
    return allowedSizes[resourceIndex].folderName;
}
    
inline int32 VirtualCoordinatesSystem::GetDesirableResourceIndex() const
{
    return desirableIndex;
}
    
inline int32 VirtualCoordinatesSystem::GetBaseResourceIndex() const
{
    return 0;
}
    
inline bool VirtualCoordinatesSystem::WasScreenSizeChanged() const
{
    return wasScreenResized;
}
    
const Vector2 & VirtualCoordinatesSystem::GetPhysicalDrawOffset() const
{
    return drawOffset;
}
    
inline Vector2 VirtualCoordinatesSystem::GetInputOffset() const
{
    return inputOffset;
}

inline float32 VirtualCoordinatesSystem::GetInputScaleFactor() const
{
    return inputScaleFactor;
}
    
};
#endif