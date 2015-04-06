#ifndef __QUICKED_CONTROL_SELECTION_LISTENER_H__
#define __QUICKED_CONTROL_SELECTION_LISTENER_H__

namespace DAVA
{
    class UIControl;
}

class ControlSelectionListener
{
public:
    ControlSelectionListener() {}
    virtual ~ControlSelectionListener() {}
    
    virtual void OnControlSelected(DAVA::UIControl *rootControl, DAVA::UIControl *selectedControl) = 0;
};

#endif // __QUICKED_CONTROL_SELECTION_LISTENER_H__
