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


#ifndef __UI_EDITOR_CONTROL_NODE__
#define __UI_EDITOR_CONTROL_NODE__

#include "PackageBaseNode.h"
#include "ControlsContainerNode.h"

namespace DAVA
{
    class UIControlPackageContext;
}

class PackageNode;
class RootProperty;

class ControlNode : public ControlsContainerNode
{
public:
    enum eCreationType
    {
        CREATED_FROM_CLASS,
        CREATED_FROM_PROTOTYPE,
        CREATED_FROM_PROTOTYPE_CHILD
    };
    
private:
    ControlNode(DAVA::UIControl *control, bool recursively);
    ControlNode(ControlNode *node, eCreationType creationType);
    virtual ~ControlNode();

public:
    static ControlNode *CreateFromControl(DAVA::UIControl *control);
    static ControlNode *CreateFromControlWithChildren(DAVA::UIControl *control);
    static ControlNode *CreateFromPrototype(ControlNode *sourceNode);
    static ControlNode *CreateFromPrototypeChild(ControlNode *sourceNode);

public:
    ControlNode *Clone();
    
    void Add(ControlNode *node) override;
    void InsertAtIndex(int index, ControlNode *node) override;
    void Remove(ControlNode *node) override;
    int GetCount() const override;
    ControlNode *Get(int index) const override;
    void Accept(PackageVisitor *visitor) override;
    
    ControlNode *FindByName(const DAVA::String &name) const;
    
    virtual DAVA::String GetName() const override;
    DAVA::UIControl *GetControl() const;
    DAVA::UIControlPackageContext *GetPackageContext() const;
    void SetPackageContext(DAVA::UIControlPackageContext *context);
    
    ControlNode *GetPrototype() const;
    const DAVA::Vector<ControlNode*> &GetInstances() const;
    bool IsDependsOnPackage(PackageNode *package) const;

    virtual bool IsEditingSupported() const override;
    virtual bool IsInsertingControlsSupported() const override;
    virtual bool CanInsertControl(ControlNode *node, DAVA::int32 pos) const override;
    virtual bool CanRemove() const override;
    virtual bool CanCopy() const override;

    eCreationType GetCreationType() const { return creationType; }

    RootProperty *GetRootProperty() const {return rootProperty; }
    void RefreshProperties();

    void MarkAsRemoved();
    void MarkAsAlive();

    DAVA::String GetPathToPrototypeChild() const;

private:
    bool IsInstancedFrom(const ControlNode *prototype) const;
    void AddControlToInstances(ControlNode *control);
    void RemoveControlFromInstances(ControlNode *control);

private:
    DAVA::UIControl *control;
    RootProperty *rootProperty;
    DAVA::Vector<ControlNode*> nodes;
    
    ControlNode *prototype;
    DAVA::Vector<ControlNode*> instances; // weak

    eCreationType creationType;
};


#endif // __UI_EDITOR_CONTROL_NODE__
