#ifndef __UIEditor_EditScreen_h__
#define __UIEditor_EditScreen_h__

#include "DAVAEngine.h"

class ControlSelectionListener;

class CheckeredCanvas: public DAVA::UIControl
{
public:
    CheckeredCanvas();
private:
    virtual ~CheckeredCanvas();
    
public:
    virtual void Draw(const DAVA::UIGeometricData &geometricData) override;
    virtual void DrawAfterChilds(const DAVA::UIGeometricData &geometricData) override;
    virtual bool SystemInput(DAVA::UIEvent *currentInput) override;

public:
    void SelectControl(UIControl *control);
    void RemoveSelection(UIControl *control);
    void ClearSelections();
    UIControl *GetControlByPos(UIControl *control, const DAVA::Vector2 &pos);
    
    void AddControlSelectionListener(ControlSelectionListener *listener);
    void RemoveControlSelectionListener(ControlSelectionListener *listener);

private:
    DAVA::Set<UIControl*> selectionControls;
    DAVA::List<ControlSelectionListener*> selectionListeners;
    
};

class PackageCanvas: public DAVA::UIControl
{
public:
    PackageCanvas();
private:
    virtual ~PackageCanvas();
public:
    void LayoutCanvas();
private:
};

#endif // __UIEditor_EditScreen_h__
