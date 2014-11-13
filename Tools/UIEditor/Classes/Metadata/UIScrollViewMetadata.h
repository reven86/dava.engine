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

#ifndef __UIEditor__UIScrollViewMetadata__
#define __UIEditor__UIScrollViewMetadata__

#include "UIControlMetadata.h"
#include "UI/UIScrollView.h"

namespace DAVA {

// Metadata class for DAVA UIList control.
class UIScrollViewMetadata : public UIControlMetadata
{
    Q_OBJECT
	
    // Horizontal position of scroll
    Q_PROPERTY(float HorizontalScrollPosition READ GetHorizontalScrollPosition WRITE SetHorizontalScrollPosition);
    Q_PROPERTY(float VerticalScrollPosition READ GetVerticalScrollPosition WRITE SetVerticalScrollPosition);
	Q_PROPERTY(float ContentSizeX READ GetContentSizeX WRITE SetContentSizeX);
	Q_PROPERTY(float ContentSizeY READ GetContentSizeY WRITE SetContentSizeY);
	
	
public:
    UIScrollViewMetadata(QObject* parent = 0);

protected:
    virtual bool GetInitialInputEnabled() const {return true;};

    // Initialize the appropriate control.
    virtual void InitializeControl(const String& controlName, const Vector2& position);

    virtual QString GetUIControlClassName() const { return "UIScrollView"; };
	
    // Helper to access active UI ScrollView.
    UIScrollView* GetActiveUIScrollView() const;
	
    // Getters/setters.
    float GetHorizontalScrollPosition() const;
	void SetHorizontalScrollPosition(float value);
    float GetVerticalScrollPosition() const;
	void SetVerticalScrollPosition(float value);
	float GetContentSizeX() const;
	void SetContentSizeX(float value);
	float GetContentSizeY() const;
	void SetContentSizeY(float value);
};

};

#endif /* defined(__UIEditor__UIScrollViewMetadata__) */
