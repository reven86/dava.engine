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
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlEmitter.h"
#include "FileSystem/YamlParser.h"
#include "UI/UIControl.h"

namespace DAVA
{

UIPackage::UIPackage(const FilePath &path)
    : packagePath(path)
{
}

UIPackage::~UIPackage()
{
    for (auto iter = controls.begin(); iter != controls.end(); ++iter)
        SafeRelease(*iter);
    controls.clear();
    
    for (auto it = importedPackages.begin(); it != importedPackages.end(); ++it)
        SafeRelease(*it);
    importedPackages.clear();
}

String UIPackage::GetName() const
{
    return packagePath.GetBasename();
}

DAVA::int32 UIPackage::GetControlsCount() const
{
    return (int32) controls.size();
}
    
UIControl * UIPackage::GetControl(const String &name) const
{
    Vector<UIControl *>::const_iterator iter = controls.begin();
    Vector<UIControl *>::const_iterator end = controls.end();
    for (; iter != end; ++iter)
    {
        if ((*iter)->GetName() == name)
            return *iter;
    }

    return NULL;
}

UIControl * UIPackage::GetControl(int32 index) const
{
    DVASSERT(0 <= index && index < static_cast<int32>(controls.size()));
    return controls[index];
}

void UIPackage::AddControl(UIControl *control)
{
    controls.push_back(SafeRetain(control));
}
    
void UIPackage::InsertControlBelow(UIControl *control, const UIControl *belowThis)
{
    auto it = find(controls.begin(), controls.end(), belowThis);
    if (it != controls.end())
    {
        ++it;
        controls.insert(it, SafeRetain(control));
    }
    else
    {
        DVASSERT(false);
    }
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

int32 UIPackage::GetPackagesCount() const
{
    return (int32) importedPackages.size();
}

UIPackage *UIPackage::GetPackage(int32 index) const
{
    return importedPackages[index];
}

UIPackage *UIPackage::GetPackage(const String name) const
{
    for (auto it = importedPackages.begin(); it != importedPackages.end(); ++it)
    {
        if ((*it)->GetName() == name)
            return *it;
    }
    return NULL;
}

void UIPackage::AddPackage(UIPackage *package)
{
    if (std::find(importedPackages.begin(), importedPackages.end(), package) == importedPackages.end())
        importedPackages.push_back(SafeRetain(package));
}

}
