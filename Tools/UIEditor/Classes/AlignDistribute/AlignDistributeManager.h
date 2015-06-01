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


#ifndef __ALIGNDIISTRIBUTEMANAGER__H__
#define __ALIGNDIISTRIBUTEMANAGER__H__

#include "DAVAEngine.h"
#include "AlignDistributeEnums.h"
#include "ControlsPositionData.h"

#include "AlignHandlers.h"
#include "DistributeHandlers.h"

namespace DAVA {

// Controls Alignment/Distribution Manager for UI Editor.
class AlignDistributeManager
{
public:
	// Align controls according to the type passed, return previous position of the
	// controls (needed for undo).
	static ControlsPositionData AlignControls(const List<UIControl*>& controlsList, eAlignControlsType alingType);

	// Distribute controls according to the type passed, return previous position of the
	// controls (needed for undo).
	static ControlsPositionData DistributeControls(const List<UIControl*>& controlsList, eDistributeControlsType distributeType);

	// Undo the alignment/distribution, return the controls back to their positions.
	static void UndoAlignDistribute(const ControlsPositionData& positionData);
	
protected:
	// Create the appropriate Align/Distribute Handler.
	static BaseAlignHandler* CreateAlignHandler(eAlignControlsType alignType);
	static BaseDistributeHandler* CreateDistributeHandler(eDistributeControlsType distributeType);
};

};

#endif /* __ALIGNDIISTRIBUTEMANAGER__H__ */
