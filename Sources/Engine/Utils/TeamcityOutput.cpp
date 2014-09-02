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



#include "Utils/TeamcityOutput.h"
#include "Utils/Utils.h"

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)


namespace DAVA
{
    
void TeamcityOutput::Output(Logger::eLogLevel ll, const char8 *text) const
{
    String outStr = NormalizeString(text);
	String status = "NORMAL";
    switch (ll)
    {
        case Logger::LEVEL_ERROR:
			status = "ERROR";
            break;

        case Logger::LEVEL_WARNING:
			status = "WARNING";
            break;
            
        default:
            break;
    }

    String output = "##teamcity[message text=\'" + outStr + "\' errorDetails=\'\' status=\'" + status + "\']\n";
    PlatformOutput(output);
}

void TeamcityOutput::Output(Logger::eLogLevel ll, const char16 *text) const
{
    WideString wstr = text;
    Output(ll, WStringToString(wstr).c_str());
}

String TeamcityOutput::NormalizeString(const char8 *text) const
{
    String str = text;
    
    StringReplace(str, "|", "||");

    StringReplace(str, "'", "|'");
    StringReplace(str, "\n", "|n");
    StringReplace(str, "\r", "|r");

//    StringReplace(str, "\u0085", "|x");
//     StringReplace(str, "\u2028", "|l");
//     StringReplace(str, "\u2029", "|p");

    StringReplace(str, "[", "|[");
    StringReplace(str, "]", "|]");
    
    return str;
}
	
#if defined (__DAVAENGINE_WIN32__)
void TeamcityOutput::PlatformOutput(const String &text) const
{
    OutputDebugStringA(text.c_str());
}
#endif //#if defined (__DAVAENGINE_WIN32__)
    
}; // end of namespace DAVA

#endif //#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)

