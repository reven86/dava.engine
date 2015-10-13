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


#include "UISpinner.h"
#include "UI/UIEvent.h"
#include "Animation/Animation.h"

namespace DAVA 
{

//use these names for children buttons to define UISpinner in .yaml
static const String UISPINNER_BUTTON_NEXT_NAME = "buttonNext";
static const String UISPINNER_BUTTON_PREVIOUS_NAME = "buttonPrevious";
static const String UISPINNER_CONTENT_NAME = "content";
static const float32 ANIMATION_TIME = 0.1f;
static const int32 MOVE_ANIMATION_TRACK = 10;
static const float32 X_UNDEFINED = 10000;
static const float32 SLIDE_GESTURE_SPEED = 20.f;
static const float32 SLIDE_GESTURE_TIME = 0.1f;
    
void SpinnerAdapter::AddObserver(SelectionObserver* anObserver)
{
    observers.insert(anObserver);
}

void SpinnerAdapter::RemoveObserver(SelectionObserver* anObserver)
{
    observers.erase(anObserver);
}

void SpinnerAdapter::NotifyObservers(bool isSelectedFirst, bool isSelectedLast, bool isSelectedChanged)
{
    Set<SelectionObserver*>::const_iterator end = observers.end();
    for (Set<SelectionObserver*>::iterator it = observers.begin(); it != end; ++it)
    {
        (*it)->OnSelectedChanged(isSelectedFirst, isSelectedLast, isSelectedChanged);
    }
}

bool SpinnerAdapter::Next()
{
    bool completedOk = SelectNext();
    if (completedOk)
        NotifyObservers(false /*as we selected next it can't be first*/, IsSelectedLast(), true);
    return completedOk;
}

bool SpinnerAdapter::Previous()
{
    bool completedOk = SelectPrevious();
    if (completedOk)
        NotifyObservers(IsSelectedFirst(), false /*as we selected previous it can't be last*/, true);
    return completedOk;
}

void SpinnerAdapter::DisplaySelectedData(UISpinner * spinner)
{
    FillScrollableContent(spinner->GetContent(), CURRENT);
}
    
    
UISpinner::UISpinner(const Rect &rect)
    : UIControl(rect)
    , adapter(nullptr)
    , buttonNext(new UIButton())
    , buttonPrevious(new UIButton())
    , content(new UIControl())
    , nextContent(new UIControl())
    , contentViewport(new UIControl())
    , dragAnchorX(X_UNDEFINED)
    , previousTouchX(X_UNDEFINED)
    , currentTouchX(X_UNDEFINED)
    , totalGestureTime(0)
    , totalGestureDx(0)
{
    buttonNext->SetName(UISPINNER_BUTTON_NEXT_NAME);
    buttonPrevious->SetName(UISPINNER_BUTTON_PREVIOUS_NAME);
    content->SetName(UISPINNER_CONTENT_NAME);

    AddControl(buttonNext.Get());
    AddControl(buttonPrevious.Get());
    AddControl(content.Get());

    contentViewport->AddControl(nextContent.Get());
    contentViewport->SetInputEnabled(false);
    contentViewport->SetClipContents(true);
}

UISpinner::~UISpinner()
{
    SetAdapter(nullptr);
}

void UISpinner::Update(DAVA::float32 timeElapsed)
{
    if (currentTouchX < X_UNDEFINED)
    {
        Move move;
        move.dx = currentTouchX - previousTouchX;
        move.time = timeElapsed;
        moves.push_back(move);
        totalGestureDx += move.dx;
        totalGestureTime += move.time;
        if (totalGestureTime > SLIDE_GESTURE_TIME)
        {
            List<Move>::iterator it = moves.begin();
            totalGestureTime -= it->time;
            totalGestureDx -= it->dx;
            moves.erase(it);
        }
        previousTouchX = currentTouchX;
    }
}
    
void UISpinner::Input(UIEvent *currentInput)
{
    if (NULL == adapter)
    {
        return;
    }

    if (content->IsAnimating(MOVE_ANIMATION_TRACK))
    {
        return;
    }
    
    Vector2 touchPos = currentInput->point;
    if (currentInput->phase == UIEvent::Phase::BEGAN)
    {
        if (content->IsPointInside(touchPos))
        {
            content->relativePosition = Vector2();
            content->SetPivot(Vector2());
            
            contentViewport->AddControl(content.Get());
            AddControl(contentViewport.Get());
            dragAnchorX = touchPos.x - content->relativePosition.x;
            currentTouchX = touchPos.x;
            previousTouchX = currentTouchX;
        }
        else
        {
            dragAnchorX = X_UNDEFINED;
        }    
    }
    else if (currentInput->phase == UIEvent::Phase::DRAG)
    {
        if (dragAnchorX < X_UNDEFINED)
        {
            currentTouchX = touchPos.x;
            float32 contentNewX = touchPos.x - dragAnchorX;
            float32 contentNewLeftEdge = contentNewX - content->GetPivotPoint().x;
            if (!(contentNewLeftEdge < 0 && adapter->IsSelectedLast()) && !(contentNewLeftEdge > 0 && adapter->IsSelectedFirst()))
            {
                if (contentNewX != 0)
                {
                    if (content->relativePosition.x * contentNewX <= 0) //next content just appears or visible side changes
                    {
                        //adjust nextContent->pivotPoint to make more convenient setting of nextContent->relativePosition below
                        Vector2 newPivotPoint = nextContent->GetPivotPoint();
                        newPivotPoint.x = contentNewX > 0 ? content->size.dx : -content->size.dx;
                        nextContent->SetPivotPoint(newPivotPoint);
                        adapter->FillScrollableContent(nextContent.Get(), contentNewX > 0 ? SpinnerAdapter::PREVIOUS : SpinnerAdapter::NEXT);
                    }
                }
                content->relativePosition.x = contentNewX;
                nextContent->relativePosition.x = contentNewX; //for this to work we adjust pivotPoint above
                
                if (Abs(content->relativePosition.x) > content->size.dx / 2)
                {
                    OnSelectWithSlide(content->relativePosition.x > 0);
                    dragAnchorX = touchPos.x - content->relativePosition.x;
                }
            }
        }
    }
    else if (currentInput->phase == UIEvent::Phase::ENDED || currentInput->phase == UIEvent::Phase::CANCELLED)
    {
        if (dragAnchorX < X_UNDEFINED)
        {
            if (totalGestureTime > 0)
            {
                float32 averageSpeed = totalGestureDx / totalGestureTime;
                bool selectPrevious = averageSpeed > 0;
                if (selectPrevious == content->relativePosition.x > 0) //switch only if selected item is already shifted in slide direction
                {
                    bool isSelectedLast = selectPrevious ? adapter->IsSelectedFirst() : adapter->IsSelectedLast();
                    if (Abs(averageSpeed) > SLIDE_GESTURE_SPEED && !isSelectedLast)
                    {
                        OnSelectWithSlide(selectPrevious);
                    }
                }
            }
            
            Animation* animation = content->PositionAnimation(Vector2(0, content->relativePosition.y), ANIMATION_TIME, Interpolation::EASY_IN, MOVE_ANIMATION_TRACK);
            animation->AddEvent(Animation::EVENT_ANIMATION_END, Message(this, &UISpinner::OnScrollAnimationEnd));
            nextContent->PositionAnimation(Vector2(0, content->relativePosition.y), ANIMATION_TIME, Interpolation::EASY_IN, MOVE_ANIMATION_TRACK);

            currentTouchX = X_UNDEFINED;
            previousTouchX = X_UNDEFINED;
            dragAnchorX = X_UNDEFINED;
            moves.clear();
            totalGestureTime = 0;
            totalGestureDx = 0;
        }
    }

    currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_HARD); // Drag is handled - see please DF-2508.
}
    
void UISpinner::OnSelectWithSlide(bool isPrevious)
{
    RefPtr<UIControl> temp = content;
    content = nextContent;
    nextContent = temp;
    
    //save display position but change pivot points
    Vector2 newPivotPoint = nextContent->GetPivotPoint();
    newPivotPoint.x -= content->GetPivotPoint().x;
    nextContent->SetPivotPoint(newPivotPoint);

    nextContent->relativePosition.x -= content->GetPivotPoint().x;
    content->relativePosition.x -= content->GetPivotPoint().x;

    newPivotPoint = content->GetPivotPoint();
    newPivotPoint.x = 0;
    content->SetPivotPoint(newPivotPoint);
    
    if (isPrevious)
        adapter->Previous();
    else
        adapter->Next();
}
    
void UISpinner::OnScrollAnimationEnd(BaseObject * caller, void * param, void *callerData)
{
    DVASSERT(NULL != contentViewport->GetParent());
    content->SetPivotPoint(contentViewport->GetPivotPoint());
    content->relativePosition = contentViewport->relativePosition;
    RemoveControl(contentViewport.Get());
    AddControl(content.Get());
}

void UISpinner::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
{
    //release default buttons - they have to be loaded from yaml
    RemoveAllControls();
    content = nullptr;
    UIControl::LoadFromYamlNode(node, loader);
}

void UISpinner::CopyDataFrom(UIControl *srcControl)
{
    UIControl::CopyDataFrom(srcControl);
    SetupInternalControls();
    UISpinner *src = DynamicTypeCheck<UISpinner *>(srcControl);
    SetAdapter(src->GetAdater());
}

UISpinner* UISpinner::Clone()
{
    UISpinner *control = new UISpinner(GetRect());
    control->CopyDataFrom(this);
    return control;
}

void UISpinner::AddControl(UIControl *control)
{
    UIControl::AddControl(control);

    if (control->GetName() == UISPINNER_BUTTON_NEXT_NAME && control != buttonNext.Get())
    {
        buttonNext = DynamicTypeCheck<UIButton*>(control);
    }
    else if (control->GetName() == UISPINNER_BUTTON_PREVIOUS_NAME && control != buttonPrevious.Get())
    {
        buttonPrevious = DynamicTypeCheck<UIButton*>(control);
    }
}

void UISpinner::RemoveControl(UIControl *control)
{
    if (control == buttonNext.Get())
    {
        buttonNext = nullptr;
    }
    else if (control == buttonPrevious.Get())
    {
        buttonPrevious = nullptr;
    }
    
    UIControl::RemoveControl(control);
}

void UISpinner::LoadFromYamlNodeCompleted()
{
    SetupInternalControls();
    SetAdapter(nullptr);
}

YamlNode * UISpinner::SaveToYamlNode(UIYamlLoader * loader)
{
    buttonPrevious->SetName(UISPINNER_BUTTON_PREVIOUS_NAME);
	buttonNext->SetName(UISPINNER_BUTTON_NEXT_NAME);
	content->SetName(UISPINNER_CONTENT_NAME);

	YamlNode *node = UIControl::SaveToYamlNode(loader);
	return node;
}
	
List<UIControl* > UISpinner::GetSubcontrols()
{
	List<UIControl* > subControls;

	// Lookup for the contols by their names.
	AddControlToList(subControls, UISPINNER_BUTTON_PREVIOUS_NAME);
	AddControlToList(subControls, UISPINNER_BUTTON_NEXT_NAME);
	AddControlToList(subControls, UISPINNER_CONTENT_NAME);

	return subControls;
}

void UISpinner::SetAdapter(SpinnerAdapter * anAdapter)
{
    buttonNext->RemoveEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UISpinner::OnNextPressed));
    buttonPrevious->RemoveEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UISpinner::OnPreviousPressed));

    if (adapter)
    {
        adapter->RemoveObserver(this);
        SafeRelease(adapter);
    }

    adapter = SafeRetain(anAdapter);
    if (adapter)
    {
        adapter->DisplaySelectedData(this);
        adapter->AddObserver(this);

        buttonNext->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UISpinner::OnNextPressed));
        buttonPrevious->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UISpinner::OnPreviousPressed));
    }

    buttonNext->SetDisabled(!adapter || adapter->IsSelectedLast());
    buttonPrevious->SetDisabled(!adapter || adapter->IsSelectedFirst());
}

void UISpinner::OnNextPressed(DAVA::BaseObject * caller, void * param, void *callerData)
{
    if (content->IsAnimating(MOVE_ANIMATION_TRACK))
    {
        return;
    }
    //buttonNext is disabled if we have no adapter or selected adapter element is last, so we don't need checks here
    adapter->Next();
}    

void UISpinner::OnPreviousPressed(DAVA::BaseObject * caller, void * param, void *callerData)
{
    if (content->IsAnimating(MOVE_ANIMATION_TRACK))
    {
        return;
    }
    //buttonPrevious is disabled if we have no adapter or selected adapter element is first, so we don't need checks here
    adapter->Previous();
}

void UISpinner::OnSelectedChanged(bool isSelectedFirst, bool isSelectedLast, bool isSelectedChanged)
{
    buttonNext->SetDisabled(isSelectedLast);
    buttonPrevious->SetDisabled(isSelectedFirst);
    if (isSelectedChanged)
    {
        adapter->DisplaySelectedData(this);
        PerformEvent(UIControl::EVENT_VALUE_CHANGED);
    }
}

void UISpinner::SetupInternalControls()
{
    content = FindByPath(UISPINNER_CONTENT_NAME);
    content->SetInputEnabled(false);
    contentViewport->SetRect(content->GetRect());
    contentViewport->SetPivotPoint(content->GetPivotPoint());
    nextContent->CopyDataFrom(content.Get());
    nextContent->relativePosition = Vector2();
    Vector2 newPivotPoint = nextContent->GetPivotPoint();
    newPivotPoint.x = content->size.dx;
    nextContent->SetPivotPoint(newPivotPoint);
}

}