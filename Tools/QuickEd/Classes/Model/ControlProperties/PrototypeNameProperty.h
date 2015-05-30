#ifndef __UI_EDITOR_PROTOTYPE_NAME_PROPERTY__
#define __UI_EDITOR_PROTOTYPE_NAME_PROPERTY__

#include "ValueProperty.h"

class ControlNode;

class PrototypeNameProperty : public ValueProperty
{

public:
    PrototypeNameProperty(ControlNode *aNode, const PrototypeNameProperty *sourceProperty, eCloneType cloneType);

protected:
    virtual ~PrototypeNameProperty();
    
public:
    void Accept(PropertyVisitor *visitor) override;

    ePropertyType GetType() const override;
    DAVA::VariantType GetValue() const override;
    bool IsReadOnly() const override;
    DAVA::String GetPrototypeName() const;
    
    ControlNode *GetControl() const;

protected:
    virtual void ApplyValue(const DAVA::VariantType &value);

private:
    ControlNode *node; // weak
};

#endif //__UI_EDITOR_PROTOTYPE_NAME_PROPERTY__
