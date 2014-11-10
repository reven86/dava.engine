#ifndef __UI_EDITOR_CONTROL_NODE__
#define __UI_EDITOR_CONTROL_NODE__

#include "PackageBaseNode.h"

#include "UIControls/ControlProperties/PropertiesRoot.h"

class ControlNode : public PackageBaseNode
{
public:
    enum eCreationType
    {
        CREATED_FROM_CLASS,
        CREATED_FROM_PROTOTYPE,
        CREATED_FROM_PROTOTYPE_CHILD
    };
    
public:
    ControlNode(DAVA::UIControl *control);
    ControlNode(ControlNode *node, DAVA::UIPackage *prototypePackage, eCreationType creationType = CREATED_FROM_PROTOTYPE);
    virtual ~ControlNode();

    void Add(ControlNode *node);
    void InsertBelow(ControlNode *node, const ControlNode *belowThis);
    void Remove(ControlNode *node);
    virtual int GetCount() const override;
    virtual PackageBaseNode *Get(int index) const override;
    ControlNode *FindByName(const DAVA::String &name) const;
    
    virtual DAVA::String GetName() const;
    DAVA::UIControl *GetControl() const;
    DAVA::String GetPrototypeName() const;

    virtual int GetFlags() const override;
    void SetReadOnly();
    
    eCreationType GetCreationType() const { return creationType; }

    PropertiesRoot *GetPropertiesRoot() const {return propertiesRoot; }
    DAVA::YamlNode *Serialize(DAVA::YamlNode *prototypeChildren) const;

private:
    DAVA::UIControl *control;
    PropertiesRoot *propertiesRoot;
    DAVA::Vector<ControlNode*>nodes;
    
    ControlNode *prototype;
    DAVA::UIPackage *prototypePackage;

    eCreationType creationType;
    
    bool readOnly;
    
};


#endif // __UI_EDITOR_CONTROL_NODE__
