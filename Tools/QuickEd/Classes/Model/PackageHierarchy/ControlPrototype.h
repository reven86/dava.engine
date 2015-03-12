#ifndef __QUICKED_CONTROL_PROTOTYPE_H__
#define __QUICKED_CONTROL_PROTOTYPE_H__

#include "Base/BaseObject.h"

class PackageRef;
class ControlNode;

class ControlPrototype : public DAVA::BaseObject
{
public:
    ControlPrototype(ControlNode *_controlNode, PackageRef *_packageRef);
    
protected:
    virtual ~ControlPrototype();
    
public:
    ControlNode *GetControlNode() const;
    DAVA::String GetName(bool withPackage) const;
    PackageRef *GetPackageRef() const;
    
private:
    ControlNode *controlNode;
    PackageRef *packageRef;
};

#endif //__QUICKED_CONTROL_PROTOTYPE_H__
