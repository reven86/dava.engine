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




#include "ComboBox.h"
#include "ControlsFactory.h"

ComboBox::ComboBox(const Rect &rect, ComboBoxDelegate *comboDelegate, const Vector<String> &listItems)
: UIControl(rect)
, maxVisibleItemsCount(0)
{
    delegate = comboDelegate;

    list = NULL;
    comboButton = ControlsFactory::CreateButton(Rect(0,0,size.x,size.y), L"");
    comboButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ComboBox::OnButton));
    AddControl(comboButton);

    SetNewItemsSet(listItems);
}

ComboBox::~ComboBox()
{
	SafeRelease(comboButton);

    SafeRelease(list);
}

int32 ComboBox::IndexByKey(const String &key)
{
	Map<String, int32>::const_iterator it;
	it = indecesMap.find(key);
    
    DVASSERT(it != indecesMap.end());
    
    return it->second;
}

void ComboBox::SetMaxVisibleItemsCount(int32 itemsCount)
{
	maxVisibleItemsCount = itemsCount;
}

void ComboBox::SetNewItemsSet(const Vector<String> &listItems)
{
    indecesMap.clear();
    items = listItems;

    Font *font = ControlsFactory::GetFont12();
    listWidth = size.x;
    for (int i = 0; i < (int32)items.size(); i++)
    {
        float itemWidth = (float32)font->GetStringSize(StringToWString(items[i])).dx;
        listWidth = Max(listWidth, itemWidth);
        
        indecesMap[items[i]] = i;
    }
    
    if(listWidth != size.x)
    {
        listWidth += (ControlsFactory::OFFSET * 2);
    }
    
//    int32 sz = Min(8, (int32)items.size());
    int32 sz = (int32)items.size();

	if(maxVisibleItemsCount > 0 && sz > maxVisibleItemsCount)
	{
		sz = maxVisibleItemsCount;
	}

    SafeRelease(list);
    list = new UIList(Rect(size.x - listWidth, size.y, listWidth, size.y * sz), UIList::ORIENTATION_VERTICAL);
    list->SetDelegate(this);
    ControlsFactory::SetScrollbar(list);
    ControlsFactory::CusomizeListControl(list);
    
    selectionIndex = 0;
    ControlsFactory::CustomizeButton(comboButton, StringToWString(items[selectionIndex]));
    SetSelectedIndex(0, false);
}


void ComboBox::SetSelectedIndex(int32 newSelecetedIndex, bool reportToDelegatge)
{
    selectionIndex = newSelecetedIndex;
    comboButton->SetStateText(UIControl::STATE_PRESSED_INSIDE, StringToWString(items[selectionIndex]));
    comboButton->SetStateText(UIControl::STATE_DISABLED, StringToWString(items[selectionIndex]));
    comboButton->SetStateText(UIControl::STATE_NORMAL, StringToWString(items[selectionIndex]));
    comboButton->SetStateText(UIControl::STATE_SELECTED, StringToWString(items[selectionIndex]));

        //todo: call delegate
    if (reportToDelegatge && delegate) 
    {
        delegate->OnItemSelected(this, items[selectionIndex], selectionIndex);
    }
    list->Refresh();
}

void ComboBox::SetSelectedKey(const String &newSelecetedKey)
{
    SetSelectedIndex(IndexByKey(newSelecetedKey), true);
}

int32 ComboBox::GetSelectedIndex()
{
    return selectionIndex;
}

const String &ComboBox::GetSelectedKey()
{
    return items[selectionIndex];
}


void ComboBox::OnButton(BaseObject * object, void * userData, void * callerData)
{
    if (list->GetParent()) 
    {
        Cancel();
    }
    else 
    {
        if(parent)
        {
            parent->BringChildFront(this);
        }

        
        UIScreen *screen = UIScreenManager::Instance()->GetScreen();
        if(screen)
        {
            Rect buttonRect = GetRect(true);
            Rect screenRect = screen->GetRect(true);
            
            float32 toRight = (screenRect.x + screenRect.dx) - (buttonRect.x);
            if(toRight < listWidth)
            {
                list->SetPosition(Vector2(buttonRect.x + buttonRect.dx - listWidth, buttonRect.y + buttonRect.dy));
            }
            else 
            {
                list->SetPosition(Vector2(buttonRect.x, buttonRect.y + buttonRect.dy));
            }

            screen->AddControl(list);
        }
        
        comboButton->SetSelected(true);
        list->Refresh();
    }

}

void ComboBox::Cancel()
{
    if (list->GetParent()) 
    {
        list->GetParent()->RemoveControl(list);
        comboButton->SetSelected(false);
    }
}

void ComboBox::Update(float32 timeElapsed)
{
    UIScreen *scr =  UIScreenManager::Instance()->GetScreen();
    if (list->GetParent() == scr) 
    {
        UIControl *f = UIControlSystem::Instance()->GetFocusedControl();
        bool isFocused = false;
        if (f == comboButton || f == list) 
        {
            isFocused = true;
        }
        else 
        {
            if (f && f->GetParent() && f->GetParent()->GetParent() == list)
            {
                isFocused = true;
            }
        }
        if (!isFocused) 
        {
            Cancel();
        }
    }
}


int32 ComboBox::ElementsCount(UIList *forList)
{
    return items.size();
}

UIListCell *ComboBox::CellAtIndex(UIList *forList, int32 index)
{
    UIListCell *c = forList->GetReusableCell("Combo cell");
    if (!c) 
    {
        c = new UIListCell(Rect(0, 0, listWidth, size.y), "Combo cell");
    }
    ControlsFactory::CustomizeButton(c, StringToWString(items[index]));
    if (index == selectionIndex) 
    {
        c->SetSelected(true);
    }
    else 
    {
        c->SetSelected(false);
    }

    return c;
}

int32 ComboBox::CellHeight(UIList *forList, int32 index)
{
    return (int32)size.y;
}

void ComboBox::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
    SetSelectedIndex(selectedCell->GetIndex(), true);
    comboButton->SetSelected(false);
	if(list->GetParent())
	{
		list->GetParent()->RemoveControl(list);
	}
}

void ComboBox::WillDisappear()
{
    Cancel();
}
