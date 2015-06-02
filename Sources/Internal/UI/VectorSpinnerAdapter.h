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


#ifndef __DAVAENGINE_UI_VECTOR_SPINNER_ADAPTER_H__
#define __DAVAENGINE_UI_VECTOR_SPINNER_ADAPTER_H__

#include "UISpinner.h"

namespace DAVA
{

/*
 * It's an abstract class: you need to implement your display logic in DisplaySelectedData(UISpinner * spinner) to use it.
 */
template<typename T> class VectorSpinnerAdapter : public SpinnerAdapter
{
public:
    VectorSpinnerAdapter(const Vector<T> &aCollection, uint32 aSelectedIndex = 0)
        : data(aCollection)
        , selectedIndex(aSelectedIndex)
    {};

    virtual ~VectorSpinnerAdapter() {};

    virtual bool IsSelectedLast() const
    {
        return selectedIndex == data.size() - 1;
    }

    virtual bool IsSelectedFirst() const
    {
        return selectedIndex == 0;
    }

    uint32 GetSelected() {return selectedIndex;}

    void SetSelected(uint32 aSelectedIndex)
    {
        DVASSERT(0 <= aSelectedIndex && aSelectedIndex < data.size());
        if (selectedIndex != aSelectedIndex)
        {
            selectedIndex = aSelectedIndex;
            NotifyObservers(IsSelectedFirst(), IsSelectedLast(), true);
        }
    }

    /*
     * Call it after you made changes to dataset via 'data' field.
     * You should specify if selected element data was changed with 'isSelectedChanged' argument.
     */
    void NotifyDataSetChange(bool isSelectedChanged)
    {
        NotifyObservers(IsSelectedFirst(), IsSelectedLast(), isSelectedChanged);
    }

    Vector<T> data;

protected:
    virtual bool SelectNext()
    {
        bool isSelectedLast = IsSelectedLast();
        if (!isSelectedLast)
            selectedIndex++;
        return !isSelectedLast;
    }

    virtual bool SelectPrevious()
    {
        bool isSelectedFirst = IsSelectedFirst();
        if (!isSelectedFirst)
            selectedIndex--;
        return !isSelectedFirst;
    }

    uint32 selectedIndex;
};

}

#endif //__DAVAENGINE_UI_VECTOR_SPINNER_ADAPTER_H__
