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
    PackageBaseNode(PackageBaseNode* parent);

protected:
    virtual ~PackageBaseNode();

public:
    virtual int GetCount() const = 0;
    virtual PackageBaseNode* Get(int index) const = 0;
    int GetIndex(const PackageBaseNode* node) const;

    PackageBaseNode* GetParent() const;
    void SetParent(PackageBaseNode* parent);

    virtual void Accept(PackageVisitor* visitor) = 0;

    virtual DAVA::String GetName() const;
    virtual PackageNode* GetPackage();
    virtual const PackageNode* GetPackage() const;

    virtual DAVA::UIControl* GetControl() const;

    virtual void debugDump(int depth);

    virtual bool IsEditingSupported() const;
    virtual bool IsInsertingControlsSupported() const;
    virtual bool IsInsertingPackagesSupported() const;
    virtual bool IsInsertingStylesSupported() const;
    virtual bool CanInsertControl(const ControlNode* node, DAVA::int32 pos) const;
    virtual bool CanInsertStyle(StyleSheetNode* node, DAVA::int32 pos) const;
    virtual bool CanInsertImportedPackage(PackageNode* package) const;
    virtual bool CanRemove() const;
    virtual bool CanCopy() const;
    virtual bool IsReadOnly() const;

private:
    PackageBaseNode* parent;
};

bool CompareByLCA(PackageBaseNode* left, PackageBaseNode* right);

#endif // __UI_EDITOR_UI_PACKAGE_MODEL_NODE__
