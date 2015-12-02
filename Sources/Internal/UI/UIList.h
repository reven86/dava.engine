/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_UI_LIST_H__
#define __DAVAENGINE_UI_LIST_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "UI/UIListCell.h"
#include "UI/UIScrollBar.h"

#include "UI/ScrollHelper.h"

namespace DAVA
{

/**
    \ingroup controlsystem
    \brief Class container for other controls and display them as list

    This class can show inner controls (UIListCell) as list.
    Inner controls are not real childs of UIList. Each List has scrollContainer which contain cells.
    All cells are build from specified by ID UIAggregatorControl.

    Aggregator ID of specific aggregator is set inside user's implementation of UIListDelegate::CellAtIndex.
    Only one aggregator for cells is allowed for a list.

    Aggregator Path is used for Save/Load procedure. Using this path, system can locate proper aggregator and get
    its ID.
*/

class UIList;
/**
    \ingroup controlsystem
    \brief UIListDelegate interface declares methods that are implemented by the delegate of UIList control.
    The methods provide data for UIList, and define it's content and allow to modify it's behaviour.
 */
class UIListDelegate
{
    friend class UIList;

    /**
        \brief This method is called by control when it need to know how many items is should display.
        Method should return number of items list in the list. It called initially when you add list and after UIList::Refresh.
        \param[in] list list object that requesting the information. You can have multiple lists with same delegate.
        \returns number of elements in the list.
     */
    virtual int32 ElementsCount(UIList * list) = 0;
    /**
        \brief This method should return UIListCell object for given index.
        \param[in] list list object that requesting the information. You can have multiple lists with same delegate.
        \param[in] index index of the list item
        \returns UIListCell that should be placed at index position in the list.
     */
    virtual UIListCell *CellAtIndex(UIList *list, int32 index) = 0;

    /**
        \brief This method is called by UIList when it need to know what is the width of the cell. It called only for horizontal lists.
        \param[in] list list object that requesting the information. You can have multiple lists with same delegate.
        \param[in] index index of the list item
        \returns width in pixels of the cell with given index. Default value is 20px.
     */
    virtual float32 CellWidth(UIList * list, int32 index);   //! control calls this method only when it's in horizontal orientation

    /**
        \brief This method is called by UIList when it need to know what is the height of the cell. It called only for vertical lists.
        \param[in] list list object that requesting the information. You can have multiple lists with same delegate.
        \param[in] index index of the list item
        \returns height in pixels of the cell with given index. Default value is 20px.
     */
    virtual float32 CellHeight(UIList * list, int32 index);  //control calls this method only when it's in vertical orientation

    /**
        \brief This method is called by UIList when cell was selected by user.
        \param[in] list list object that requesting the information. You can have multiple lists with same delegate.
        \param[in] index index of the list item
     */
    virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);

    /**
        \brief This metod is called by UIList when need to save.
    */
    virtual void SaveToYaml(UIList *forList, YamlNode *node);
};
/**
    \ingroup controlsystem
    \brief UIList is a control for displaying lists of information on the screen.
    It's simple and powerfull. Using this class you can create list.
    Lists can be vertical also, so you can create scrollable pages easily.

    Example of UIList usage:
    \code
    //on list creation you need to set your class as the delegate
    void MultiplayerScreen::LoadResources()
    {
        serversList = new UIList(Rect(10, 45, 460, 210), UIList::ORIENTATION_VERTICAL);
        serversList->SetDelegate(this);
        AddControl(serversList);
    }

    //next method should be realized in a delegate class

    //returns total cells count in the list
    int32 MultiplayerScreen::ElementsCount(UIList *forList)
    {
        return GameServer::Instance()->totalServers.size();
    }

    //returns cell dimension for the UIList calculations
    int32 MultiplayerScreen::CellHeight(UIList *forList, int32 index)//calls only for vertical orientation
    {
        return SERVER_CELL_HEIGHT;
    }

    //create cells and fill them with data
    UIListCell *MultiplayerScreen::CellAtIndex(UIList *forList, int32 index)
    {
        GameServerCell *c = (GameServerCell *)forList->GetReusableCell("Server cell"); //try to get cell from the reusable cells store
        if(!c)
        { //if cell of requested type isn't find in the store create new cell
            c = new GameServerCell(Rect((0, 0, 200, SERVER_CELL_HEIGHT), "Server cell");
        }
        //fill cell whith data
        c->serverName = GameServer::Instance()->totalServers[index].name + LocalizedString("'s game");
        c->SetStateText(UIControl::STATE_NORMAL, c->serverName, Vector2(c->GetStateBackground(UIControl::STATE_NORMAL)->GetSprite()->GetWidth() * 1.7 - 30, 0));
        c->connection = GameServer::Instance()->totalServers[index].connection;
        c->serverIndex = GameServer::Instance()->totalServers[index].index;

        return c;//returns cell
        //your application don't need to manage cells. UIList do all cells management.
        //you can create cells of your own types derived from the UIListCell
    }

    //when cell is pressed
    void MultiplayerScreen::OnCellSelected(UIList *forList, UIListCell *selectedCell)
    {
        PlayButtonSound();

        currentName = selectedCell->serverName;
        currentConnection = selectedCell->connection;
    }
    \endcode
 */
class UIList : public UIControl , public UIScrollBarDelegate
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

    virtual void SetRect(const Rect &rect);
    
    virtual void SetSize(const Vector2 &newSize);

    void SetOrientation(int32 orientation);
    inline int32 GetOrientation() const { return orientation; };

    const List<UIControl*> &GetVisibleCells();

    UIListCell* GetReusableCell(const String &cellIdentifier);//returns cell from the cells cache, if returns 0 you need to create the new one

    virtual void SystemWillAppear(); // Internal method used by ControlSystem

    virtual void LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader);
    virtual YamlNode * SaveToYamlNode(UIYamlLoader * loader);

    virtual float32 VisibleAreaSize(UIScrollBar *forScrollBar);
    virtual float32 TotalAreaSize(UIScrollBar *forScrollBar);
    virtual float32 ViewPosition(UIScrollBar *forScrollBar);
    virtual void OnViewPositionChanged(UIScrollBar *byScrollBar, float32 newPosition);

    UIList* Clone() override;
    virtual void CopyDataFrom(UIControl *srcControl);

    virtual const String GetDelegateControlPath(const UIControl *rootControl) const;

protected:
    void InitAfterYaml();
    virtual ~UIList();

    void FullRefresh();

    virtual void Update(float32 timeElapsed);

    virtual void Input(UIEvent *currentInput);
    virtual bool SystemInput(UIEvent *currentInput);// Internal method used by ControlSystem

    Vector<UIListCell*> *GetStoreVector(const String &cellIdentifier);
    void AddCellAtPos(UIListCell *cell, float32 pos, float32 size, int32 index);

    void OnSelectEvent(BaseObject *pCaller, void *pUserData, void *callerData);


    UIListDelegate *delegate;
    eListOrientation orientation;

    UIControl *scrollContainer;

    int32 mainTouch;

    ScrollHelper *scroll;
    float32 addPos;
    float32 oldPos;
    float32 newPos;
    float32 oldScroll = 0.f;
    float32 newScroll = 0.f;

    int32 touchHoldSize;

    // Private boolean variables are grouped together because of DF-2149.
    bool lockTouch : 1;
    bool needRefresh : 1;

    FilePath aggregatorPath;

    Map<String,Vector<UIListCell*>*> cellStore;
public:
    INTROSPECTION_EXTEND(UIList, UIControl,
        PROPERTY("orientation",  InspDesc("List orientation", GlobalEnumMap<UIList::eListOrientation>::Instance()), GetOrientation, SetOrientation, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("aggregatorPath", "Aggregator Path", GetAggregatorPath, SetAggregatorPath, I_SAVE | I_VIEW | I_EDIT)
        );
};
};
#endif
