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


#ifndef __DAVAENGINE_UI_STYLESHEET_H__
#define __DAVAENGINE_UI_STYLESHEET_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "FileSystem/VariantType.h"
#include "UI/Styles/UIStyleSheetPropertiesTable.h"

namespace DAVA
{
    struct UIStyleSheetSelector
    {
        UIStyleSheetSelector() :
            controlClassName(""),
            name(),
            controlStateMask(0)
        {

        }

        String controlClassName;
        FastName name;
        int32 controlStateMask;
        Vector<FastName> classes;
    };

    class UIStyleSheetPropertyTable :
        public BaseObject
    {
    public:
        void SetProperties(const Vector<std::pair<uint32, VariantType>>& properties);
        const Vector<std::pair<uint32, VariantType>>& GetProperties() const;
    private:
        Vector<std::pair<uint32, VariantType>> properties;
    };

    class UIStyleSheet :
        public BaseObject
    {
    protected:
        virtual ~UIStyleSheet();
    public:
        UIStyleSheet();

        int32 GetScore() const;

        const UIStyleSheetPropertyTable* GetPropertyTable() const;
        const Vector< UIStyleSheetSelector >& GetSelectorChain() const;

        void SetPropertyTable(UIStyleSheetPropertyTable* properties);
        void SetSelectorChain(const Vector<UIStyleSheetSelector>& selectorChain);
    private:
        void RecalculateScore();

        Vector<UIStyleSheetSelector> selectorChain;
        
        RefPtr<UIStyleSheetPropertyTable> properties;

        int32 score;
    };
};


#endif
