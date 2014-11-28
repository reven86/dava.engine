/* File : AutotestingSystem.i */
%module AutotestingSystem

%import AutotestingSystemConfig.h

%import Base/Singleton.h
%import Base/BaseTypes.h

%{
#include "AutotestingSystemLua.h"
%}

%template(Singleton_Autotesting) DAVA::Singleton<DAVA::AutotestingSystemLua>;

/* Let's just grab the original header file here */
%include "std_string.i"
%import "UIControl.i"
%import "UI/UIEvent.h"

namespace DAVA
{
class UIList;

class UIListDelegate
{
    friend class UIList;

    virtual int32 ElementsCount(UIList * list) = 0;

    virtual UIListCell *CellAtIndex(UIList *list, int32 index) = 0;

    virtual float32 CellWidth(UIList * list, int32 index);   //! control calls this method only when it's in horizontal orientation

    virtual float32 CellHeight(UIList * list, int32 index);  //control calls this method only when it's in vertical orientation

    virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);

    virtual void SaveToYaml(UIList *forList, YamlNode *node);
};
class UIList
{
public:

    static const int32 maximumElementsCount = 100000;
    enum eListOrientation
    {
        ORIENTATION_VERTICAL = 0,
        ORIENTATION_HORIZONTAL,
    };

    UIList(const Rect &rect = Rect(), eListOrientation requiredOrientation = ORIENTATION_VERTICAL);

    void SetDelegate(UIListDelegate *newDelegate);
    UIListDelegate * GetDelegate();

    void ScrollToElement(int32 index);

    // Get and set aggregator path
    const FilePath & GetAggregatorPath();
    void SetAggregatorPath(const FilePath &aggregatorPath);

    float32 GetScrollPosition();
    void SetScrollPosition(float32 newScrollPos);
    void ResetScrollPosition();
    void Refresh();

    void SetSlowDownTime(float newValue);//sets how fast reduce speed (for example 0.25 reduces speed to zero for the 0.25 second ). To remove inertion effect set tihs value to 0
    void SetBorderMoveModifer(float newValue);//sets how scrolling element moves after reachig a border (0.5 as a default). To remove movement effect after borders set thus value to 0

    void SetTouchHoldDelta(int32 holdDelta);//the amount of pixels user must move the finger on the button to switch from button to scrolling (default 30)
    int32 GetTouchHoldDelta();

    void ScrollTo(float delta);

    void ScrollToPosition(float32 position, float32 timeSec = 0.3f);


    void SetOrientation(int32 orientation);
    inline int32 GetOrientation() const { return orientation; };

    const List<UIControl*> &GetVisibleCells();

    virtual List<UIControl* >& GetRealChildren();

    UIListCell* GetReusableCell(const String &cellIdentifier);//returns cell from the cells cache, if returns 0 you need to create the new one

    virtual void SystemWillAppear(); // Internal method used by ControlSystem

    virtual void LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader);
    virtual YamlNode * SaveToYamlNode(UIYamlLoader * loader);

    virtual float32 VisibleAreaSize(UIScrollBar *forScrollBar);
    virtual float32 TotalAreaSize(UIScrollBar *forScrollBar);
    virtual float32 ViewPosition(UIScrollBar *forScrollBar);
    virtual void OnViewPositionChanged(UIScrollBar *byScrollBar, float32 newPosition);

    virtual UIControl *Clone();
    virtual void CopyDataFrom(UIControl *srcControl);

    virtual const String GetDelegateControlPath(const UIControl *rootControl) const;

};
};

%include "KeyedArchive.i"
%include "AutotestingSystemLua.h"