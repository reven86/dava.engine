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
    virtual void Serialize(PackageSerializer *serializer) const override;

    virtual ePropertyType GetType() const override;
    virtual DAVA::VariantType GetValue() const override;
    virtual bool IsReadOnly() const override;

protected:
    virtual void ApplyValue(const DAVA::VariantType &value);

private:
    ControlNode *node; // weak
};

#endif //__UI_EDITOR_PROTOTYPE_NAME_PROPERTY__
