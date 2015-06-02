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



#ifndef __UIEditor__EditorListDelegate__
#define __UIEditor__EditorListDelegate__

#include "Base/BaseTypes.h"
#include "UI/UIList.h"

namespace DAVA {

class EditorListDelegate : public UIControl, public UIListDelegate
{
	const static int32 CELL_COUNT = 3;
	const static int32 DEFAULT_AGGREGATOR_ID = 0;
	const static int32 DEFAULT_CELL_HEIGHT = 30;
	const static int32 DEFAULT_CELL_WIDTH = 100;
protected:
	virtual ~EditorListDelegate();
public:
	EditorListDelegate(UIList* list, bool rectInAbsoluteCoordinates = false);

	// UIListDelegate
    virtual int32 ElementsCount(UIList *forList);
	virtual UIListCell *CellAtIndex(UIList *forList, int32 index);
	virtual float32 CellHeight(UIList *forList, int32 index);
	virtual float32 CellWidth(UIList *forList, int32 index);
	virtual void SaveToYaml(UIList *forList, YamlNode *node);
	
	void SetAggregatorID(int32 agId);
	int32 GetAggregatorID();
	
	void ResetElementsCount();

private:
	int32 aggregatorID;
	int32 cellsCount;
	Vector2 cellSize;
	bool isElementsCountNeedUpdate;
	
	void SetCellSize(const Vector2 &size);
	void UpdateCellSize(UIList *forList);
	UIControl *GetCurrentAggregatorControl();
    UIList* currentList;
};

};

#endif /* defined(__UIEditor__EditorListDelegate__) */
