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


#include "ChangeAlignPropertyCommand.h"

ChangeAlignPropertyCommand::ChangeAlignPropertyCommand(BaseMetadata* baseMetadata,
								const PropertyGridWidgetData& propertyGridWidgetData,
                                bool value):
ChangePropertyCommand<bool>(baseMetadata, propertyGridWidgetData, value)
{
}

void ChangeAlignPropertyCommand::Rollback()
{
	// The previous values are stored in Command Data.
	QString propertyName = GetPropertyName();
    for (COMMANDDATAVECTITER iter = this->commandData.begin(); iter != commandData.end(); iter ++)
    {
		bool previousValue = (*iter).GetTreeNodePropertyValue();
		bool propertySetOK = ApplyPropertyValue(iter, previousValue);
		// Always restore control rect for align property while rollback
		RestoreControlRect(iter);

        if (propertySetOK)
        {			
            CommandsController::Instance()->EmitChangePropertySucceeded(propertyName);
        }
        else
        {
			CommandsController::Instance()->EmitChangePropertyFailed(propertyName);
        }
    }
}

void ChangeAlignPropertyCommand::RestoreControlRect(const COMMANDDATAVECTITER& iter)
{
	BaseMetadata* baseMetadata = GetMetadataForTreeNode((*iter).GetTreeNodeID());
	if (baseMetadata)
	{
		Rect rect = (*iter).GetTreeNodeRect();
		// This command is NOT state-aware and contains one and only param.
		baseMetadata->SetActiveParamID(0);
		// Restore control position and size
		baseMetadata->ApplyMove(Vector2(rect.x, rect.y), false);
		baseMetadata->ApplyResize(Rect(), rect);
	}
}