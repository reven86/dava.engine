#ifndef __DAVEENGINE_UI_SYSMTE_H__
#define __DAVEENGINE_UI_SYSMTE_H__ 

#include "Base/BaseTypes.h"

namespace DAVA
{

class UISystem
{
public:

    enum eSystemType
    {
        UI_RENDER_SYSTEM = 1,
		UI_UPDATE_SYSTEM,

        UI_CONTROL_SYSTEMS_COUNT
    };

    UISystem() {}
    virtual ~UISystem() {}

    virtual uint32 GetRequiredComponents() const = 0;
    virtual uint32 GetType() const = 0;

    virtual void Process() = 0;

};

}


#endif //__DAVEENGINE_UI_SYSMTE_H__ 