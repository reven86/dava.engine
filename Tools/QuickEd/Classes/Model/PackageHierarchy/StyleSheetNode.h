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


#ifndef __UI_EDITOR_STYLE_SHEET_NODE_H__
#define __UI_EDITOR_STYLE_SHEET_NODE_H__

#include "PackageBaseNode.h"
#include "UI/Styles/UIStyleSheetSelectorChain.h"

namespace DAVA
{
    class UIStyleSheet;
}

class StyleSheetRootProperty;

class StyleSheetNode : public PackageBaseNode
{
public:
    StyleSheetNode(const DAVA::Vector<DAVA::UIStyleSheetSelectorChain> &selectorChains, const DAVA::Vector<DAVA::UIStyleSheetProperty> &properties);

protected:
    virtual ~StyleSheetNode();

public:
    StyleSheetNode *Clone() const;

    int GetCount() const override;
    PackageBaseNode *Get(DAVA::int32 index) const override;
    void Accept(PackageVisitor *visitor) override;
    
    DAVA::String GetName() const override;
    void UpdateName();

    bool CanRemove() const override;
    bool CanCopy() const override;

    StyleSheetRootProperty *GetRootProperty() const;
    
private:
    StyleSheetRootProperty *rootProperty;
    DAVA::String name;
};

#endif //__UI_EDITOR_STYLE_SHEET_NODE_H__
