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


#include "Utils/Utils.h"
#include "Utils/StringFormat.h"
#include "FileSystem/YamlParser.h"

namespace DAVA
{

	
WideString WcharToWString(const wchar_t *s)
{
//	Logger::Info("[WcharToWString] s = %s", s);

	WideString temp;
	if(s)
	{

		wchar_t c = 0;
		int size = 0;
		do 
		{
			c = s[size];
			++size;
//			Logger::Info("[WcharToWString] c = %d, size = %d", c, size);
			
			if(c)
				temp.append(1, c);
		} while (c);
	}

	return temp; 
}

bool IsEqual(const WideString& s1, const WideString& s2)
{
	char16 *p1 = (char16 *)s1.c_str();
	char16 *p2 = (char16 *)s2.c_str();

	while(*p1 && *p2)
	{
		if(*p1 != *p2)
			return false;

		++p1;
		++p2;
	}

	return (*p1 == *p2);
}
    
void Split(const String & inputString, const String & delims, Vector<String> & tokens, bool skipDuplicated/* = false*/, bool addEmptyTokens/* = false*/)
{
    std::string::size_type pos, lastPos = 0;
    bool needAddToken = true;
    bool exit = false;
    String token = "";
    while(true)
    {
        needAddToken = false;
        pos = inputString.find_first_of(delims, lastPos);
        if(pos == std::string::npos)
        {
            pos = inputString.length();
            exit = true;
        }
        if(pos != lastPos || addEmptyTokens)
             needAddToken = true;
        
        token = String(inputString.data()+lastPos,pos-lastPos );
        if (skipDuplicated && needAddToken)
		{
			for (uint32 i = 0; i < tokens.size(); ++i)
			{
				if (token.compare(tokens[i]) == 0)
				{
					needAddToken = false;
					break;
				}
			}
		}
		if (needAddToken)
			tokens.push_back(token);
        if (exit)
            break;
        lastPos = pos + 1;
    }
}

void Merge(const Vector<String> & tokens, const char delim, String & outString)
{
    outString.clear();

    uint32 tokensSize = tokens.size();
    if (tokensSize > 0)
    {
        outString.append(tokens[0]);
        if (tokensSize > 1)
        {
            for (uint32 i = 1; i < tokensSize; ++i)
            {
                outString += delim;
                outString += tokens[i];
            }
        }
    }
}

/* Set a generic reader. */
int read_handler(void *ext, unsigned char *buffer, size_t size, size_t *length)
{
	YamlParser::YamlDataHolder * holder = (YamlParser::YamlDataHolder*)ext;
	int32 sizeToWrite = Min((uint32)size, holder->fileSize-holder->dataOffset);

    memcpy(buffer, holder->data+holder->dataOffset, sizeToWrite);
	
    *length = sizeToWrite;

	holder->dataOffset += sizeToWrite;

	return 1;
}

int32 CompareCaseInsensitive(const String &str1, const String &str2)
{
    String newStr1 = "";
    newStr1.resize(str1.length());
    std::transform(str1.begin(), str1.end(), newStr1.begin(), ::tolower);
    
    String newStr2 = "";
    newStr2.resize(str2.length());
    std::transform(str2.begin(), str2.end(), newStr2.begin(), ::tolower);
    
    if(newStr1 == newStr2)
    {
        return 0;   
    }
    else if(newStr1 < newStr2)
    {
        return -1;
    }
    
    return 1;
}

#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
void DisableSleepTimer()
{
}
 
void EnableSleepTimer()
{
}
    
#endif

}; // end of namespace DAVA
