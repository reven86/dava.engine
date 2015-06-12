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


#ifndef __DAVAENGINE_UI_LIST_CELL_H__
#define __DAVAENGINE_UI_LIST_CELL_H__

#include "UI/UIButton.h"

namespace DAVA 
{
	/**
	 \ingroup controlsystem
	 \brief Cell unit for the UIList.
		UIButton that can be managed by the UIList.
	 */
	
	class UIListCell : public UIButton 
	{
		friend class UIList;
	public:
		/**
		 \brief Constructor.
		 \param[in] rect Used only size part of the rect. Incoming rect size can be modified by the UIList if this is neccesary.
		 \param[in] cellIdentifier literal identifier to represents cell type. For example: "Name cell", "Phone number cell", etc.
		 */
		UIListCell(const Rect &rect = Rect(), const String &cellIdentifier = String(), const FilePath &aggregatorPath = FilePath());

        /**
		 \brief Returns cell's identifier.
		 \returns identifier
		 */
        const String & GetIdentifier() const;

        /**
		 \brief set cell's identifier.
		 \param[in] new cell identifier
		 */
        void SetIdentifier(const String &identifier);
		/**
		 \brief Returns current cell sequence number in the list.
		 \returns list item index
		 */
		int32 GetIndex() const;
        
        virtual UIListCell *Clone();
        void CopyDataFrom(UIControl *srcControl);
        
        virtual void LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader);
        virtual YamlNode * SaveToYamlNode(UIYamlLoader * loader);
		
	protected:
		virtual ~UIListCell();

		virtual void WillDisappear();

		
	private:
		int32 currentIndex;
		String identifier;
		
		void *cellStore;
    public:
        INTROSPECTION_EXTEND(UIListCell, UIButton,
            PROPERTY("identifier", "Cell identifier", GetIdentifier, SetIdentifier, I_SAVE | I_VIEW | I_EDIT)
            );
    };
}

#endif