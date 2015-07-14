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


#include "UIPackage.h"

#include "UI/UIControl.h"
#include "UI/UIControlPackageContext.h"

namespace DAVA
{

UIPackage::UIPackage() : 
    controlPackageContext(new UIControlPackageContext())
{
}

UIPackage::~UIPackage()
{
    for (UIControl *control : controls)
        control->Release();
    
    controls.clear();

    SafeRelease(controlPackageContext);
}

DAVA::int32 UIPackage::GetControlsCount() const
{
    return (int32) controls.size();
}
    
UIControl * UIPackage::GetControl(const String &name) const
{
    for (UIControl *control : controls)
    {
        if (control->GetName() == name)
            return control;
    }

    return nullptr;
}

UIControl * UIPackage::GetControl(int32 index) const
{
    DVASSERT(0 <= index && index < static_cast<int32>(controls.size()));
    return controls[index];
}

void UIPackage::AddControl(UIControl *control)
{
    control->SetPackageContext(controlPackageContext);
    controls.push_back(SafeRetain(control));
}
    
void UIPackage::RemoveControl(UIControl *control)
{
    Vector<UIControl *>::iterator iter = std::find(controls.begin(), controls.end(), control);
    if (iter != controls.end())
    {
        SafeRelease(*iter);
        controls.erase(iter);
    }
}

UIControlPackageContext* UIPackage::GetControlPackageContext()
{
    return controlPackageContext;
}

RefPtr<UIPackage> UIPackage::Clone() const
{
    RefPtr<UIPackage> package(new UIPackage());

    package->controls.resize(controls.size());

    std::transform(controls.begin(), controls.end(), package->controls.begin(),
    [](UIControl *control)->UIControl *
    {
        return control->Clone();
    });
    return package;
}

}
