/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#import <Foundation/Foundation.h>
#include "FileSystem/LocalizationIPhone.h"
#include "FileSystem/LocalizationSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/Logger.h"


namespace DAVA
{
void LocalizationIPhone::SelecePreferedLocalizationForPath(const FilePath &directoryPath)
{
    NSString * lang = [[NSUserDefaults standardUserDefaults] stringForKey:@"lang"];
    String lid;
    
    if(lang)
    {
        lid = [lang UTF8String];
		File *fl = File::Create(directoryPath + (lid + ".yaml"), File::OPEN|File::READ);
		if(fl)
		{
			Logger::Info("LocalizationIPhone:: selected lang = %s", lid.c_str());
			LocalizationSystem::Instance()->SetCurrentLocale(lid);
			SafeRelease(fl);
			return;
		}        
    }
    else
    {
        NSArray *ar = [NSLocale preferredLanguages];

        lid = [[ar objectAtIndex:0] UTF8String];

        Logger::Info("LocalizationIPhone:: pref lang = %s", lid.c_str());
        File *fl = File::Create(directoryPath + (lid + ".yaml"), File::OPEN|File::READ);
        if(fl)
        {
            Logger::Info("LocalizationIPhone:: selected lang = %s", lid.c_str());
            LocalizationSystem::Instance()->SetCurrentLocale(lid);
            SafeRelease(fl);
            return;
        }
    }
    
    // we can reach this point only if we don't have yaml file for the preferred language
    if(lid.size() > 2)
    {
        String langPart = lid.substr(0, 2);
        Logger::Info("LocalizationIPhone:: pref lang = %s not found, trying to check language part %s", lid.c_str(), langPart.c_str());
        File *fl1 = File::Create(directoryPath + (langPart + ".yaml"), File::OPEN|File::READ);
        if(fl1)
        {
            Logger::Info("LocalizationIPhone:: selected lang = %s", langPart.c_str());
            LocalizationSystem::Instance()->SetCurrentLocale(langPart);
            SafeRelease(fl1);
            return;
        }
        Logger::Info("LocalizationIPhone:: pref lang = %s language part %s not found", lid.c_str(), langPart.c_str());
    }
    
    // we can reach this point only if we don't have yaml file for the language part of the preferred language
    File *fl2 = File::Create(directoryPath + ("en.yaml"), File::OPEN|File::READ);
    if(fl2)
    {
        Logger::Info("LocalizationIPhone:: selected lang = en (default)");
        LocalizationSystem::Instance()->SetCurrentLocale("en");
        SafeRelease(fl2);
        return;
    }
}
};