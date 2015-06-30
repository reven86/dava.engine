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


#include "LocalizationSystemHelper.h"
#include "FileSystem/Logger.h"

using namespace DAVA;

const LocalizationSystemHelper::LocalizationSystemHelperData LocalizationSystemHelper::helperData[] =
{
    {"en", "English"},
    {"ru", "Russian"},
    {"de", "German"},
    {"es", "Spanish"},
    {"fr", "French"},
    {"it", "Italian"},
    {"cs", "Czech"},
    {"fi", "Finnish"},
    {"pl", "Polish"},
    {"pt", "Portuguese"},
    {"tr", "Turkish"},
    {"ja", "Japanese"},
    {"ko", "Korean"},
    {"zh-Hant", "Chinese(Traditional)"},
    {"zh-Hans", "Chinese(Simplified)"},
    {"ar", "Arabic"},
    {"th", "Thai"},
    {"vi", "Vietnamese"},
    //{"nl", "Dutch"},
    //{"sv", "Swedish"}
};

int LocalizationSystemHelper::GetSupportedLanguagesCount()
{
    return sizeof(helperData)/sizeof(*helperData);
}

String LocalizationSystemHelper::GetSupportedLanguageID(int index)
{
    if (ValidateLanguageIndex(index) == false)
    {
        return  helperData[0].languageID;
    }
    
    return helperData[index].languageID;
}

String LocalizationSystemHelper::GetSupportedLanguageDesc(int index)
{
    if (ValidateLanguageIndex(index) == false)
    {
        return helperData[0].languageDescription;
    }
    
    return helperData[index].languageDescription;
}

String LocalizationSystemHelper::GetLanguageDescByLanguageID(String languageID)
{
    for (int i = 0; i < GetSupportedLanguagesCount(); ++i)
    {
        if (languageID.compare(helperData[i].languageID) == 0) {
            return helperData[i].languageDescription;
        }
    }
    return helperData[0].languageDescription;
}

String LocalizationSystemHelper::GetLanguageIDByLanguageDesc(String languageDesc)
{
    for (int i = 0; i < GetSupportedLanguagesCount(); ++i)
    {
        if ( languageDesc.compare(helperData[i].languageDescription) == 0) {
            return helperData[i].languageID;
        }
    }
    return helperData[0].languageID;
}

bool LocalizationSystemHelper::ValidateLanguageIndex(int index)
{
    if (index < 0 || index >= GetSupportedLanguagesCount())
    {
        Logger::Error("Language index %i is out of bounds!", index);
        return false;
    }
    
    return true;
}