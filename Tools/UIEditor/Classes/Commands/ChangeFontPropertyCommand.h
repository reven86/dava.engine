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


#ifndef CHANGEFONTPROPERTYCOMMAND_H
#define CHANGEFONTPROPERTYCOMMAND_H

#include "BaseCommand.h"
#include "HierarchyTreeNode.h"
#include "PropertyGridWidgetData.h"
#include "PropertiesHelper.h"
#include "ChangePropertyCommand.h"

using namespace DAVA;

struct ChangeFontPropertyCommandData
{
    ChangeFontPropertyCommandData();
    ~ChangeFontPropertyCommandData();
    
    Font *GetLocalizedFont(const String& locale);
    void SetLocalizedFont(Font* localizedFont, const String& locale);
    
    ChangeFontPropertyCommandData& operator =(const ChangeFontPropertyCommandData& data);
    
    String fontPresetOriginalName;
    
    bool isApplyToAll;
    Font *font;
    String fontPresetName;
    
    Map<String, Font*> localizedFonts;
};

class ChangeFontPropertyCommand: public ChangePropertyCommand<Font *>
{
public:
	ChangeFontPropertyCommand(BaseMetadata* baseMetadata,
								const PropertyGridWidgetData& propertyGridWidgetData,
                                const ChangeFontPropertyCommandData& editFontDialogResult);
    virtual ~ChangeFontPropertyCommand();
    
    virtual void Execute();
    
	virtual void Rollback();
	
protected:
//	void RestoreControlRect(const COMMANDDATAVECTITER& iter);
    ChangeFontPropertyCommandData data;
};

class FontRenameCommand : public BaseCommand
{
public:
    FontRenameCommand(Font* _font, const String &_originalName, const String &newName);
    
public:
	virtual void Execute();
	virtual void Rollback();
	
	virtual bool IsUndoRedoSupported() {return true;};
    
protected:
	// Apply the rename of control
	void ApplyRename(const QString& prevName, const QString& updatedName);
    
private:
	Font* font;
	QString originalName;
	QString newName;
};


#endif /* defined(CHANGEFONTPROPERTYCOMMAND_H) */
