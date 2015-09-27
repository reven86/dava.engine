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

#ifndef __DAVAENGINE_CONTROL_LAYOUT_DATA_H__
#define __DAVAENGINE_CONTROL_LAYOUT_DATA_H__

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

namespace DAVA
{
class UIControl;

class ControlLayoutData
{
public:
    enum eFlag
    {
        FLAG_NONE = 0,
        FLAG_SIZE_CHANGED = 1 << 0,
        FLAG_POSITION_CHANGED = 1 << 1,
        FLAG_SIZE_CALCULATED = 1 << 2,
        FLAG_LAST_IN_LINE = 1 << 3,
    };
    
public:
    ControlLayoutData(UIControl *control_);
    
    void ApplyLayoutToControl(int32 indexOfSizeProperty);
    
    UIControl *GetControl() const;
    
    bool HasFlag(eFlag flag) const;
    void SetFlag(eFlag flag);
    
    int32 GetFirstChildIndex() const;
    void SetFirstChildIndex(int32 index);

    int32 GetLastChildIndex() const;
    void SetLastChildIndex(int32 index);
    
    bool HasChildren() const;
    
    float32 GetSize(Vector2::eAxis axis) const;
    void SetSize(Vector2::eAxis axis, float32 value);
    void SetSizeWithoutChangeFlag(Vector2::eAxis axis, float32 value);
    
    float32 GetPosition(Vector2::eAxis axis) const;
    void SetPosition(Vector2::eAxis axis, float32 value);
    
    float32 GetX() const;
    float32 GetY() const;
    float32 GetWidth() const;
    float32 GetHeight() const;
    
    bool HaveToSkipControl(bool skipInvisible) const;

private:
    UIControl *control;
    int32 flags = FLAG_NONE;
    int32 firstChild = 0;
    int32 lastChild = -1;
    Vector2 size;
    Vector2 position;
};
    
}


#endif //__DAVAENGINE_CONTROL_LAYOUT_DATA_H__
