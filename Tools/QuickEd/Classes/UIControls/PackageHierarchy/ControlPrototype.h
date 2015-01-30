#ifndef __QUICKED_CONTROL_PROTOTYPE_H__
#define __QUICKED_CONTROL_PROTOTYPE_H__

#include "Base/BaseObject.h"

class ControlNode;

class ControlPrototype : public DAVA::BaseObject
{
public:
    ControlPrototype(ControlNode *_controlNode, const DAVA::FilePath &packagePath = DAVA::FilePath());
    
protected:
    virtual ~ControlPrototype();
    
public:
    ControlNode *GetControlNode() const;
    DAVA::String GetName() const;
    
private:
    ControlNode *controlNode;
    DAVA::FilePath packagePath;
};

#endif //__QUICKED_CONTROL_PROTOTYPE_H__
