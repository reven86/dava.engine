#ifndef __QUICKED_CONTROL_SELECTION_LISTENER_H__
#define __QUICKED_CONTROL_SELECTION_LISTENER_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
    class UIControl;
}

class ControlSelectionListener
{
public:
    ControlSelectionListener() {}
    virtual ~ControlSelectionListener() {}
    
    virtual void OnControlSelected(const DAVA::List<std::pair<DAVA::UIControl *, DAVA::UIControl*> > &selectedPairs) = 0;
};

#endif // __QUICKED_CONTROL_SELECTION_LISTENER_H__
