#ifndef __QUICKED_CONTROL_PROTOTYPE_H__
#define __QUICKED_CONTROL_PROTOTYPE_H__

#include "Base/BaseObject.h"

class PackageRef;
class ControlNode;

class ControlPrototype : public DAVA::BaseObject
{
public:
    ControlPrototype(ControlNode *_controlNode, PackageRef *_packageRef, bool isPackageImported);
    
protected:
    virtual ~ControlPrototype();
    
public:
    ControlNode *GetControlNode() const;
    DAVA::String GetName() const;
    PackageRef *GetPackageRef() const;
    
private:
    ControlNode *controlNode;
    PackageRef *packageRef;
    bool isPackageImported;
};

#endif //__QUICKED_CONTROL_PROTOTYPE_H__
