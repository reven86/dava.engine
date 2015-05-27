#ifndef __UI_EDITOR_UI_PACKAGE_MODEL_NODE__
#define __UI_EDITOR_UI_PACKAGE_MODEL_NODE__

#include "Base/BaseObject.h"

namespace DAVA
{
    class UIControl;
}

class ControlNode;
class PackageNode;

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
    
    virtual DAVA::String GetName() const;
    virtual PackageNode *GetPackage();
    virtual const PackageNode *GetPackage() const;
    
    virtual DAVA::UIControl *GetControl() const;
    
    virtual void debugDump(int depth);
    
    virtual bool IsEditingSupported() const;
    virtual bool IsInsertingSupported() const;
    virtual bool CanInsertControl(ControlNode *node, DAVA::int32 pos) const;
    virtual bool CanInsertImportedPackage() const;
    virtual bool CanRemove() const;
    virtual bool CanCopy() const;
    virtual bool IsReadOnly() const;
    
private:
    PackageBaseNode *parent;
};

#endif // __UI_EDITOR_UI_PACKAGE_MODEL_NODE__
