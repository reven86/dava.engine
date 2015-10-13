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


#include "UI/UIList.h"
#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"
#include "UI/UIControlSystem.h"
#include "Base/ObjectFactory.h"
#include "FileSystem/YamlNode.h"
#include "UI/UIYamlLoader.h"
#include "UI/UIControlHelpers.h"

namespace DAVA
{

float32 UIListDelegate::CellWidth(UIList* /*list*/, int32 /*index*/)
{
    return 20.0f;
};

float32 UIListDelegate::CellHeight(UIList* /*list*/, int32 /*index*/)
{
    return 20.0f;
};

void UIListDelegate::OnCellSelected(UIList* /*forList*/, UIListCell* /*selectedCell*/)
{
};

void UIListDelegate::SaveToYaml(UIList* /*forList*/, YamlNode* /*node*/)
{
};

UIList::UIList(const Rect &rect/* = Rect()*/, eListOrientation requiredOrientation/* = ORIENTATION_VERTICAL*/)
    : UIControl(rect)
    , delegate(NULL)
    , orientation(requiredOrientation)
    , scrollContainer(NULL)
    , scroll(NULL)
    , aggregatorPath(FilePath())
{
    InitAfterYaml();
}

void UIList::InitAfterYaml()
{
    SetInputEnabled(true, false);
    SetFocusEnabled(false);
    clipContents = true;
    Rect r = GetRect();
    r.x = 0;
    r.y = 0;

    // InitAfterYaml might be called multiple times - check this and remove previous scroll, if yes.
    if (scrollContainer)
    {
        RemoveControl(scrollContainer);
        SafeRelease(scrollContainer);
    }
    
    scrollContainer = new UIControl(r);
    AddControl(scrollContainer);
    scrollContainer->SetFocusEnabled(false);

    oldPos = 0;
    newPos = 0;

    mainTouch = -1;

    touchHoldSize = 15;

    lockTouch = false;

    needRefresh = false;

    if (scroll == NULL)
    {
        scroll = new ScrollHelper();
    }
}

UIList::~UIList()
{
    SafeRelease(scrollContainer);
    SafeRelease(scroll);

    for (Map<String,Vector<UIListCell*>*>::iterator mit = cellStore.begin() ; mit != cellStore.end(); mit++)
    {
        for (Vector<UIListCell*>::iterator it = mit->second->begin(); it != mit->second->end(); it++)
        {
            SafeRelease(*it);
        }
        delete mit->second;
        mit->second = NULL;
    }
}

void UIList::ScrollTo(float delta)
{
    scroll->Impulse(delta * -4.8f);
}

void UIList::SetRect(const Rect &rect)
{
    if(orientation == ORIENTATION_HORIZONTAL)
    {
        scroll->SetViewSize(rect.dx);
    }
    else
    {
        scroll->SetViewSize(rect.dy);
    }

    UIControl::SetRect(rect);
    scrollContainer->SetRect(rect);
}
    
void UIList::SetSize(const Vector2 &newSize)
{
    UIControl::SetSize(newSize);
    scrollContainer->SetSize(newSize);
}

void UIList::SetDelegate(UIListDelegate *newDelegate)
{
    delegate = newDelegate;
    Refresh();
}

UIListDelegate * UIList::GetDelegate()
{
    return delegate;
}

void UIList::ScrollToElement(int32 index)
{
    DVASSERT(delegate)
    DVASSERT(0<=index && index<delegate->ElementsCount(this))
    float32 newScrollPos = 0.0f;
    if(orientation == ORIENTATION_HORIZONTAL)
    {
        for(int32 i=0; i<index; ++i)
        {
            newScrollPos += delegate->CellWidth(this,i);
        }
    }
    else
    {
        for(int32 i=0; i<index; ++i)
        {
            newScrollPos += delegate->CellHeight(this,i);
        }
    }
    SetScrollPosition(newScrollPos);
}

void UIList::SetOrientation(int32 _orientation)
{
    orientation = (UIList::eListOrientation)_orientation;
}

float32 UIList::GetScrollPosition()
{
    return scroll->GetPosition();
}

void UIList::SetScrollPosition(float32 newScrollPos)
{
    if(needRefresh)
    {
        FullRefresh();
    }

    if(orientation == ORIENTATION_HORIZONTAL)
    {
        scroll->SetPosition(-newScrollPos);
    }
    else
    {
        scroll->SetPosition(-newScrollPos);
    }
}

void UIList::ResetScrollPosition()
{
    if(orientation == ORIENTATION_HORIZONTAL)
    {
        scrollContainer->relativePosition.x = 0;
        scroll->SetPosition(0);
    }
    else
    {
        scrollContainer->relativePosition.y = 0;
        scroll->SetPosition(0);
    }
}

void UIList::FullRefresh()
{
    scrollContainer->RemoveAllControls();
    if(!delegate)
    {
        return;
    }

    needRefresh = false;

    addPos = 0.0f;
    float32 scrollAdd;
    float32 maxSize;
    if(orientation == ORIENTATION_HORIZONTAL)
    {
        scrollAdd = scrollContainer->GetRect().x;
        maxSize = GetRect().dx;
    }
    else
    {
        scrollAdd = scrollContainer->GetRect().y;
        maxSize = GetRect().dy;
    }

    scroll->SetViewSize(maxSize);

    float32 sz = 0.0f;
    int32 elCnt = delegate->ElementsCount(this);
    int32 index = 0;
    for (; index < elCnt; index++)
    {
        float32 curPos = addPos + scrollAdd;
        float32 size = 0.0f;
        if(orientation == ORIENTATION_HORIZONTAL)
        {
            size = delegate->CellWidth(this, index);
        }
        else
        {
            size = delegate->CellHeight(this, index);
        }

        sz += size;
        if(curPos + size > -maxSize)
        {
            AddCellAtPos(delegate->CellAtIndex(this, index), addPos, size, index);
        }

        addPos += size;
        if(addPos + scrollAdd > maxSize * 2.0f)
        {
            break;
        }
    }

    index++;
    for (; index < elCnt; index++)
    {
        if(orientation == ORIENTATION_HORIZONTAL)
        {
            sz += delegate->CellWidth(this, index);
        }
        else
        {
            sz += delegate->CellHeight(this, index);
        }
    }

    scroll->SetElementSize(sz);
}

void UIList::Refresh()
{
    needRefresh = true;
}

void UIList::Update(float32 timeElapsed)
{
    if(!delegate)
    {
        return;
    }

    if(needRefresh)
    {
        FullRefresh();
    }

    float d = newPos - oldPos;
    oldPos = newPos;
    Rect r = scrollContainer->GetRect();
    if(orientation == ORIENTATION_HORIZONTAL)
    {
        r.x = scroll->GetPosition(d, SystemTimer::FrameDelta(), lockTouch);
    }
    else
    {
        r.y = scroll->GetPosition(d, SystemTimer::FrameDelta(), lockTouch);
    }

    if (r != scrollContainer->GetRect())
    {
        scrollContainer->SetRect(r);
    }

    List<UIControl*>::const_iterator it;
    Rect viewRect = GetGeometricData().GetUnrotatedRect();//GetRect(TRUE);
    const List<UIControl*> &scrollList = scrollContainer->GetChildren();
    List<UIControl*> removeList;

    //removing invisible elements
    for(it = scrollList.begin(); it != scrollList.end(); it++)
    {
        Rect crect = (*it)->GetGeometricData().GetUnrotatedRect();//GetRect(TRUE);
        if(orientation == ORIENTATION_HORIZONTAL)
        {
            if(crect.x + crect.dx < viewRect.x - viewRect.dx || crect.x > viewRect.x + viewRect.dx*2)
            {
                removeList.push_back(*it);
            }
        }
        else
        {
            if(crect.y + crect.dy < viewRect.y - viewRect.dy || crect.y > viewRect.y + viewRect.dy*2)
            {
                removeList.push_back(*it);
            }
        }
    }
    for(it = removeList.begin(); it != removeList.end(); it++)
    {
        scrollContainer->RemoveControl((*it));
    }

    if (!scrollList.empty())
    {
            //adding elements at the list end
        int32 ind = -1;
        UIListCell *fc = NULL;
        for(it = scrollList.begin(); it != scrollList.end(); it++)
        {
            UIListCell *lc = (UIListCell *)(*it);
            int32 i = lc->GetIndex();
            if(i > ind)
            {
                ind = i;
                fc = lc;
            }
        }
        if(fc)
        {
            float32 borderPos;
            float32 rPos;
            float32 size = 0.0f;
            float32 off;
            if(orientation == ORIENTATION_HORIZONTAL)
            {
                borderPos = viewRect.dx + viewRect.dx / 2.0f;
                off = scrollContainer->GetRect().x;
                rPos = fc->GetRect().x + fc->GetRect().dx + off;
            }
            else
            {
                borderPos = viewRect.dy + viewRect.dy / 2.0f;
                off = scrollContainer->GetRect().y;
                rPos = fc->GetRect().y + fc->GetRect().dy + off;
            }

            int32 elementsCount = delegate->ElementsCount(this);
            while(rPos < borderPos && fc->GetIndex() < elementsCount - 1)
            {
                int32 i = fc->GetIndex() + 1;
                fc = delegate->CellAtIndex(this, i);
                if(orientation == ORIENTATION_HORIZONTAL)
                {
                    size = delegate->CellWidth(this, i);
                }
                else
                {
                    size = delegate->CellHeight(this, i);
                }
                AddCellAtPos(fc, rPos - off, size, i);
                rPos += size;
                    //			scroll->SetElementSize((float32)(rPos - off));
            }
        }

            //adding elements at the list begin
        ind = maximumElementsCount;
        fc = NULL;
        for(it = scrollList.begin(); it != scrollList.end(); it++)
        {
            UIListCell *lc = (UIListCell *)(*it);
            int32 i = lc->GetIndex();
            if(i < ind)
            {
                ind = i;
                fc = lc;
            }
        }
        if(fc)
        {
            float32 borderPos;
            float32 rPos;
            float32 size = 0.0f;
            float32 off;
            if(orientation == ORIENTATION_HORIZONTAL)
            {
                borderPos = -viewRect.dx/2.0f;
                off = scrollContainer->GetRect().x;
                rPos = fc->GetRect().x + off;
            }
            else
            {
                borderPos = -viewRect.dy/2.0f;
                off = scrollContainer->GetRect().y;
                rPos = fc->GetRect().y + off;
            }
            while(rPos > borderPos && fc->GetIndex() > 0)
            {
                int32 i = fc->GetIndex() - 1;
                fc = delegate->CellAtIndex(this, i);
                if(orientation == ORIENTATION_HORIZONTAL)
                {
                    size = delegate->CellWidth(this, i);
                }
                else
                {
                    size = delegate->CellHeight(this, i);
                }
                rPos -= size;
                AddCellAtPos(fc, rPos - off, size, i);
            }
        }
    }
    else
    {
        FullRefresh();
    }



}

void UIList::Input(UIEvent *currentInput)
{
    if (lockTouch && currentInput->tid != mainTouch)
    {
        // Ignore any other touches when the input is locked.
        currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_HARD);
        return;
    }

    if(orientation == ORIENTATION_HORIZONTAL)
    {
        newPos = currentInput->point.x;
    }
    else
    {
        newPos = currentInput->point.y;
    }

    switch (currentInput->phase)
    {
    case UIEvent::Phase::BEGAN:
        {
            lockTouch = true;
            oldPos = newPos;
            mainTouch = currentInput->tid;
        }
            break;
            case UIEvent::Phase::DRAG:
        {
        }
            break;
            case UIEvent::Phase::ENDED:
        {
            lockTouch = false;
            mainTouch = -1;
        }
            break;
    }

    currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_HARD); // Drag is handled - see please DF-2508.
}

bool UIList::SystemInput(UIEvent *currentInput)
{
    if(!GetInputEnabled() || !visible || controlState & STATE_DISABLED)
    {
        return false;
    }

    if(currentInput->touchLocker != this)
    {
        if (currentInput->phase == UIEvent::Phase::BEGAN)
        {
            if(IsPointInside(currentInput->point))
            {
                PerformEvent(EVENT_TOUCH_DOWN);
                Input(currentInput);
            }
        }
        else if (currentInput->tid == mainTouch && currentInput->phase == UIEvent::Phase::DRAG)
        {
            if(orientation == ORIENTATION_HORIZONTAL)
            {
                if(Abs(currentInput->point.x - newPos) > touchHoldSize)
                {
                    UIControlSystem::Instance()->SwitchInputToControl(mainTouch, this);
                    newPos = currentInput->point.x;
                    return true;
                }
            }
            else
            {
                if(Abs(currentInput->point.y - newPos) > touchHoldSize)
                {
                    UIControlSystem::Instance()->SwitchInputToControl(mainTouch, this);
                    newPos = currentInput->point.y;
                    return true;
                }
            }
        }
        else if (currentInput->tid == mainTouch && currentInput->phase == UIEvent::Phase::ENDED)
        {
            mainTouch = -1;
            lockTouch = false;
            SetFocusEnabled(true);
            scrollContainer->SetFocusEnabled(true);
            bool retVal = UIControl::SystemInput(currentInput);
            SetFocusEnabled(false);
            scrollContainer->SetFocusEnabled(false);
            return retVal;
        }



    }

    return UIControl::SystemInput(currentInput);
}

void UIList::OnSelectEvent(BaseObject *pCaller, void *pUserData, void *callerData)
{
    if(delegate)
    {
        delegate->OnCellSelected(this, (UIListCell*)pCaller);
    }
}

void UIList::AddCellAtPos(UIListCell *cell, float32 pos, float32 size, int32 index)
{
    DVASSERT(cell);
    DVASSERT(cell->cellStore == NULL || cell->cellStore == this);
    DVASSERT(index >= 0);
    if(!cell->cellStore)
    {
        cell->cellStore = this;
        cell->AddEvent(EVENT_TOUCH_UP_INSIDE, Message(this, &UIList::OnSelectEvent));
        Vector<UIListCell*> *store = GetStoreVector(cell->identifier);
        if(!store)
        {
            store = new Vector<UIListCell*>;
            cellStore[cell->identifier] = store;
        }
        store->push_back(cell);
    }
    cell->currentIndex = index;
    Rect r = cell->GetRect();
    if(orientation == ORIENTATION_HORIZONTAL)
    {
        r.dx = size;
        r.x = pos;
    }
    else
    {
        r.dy = size;
        r.y = pos;
    }
    cell->SetRect(r);
    cell->UpdateLayout();

    // Full refresh removes the cells and adds them again, losing the IsVisibleForUIEditor flag
    // (see please DF-2860). So need to recover it basing on what is set on parent's level.
    cell->SetVisibleForUIEditor(GetVisibleForUIEditor());
    scrollContainer->AddControl(cell);

}

Vector<UIListCell*> *UIList::GetStoreVector(const String &cellIdentifier)
{
    Map<String,Vector<UIListCell*>*>::const_iterator mit;
    mit = cellStore.find(cellIdentifier);
    if (mit == cellStore.end())
    {
        return NULL;
    }

    return mit->second;
}

UIListCell* UIList::GetReusableCell(const String &cellIdentifier)
{
    Vector<UIListCell*> *store = GetStoreVector(cellIdentifier);
    if(!store)
    {
        return NULL;
    }

    for(Vector<UIListCell*>::iterator it = store->begin(); it != store->end(); it++)
    {
        if((*it)->GetIndex() == -1)
        {
            return (*it);
        }
    }

    return NULL;

}

const List<UIControl*> &UIList::GetVisibleCells()
{
    return scrollContainer->GetChildren();
}


void UIList::SetTouchHoldDelta(int32 holdDelta)
{
    touchHoldSize = holdDelta;
}
int32 UIList::GetTouchHoldDelta()
{
    return touchHoldSize;
}

void UIList::SetSlowDownTime(float newValue)
{
    scroll->SetSlowDownTime(newValue);
}
void UIList::SetBorderMoveModifer(float newValue)
{
    scroll->SetBorderMoveModifer(newValue);
}

void UIList::SystemWillAppear()
{
    UIControl::SystemWillAppear();
    Refresh();
}

void UIList::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
{
    UIControl::LoadFromYamlNode(node, loader);

    const YamlNode * orientNode = node->Get("orientation");
    if (orientNode)
    {
        if (orientNode->AsString() == "ORIENTATION_VERTICAL")
            orientation = ORIENTATION_VERTICAL;
        else if (orientNode->AsString() == "ORIENTATION_HORIZONTAL")
            orientation = ORIENTATION_HORIZONTAL;
        else
        {
            DVASSERT(0 && "Orientation constant is wrong");
        }
    }
    // Load aggregator path
    const YamlNode * aggregatorPathNode = node->Get("aggregatorPath");
    if (aggregatorPathNode)
    {
        aggregatorPath = aggregatorPathNode->AsString();
    }

    // TODO
    InitAfterYaml();
}

UIControl *UIList::Clone()
{
    UIList *c = new UIList(GetRect(), this->orientation);
    c->CopyDataFrom(this);
    return c;
}

void UIList::CopyDataFrom(UIControl *srcControl)
{
    UIControl::CopyDataFrom(srcControl);
    UIList* t = (UIList*) srcControl;
    InitAfterYaml();
    aggregatorPath = t->aggregatorPath;
    orientation = t->orientation;
}

const FilePath & UIList::GetAggregatorPath()
{
    return aggregatorPath;
}

void UIList::SetAggregatorPath(const FilePath &aggregatorPath)
{
    this->aggregatorPath = aggregatorPath;
}

YamlNode * UIList::SaveToYamlNode(UIYamlLoader * loader)
{
    YamlNode *node = UIControl::SaveToYamlNode(loader);
    //Temp variables
    String stringValue;

    //Orientation
    eListOrientation orient = (eListOrientation)GetOrientation();
    switch(orient)
    {
        case ORIENTATION_VERTICAL:
            stringValue = "ORIENTATION_VERTICAL";
            break;
        case ORIENTATION_HORIZONTAL:
            stringValue = "ORIENTATION_HORIZONTAL";
            break;
        default:
            stringValue = "ORIENTATION_VERTICAL";
            break;
    }
    node->Set("orientation", stringValue);

    if (delegate)
    {
        // Set aggregator path from current List delegate
        delegate->SaveToYaml(this, node);
    }

    // Save aggregator path only if it is not empty
    if (!aggregatorPath.IsEmpty())
    {
        node->Set("aggregatorPath", aggregatorPath.GetFrameworkPath());
    }

    return node;
}

float32 UIList::VisibleAreaSize(UIScrollBar *forScrollBar)
{
    return scroll->GetViewSize();
}

float32 UIList::TotalAreaSize(UIScrollBar *forScrollBar)
{
    return scroll->GetElementSize();
}

float32 UIList::ViewPosition(UIScrollBar *forScrollBar)
{
    return scroll->GetPosition();
}

void UIList::OnViewPositionChanged(UIScrollBar *byScrollBar, float32 newPosition)
{
    scroll->SetPosition(-newPosition);
}

List<UIControl* >& UIList::GetRealChildren()
{
    List<UIControl* >& realChildren = UIControl::GetRealChildren();
    realChildren.remove(scrollContainer);
    return realChildren;
}

void UIList::ScrollToPosition( float32 position, float32 timeSec /*= 0.3f*/ )
{
    scroll->ScrollToPosition(-position);
}

const String UIList::GetDelegateControlPath(const UIControl *rootControl) const
{
    return UIControlHelpers::GetControlPath(this, rootControl);
}

};
