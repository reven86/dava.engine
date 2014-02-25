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





#ifndef COMBO_BOX
#define COMBO_BOX

#include "DAVAEngine.h"

using namespace DAVA;

class ComboBox;
class ComboBoxDelegate
{
public:
    virtual void OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex){};
};


class ComboBox : public UIControl, public UIListDelegate
{
protected:
    ~ComboBox();
public:
    
    ComboBox(const Rect &rect, ComboBoxDelegate *comboDelegate, const Vector<String> &listItems);

    
    void SetNewItemsSet(const Vector<String> &listItems);
    void SetMaxVisibleItemsCount(int32 itemsCount);
    
    void SetSelectedIndex(int32 newSelecetedIndex, bool reportToDelegatge);
    void SetSelectedKey(const String &newSelecetedKey);
    int32 GetSelectedIndex();
    const String &GetSelectedKey();
    
    void Cancel();
    
    void OnButton(BaseObject * object, void * userData, void * callerData);

    void Update(float32 timeElapsed);
    
    
    virtual int32 ElementsCount(UIList *forList);
	virtual UIListCell *CellAtIndex(UIList *forList, int32 index);
	virtual int32 CellWidth(UIList *forList, int32 index)//calls only for horizontal orientation
	{return 20;};
	virtual int32 CellHeight(UIList *forList, int32 index);//calls only for vertical orientation
	virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);
    
    
    virtual void WillDisappear();
    
protected:
    int32 IndexByKey(const String &key);

    ComboBoxDelegate *delegate;
    UIList *list;
    Vector<String> items;
    Map<String, int32> indecesMap;
    UIButton *comboButton;
    
    int32 selectionIndex;
    
    float32 listWidth;
	int32 maxVisibleItemsCount;
    
};

#endif