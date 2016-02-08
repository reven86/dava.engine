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


#ifndef __DAVAENGINE_UI_CONTROL_HELPERS_H__
#define __DAVAENGINE_UI_CONTROL_HELPERS_H__

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Math/Rect.h"

namespace DAVA
{
class UIControl;
class UIList;
class UIScrollView;

class UIControlHelpers
{
public:
    static String GetControlPath(const UIControl* control, const UIControl* rootControl = NULL);
    static String GetPathToOtherControl(const UIControl* src, const UIControl* dst);
    static UIControl* FindChildControlByName(const String& controlName, const UIControl* rootControl, bool recursive);
    static UIControl* FindChildControlByName(const FastName& controlName, const UIControl* rootControl, bool recursive);
    static UIControl* FindControlByPath(const String& controlPath, UIControl* rootControl);
    static const UIControl* FindControlByPath(const String& controlPath, const UIControl* rootControl);

    static void ScrollToControl(DAVA::UIControl* control, bool toTopLeftForBigControls = false);
    static void ScrollToControlWithAnimation(DAVA::UIControl* control, float32 animationTime = 0.3f, bool toTopLeftForBigControls = false);

private:
    template <typename ControlType>
    static ControlType* FindControlByPathImpl(const String& controlPath, ControlType* rootControl);

    template <typename ControlType>
    static ControlType* FindControlByPathImpl(Vector<FastName>::const_iterator begin, Vector<FastName>::const_iterator end, ControlType* rootControl);

    template <typename ControlType>
    static ControlType* FindControlByPathRecursivelyImpl(Vector<FastName>::const_iterator begin, Vector<FastName>::const_iterator end, ControlType* rootControl);

    static void ScrollToRect(DAVA::UIControl* control, const Rect& rect, float32 animationTime, bool toTopLeftForBigControls);
    static float32 GetScrollPositionToShowControl(float32 controlPos, float32 controlSize, float32 scrollSize, bool toTopLeftForBigControls);
    static Rect ScrollListToRect(UIList* list, const DAVA::Rect& rect, float32 animationTime, bool toTopLeftForBigControls);
    static Rect ScrollUIScrollViewToRect(UIScrollView* scrollView, const DAVA::Rect& rect, float32 animationTime, bool toTopLeftForBigControls);

    static const FastName WILDCARD_PARENT;
    static const FastName WILDCARD_CURRENT;
    static const FastName WILDCARD_ROOT;
    static const FastName WILDCARD_MATCHES_ONE_LEVEL;
    static const FastName WILDCARD_MATCHES_ZERO_OR_MORE_LEVEL;
};
};
#endif // __DAVAENGINE_UI_CONTROL_HELPERS_H__