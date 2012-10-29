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
#include "FileSystem/LocalizationMacOS.h"
#include "FileSystem/LocalizationSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/Logger.h"
#include "Render/2D/GraphicsFont.h"

namespace DAVA
{
    void LocalizationMacOS::SelectLocalizationForPath(const String &directoryPath)
    {
        NSArray *ar = [NSLocale preferredLanguages];

        for (int i = 0; i < (int)[ar count]; i++) 
        {
            NSString * lang = [[[[ar objectAtIndex:i] componentsSeparatedByString:@"-"] objectAtIndex:0] lowercaseString];
            String lid = [lang UTF8String];
            if(lid == "pt")
            {
                lang = [[[[ar objectAtIndex:i] componentsSeparatedByString:@"-"] objectAtIndex:1] lowercaseString];
                lid = [lang UTF8String];
            }
            
            Logger::Info("LocalizationMacOS:: pref lang = %s", lid.c_str());
            File *fl = File::Create(directoryPath + "/" + lid.c_str() + ".yaml", File::OPEN|File::READ);
            if(fl)
            {
                Logger::Info("LocalizationMacOS:: selected lang = %s", lid.c_str());
                LocalizationSystem::Instance()->SetCurrentLocale(lid);
                SafeRelease(fl);
                return;
            }
            else
            {
                Logger::Info("LocalizationMacOS:: Localization file %s not found", lid.c_str());
            }
        }
        
        LocalizationSystem::Instance()->SetCurrentLocale("en");
        return;
        
//        NSString * localeString = [[NSLocale currentLocale] localeIdentifier];
//        NSString * lang = [[[localeString componentsSeparatedByString:@"_"] objectAtIndex:0] lowercaseString];
//        NSString * region = [[[localeString componentsSeparatedByString:@"_"] objectAtIndex:1] lowercaseString];
//        
//        String lid = [lang UTF8String];
//        String rid = [region UTF8String];
//
//        if(rid == "br" && lid == "pt")
//            lid = "br";
//        
//        Logger::Info("LocalizationMacOS:: lang = %s", lid.c_str());
//        
//        File *fl = File::Create(directoryPath + "/" + lid.c_str() + ".yaml", File::OPEN|File::READ);
//        if(fl == 0)
//        {
//            lid = "en";
//        }
//        
//        Logger::Info("LocalizationMacOS:: selected lang = %s", lid.c_str());
//        LocalizationSystem::Instance()->SetCurrentLocale(lid);
//        SafeRelease(fl);
    }
};