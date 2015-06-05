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


#include "ChangeFontPropertyCommand.h"

#include "EditorFontManager.h"

ChangeFontPropertyCommandData::ChangeFontPropertyCommandData()
: isApplyToAll(false)
, font(NULL)
{
}
ChangeFontPropertyCommandData::~ChangeFontPropertyCommandData()
{
    SafeRelease(font);
    Map<String, Font*>::iterator it = localizedFonts.begin();
    Map<String, Font*>::iterator endIt = localizedFonts.end();
    for(; it != endIt; ++it)
    {
        SafeRelease(it->second);
    }
    localizedFonts.clear();
}

ChangeFontPropertyCommandData& ChangeFontPropertyCommandData::operator =(const ChangeFontPropertyCommandData& data)
{
    font = SafeRetain(data.font);
    isApplyToAll = data.isApplyToAll;
    fontPresetOriginalName = data.fontPresetOriginalName;
    fontPresetName = data.fontPresetName;
    
    Map<String, Font*>::const_iterator it = data.localizedFonts.begin();
    Map<String, Font*>::const_iterator endIt = data.localizedFonts.end();
    for(; it != endIt; ++it)
    {
        localizedFonts[it->first] = SafeRetain(it->second);
    }
    
    return *this;
}

Font *ChangeFontPropertyCommandData::GetLocalizedFont(const String& locale)
{
    Map<String, Font*>::const_iterator findIt = localizedFonts.find(locale);
    Map<String, Font*>::const_iterator endIt = localizedFonts.end();
    if(findIt != endIt)
    {
        return findIt->second;
    }
    return NULL;
}

void ChangeFontPropertyCommandData::SetLocalizedFont(Font* localizedFont, const String& locale)
{
    Map<String, Font*>::iterator findIt = localizedFonts.find(locale);
    Map<String, Font*>::const_iterator endIt = localizedFonts.end();
    if(findIt != endIt)
    {
        if(findIt->second != localizedFont)
        {
            SafeRelease(findIt->second);
            findIt->second = SafeRetain(localizedFont);
        }
    }
}

ChangeFontPropertyCommand::ChangeFontPropertyCommand(BaseMetadata* baseMetadata,
								const PropertyGridWidgetData& propertyGridWidgetData,
                                const ChangeFontPropertyCommandData& changeFontPropertyCommandData):
ChangePropertyCommand<Font *>(baseMetadata, propertyGridWidgetData, changeFontPropertyCommandData.font)
{
    data = changeFontPropertyCommandData;
}

ChangeFontPropertyCommand::~ChangeFontPropertyCommand()
{
}

void ChangeFontPropertyCommand::Execute()
{
    //TODO: save data to be able to revert changes
    Logger::FrameworkDebug("ChangeFontPropertyCommand::Execute SetLocalizedFont %s %p %s default", data.fontPresetOriginalName.c_str(), data.font, data.fontPresetName.c_str());
    String newFontPresetName = EditorFontManager::Instance()->SetLocalizedFont(data.fontPresetOriginalName, data.font, data.fontPresetName, data.isApplyToAll, "default");
    
    Map<String, Font*>::iterator it = data.localizedFonts.begin();
    Map<String, Font*>::const_iterator endIt = data.localizedFonts.end();
    for(; it != endIt; ++it)
    {
        Logger::FrameworkDebug("ChangeFontPropertyCommand::Execute SetLocalizedFont %s %p %s %s", data.fontPresetOriginalName.c_str(), it->second, data.fontPresetName.c_str(), it->first.c_str());
        String localizedNewFontPresetName = EditorFontManager::Instance()->SetLocalizedFont(data.fontPresetOriginalName, it->second, newFontPresetName, data.isApplyToAll, it->first);
        if(localizedNewFontPresetName != newFontPresetName)
        {
            Logger::Error("ChangeFontPropertyCommand::Execute newFontPresetName=%s is different for locale %s: localizedNewFontPresetName=%s", newFontPresetName.c_str(), it->first.c_str(), localizedNewFontPresetName.c_str());
        }
    }
    
    if(data.isApplyToAll)
    {
        Logger::FrameworkDebug("ChangeFontPropertyCommand::Execute TODO: apply to all fonts");
        //TODO: set new font in all controls that use this font
    }
    
    // apply current font change
    return ChangePropertyCommand<Font *>::Execute();
}

void ChangeFontPropertyCommand::Rollback()
{
	// The previous values are stored in Command Data.
	QString propertyName = GetPropertyName();
    for (COMMANDDATAVECTITER iter = this->commandData.begin(); iter != commandData.end(); iter ++)
    {
		Font* previousValue = (*iter).GetTreeNodePropertyValue();
		bool propertySetOK = ApplyPropertyValue(iter, previousValue);
		// Always restore control rect for align property while rollback
//		RestoreControlRect(iter);
        
        //TODO: revert changes
        

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


