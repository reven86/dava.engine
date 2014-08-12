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
public:
    VirtualCoordinatesTransformSystem() {};
    virtual ~VirtualCoordinatesTransformSystem() {};

    void SetVirtualScreenSize(const Vector2 & vector);
    void SetPhysicalScreenSize(const Vector2 & vector);
    const Vector2 & GetVirtualScreenSize();
    const Vector2 & GetPhysicalScreenSize();

    Vector2 ConvertPhysicalToVirtual(const Vector2 & vector);
    Vector2 ConvertVirtualToPhysical(const Vector2 & vector);
    Vector2 ConvertResourceToVirtual(const Vector2 & vector, DAVA::int32 resourceIndex);
    Rect ConvertPhysicalToVirtual(const Rect & rect);
    Rect ConvertVirtualToPhysical(const Rect & rect);
    Rect ConvertResourceToVirtual(const Rect & rect, int32 resourceIndex);
};

};
#endif