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


#ifndef __UI_EDITOR_UI_PACKAGE_MODEL_NODE__
#define __UI_EDITOR_UI_PACKAGE_MODEL_NODE__

#include "Base/BaseObject.h"

namespace DAVA
{
    class UIControl;
}

class ControlNode;
class StyleSheetNode;
class PackageNode;
class PackageVisitor;

class PackageBaseNode : public DAVA::BaseObject
{
public:
    PackageBaseNode(PackageBaseNode *parent);
protected:
    virtual ~PackageBaseNode();

public:
    virtual int GetCount() const = 0;
    virtual PackageBaseNode *Get(int index) const = 0;
    int GetIndex(const PackageBaseNode *node) const;
    
    PackageBaseNode *GetParent() const;
    void SetParent(PackageBaseNode *parent);
    
    virtual void Accept(PackageVisitor *visitor) = 0;
    
    virtual DAVA::String GetName() const;
    virtual PackageNode *GetPackage();
    virtual const PackageNode *GetPackage() const;
    
    virtual DAVA::UIControl *GetControl() const;
    
    virtual void debugDump(int depth);
    
    virtual bool IsEditingSupported() const;
    virtual bool IsInsertingControlsSupported() const;
    virtual bool IsInsertingPackagesSupported() const;
    virtual bool IsInsertingStylesSupported() const;
    virtual bool CanInsertControl(ControlNode *node, DAVA::int32 pos) const;
    virtual bool CanInsertStyle(StyleSheetNode *node, DAVA::int32 pos) const;
    virtual bool CanInsertImportedPackage(PackageNode *package) const;
    virtual bool CanRemove() const;
    virtual bool CanCopy() const;
    virtual bool IsReadOnly() const;
    
private:
    PackageBaseNode *parent;
};

#endif // __UI_EDITOR_UI_PACKAGE_MODEL_NODE__
