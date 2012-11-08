/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include "FileSystem/LocalizationIPhone.h"
#include "FileSystem/LocalizationSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/Logger.h"
#include "Render/2D/GraphicsFont.h"
#include "DAVAEngine.h"

namespace DAVA
{
void LocalizationIPhone::SelecePreferedLocalizationForPath(const String &directoryPath)
{
    NSString * lang = [[NSUserDefaults standardUserDefaults] stringForKey:@"KD_LOCALE"];
    
    unsigned int scv = (unsigned int)[[::UIScreen mainScreen] scale];
    int padOffset = 0;
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad && scv == 2)
        padOffset = -2;

    
    if(lang != nil && ![lang isEqualToString:@""])
    {
        if([lang isEqualToString:@"en"] || [lang isEqualToString:@"ru"])
        {
            LocalizationSystem::Instance()->graphicsFontDrawYoffset1 = 5;
            LocalizationSystem::Instance()->graphicsFontDrawYoffset2 = 4 - padOffset;
        }
        else if ([lang isEqualToString:@"ja"] || [lang isEqualToString:@"zh"] || [lang isEqualToString:@"ko"])
        {
            LocalizationSystem::Instance()->graphicsFontDrawYoffset1 = 0;
            LocalizationSystem::Instance()->graphicsFontDrawYoffset2 = 0;
        }
        else
        {
            LocalizationSystem::Instance()->graphicsFontDrawYoffset1 = 6;
            LocalizationSystem::Instance()->graphicsFontDrawYoffset2 = 3 - padOffset;
        }
        
        String lid = [lang UTF8String];
		File *fl = File::Create(directoryPath + "/" + lid.c_str() + ".yaml", File::OPEN|File::READ);
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
        NSArray *languages = [NSLocale preferredLanguages];
        for (int i = 0; i < (int)[languages count]; i++) 
        {
            NSArray * ar = [[languages objectAtIndex:i] componentsSeparatedByString:@"-"];
            NSString * lang = [[ar objectAtIndex:0] lowercaseString];
            String lid = [lang UTF8String];
            if(lid == "pt")
            {
                if([ar count] == 1)
                    lid = "br";
                else
                {
                    lang = [[ar objectAtIndex:1] lowercaseString];
                    lid = [lang UTF8String];
                }
            }

            Logger::Info("LocalizationIPhone:: pref lang = %s", lid.c_str());
            File *fl = File::Create(directoryPath + "/" + lid.c_str() + ".yaml", File::OPEN|File::READ);
            if(fl)
            {
                if(lid == "en" || lid == "ru")
                {
                    LocalizationSystem::Instance()->graphicsFontDrawYoffset1 = 5;
                    LocalizationSystem::Instance()->graphicsFontDrawYoffset2 = 4 - padOffset;
                }
                else if (lid == "ja" || lid == "zh" || lid == "ko")
                {
                    LocalizationSystem::Instance()->graphicsFontDrawYoffset1 = 0;
                    LocalizationSystem::Instance()->graphicsFontDrawYoffset2 = 0;
                }
                else
                {
                    LocalizationSystem::Instance()->graphicsFontDrawYoffset1 = 6;
                    LocalizationSystem::Instance()->graphicsFontDrawYoffset2 = 3 - padOffset;
                }
                
                Logger::Info("LocalizationIPhone:: selected lang = %s", lid.c_str());
                LocalizationSystem::Instance()->SetCurrentLocale(lid);
                SafeRelease(fl);
                return;
            }
        }
    }
}
};