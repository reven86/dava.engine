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


#ifndef __DAVAENGINE_UTILS_H__
#define __DAVAENGINE_UTILS_H__

/**
	\defgroup utils Utilities
 */

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "Render/RenderBase.h"
#include <sstream>

#ifdef __DAVAENGINE_WIN_UAP__
#   include <ppltasks.h>
#endif

namespace DAVA 
{
int read_handler(void *ext, unsigned char *buffer, size_t size, size_t *length);

WideString WcharToWString(const wchar_t *s);
bool IsEqual(const WideString& s1, const WideString& s2);


bool IsDrawThread();

inline WideString StringToWString(const String& s);
inline String WStringToString(const WideString& s);

WideString GetDeviceName();
	
void DisableSleepTimer();
void EnableSleepTimer();
	
void Split(const String & inputString, const String & delims, Vector<String> & tokens, bool skipDuplicated = false, bool addEmptyTokens = false);
void Merge(const Vector<String> & tokens, const char delim, String & outString);
void ReplaceBundleName(const String &newBundlePath);
    
template<class T>
T ParseStringTo(const String & str); 

template<class T>
bool ParseFromString(const String & str, T& res);

template<class T>
void Swap(T & v1, T & v2);

/**
 \brief Function to compare strings case-insensitive
 \param[in] ext1 - first string 
 \param[in] ext2 - second string 
 \param[out] result of comparision 
 */
int32 CompareCaseInsensitive(const String &str1, const String &str2);

//implementation

inline WideString StringToWString(const String& s)
{
	WideString temp(s.length(),L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp; 
}

inline void StringReplace(String & repString,const String & needle, const String & s)
{
	String::size_type lastpos = 0, thispos;
	while ((thispos = repString.find(needle, lastpos)) != String::npos)
	{
		repString.replace(thispos, needle.length(), s);
		lastpos = thispos + s.length();
	}
}

inline String WStringToString(const WideString& s)
{
	size_t len = s.length();
	String temp(len, ' ');
	//std::copy(s.begin(), s.end(), temp.begin());
	for (size_t i = 0; i < len; ++i)
		temp[i] = (char)s[i];
	return temp; 
}

#if defined(__DAVAENGINE_WIN_UAP__)
inline  Platform::String^ StringToRTString(const String & s)
{
    return ref new Platform::String(StringToWString(s).c_str());
}

inline String RTStringToString(Platform::String^ s)
{
    return WStringToString(s->Data());
}
#endif
    
template<class T>
bool FindAndRemoveExchangingWithLast(Vector<T> & array, const T & object)
{
    uint32 size = (uint32)array.size();
    for (uint32 k = 0; k < size; ++k)
        if (array[k] == object)
        {
            array[k] = array[size - 1];
            array.pop_back();
            return true;
        }
    
    return false;
}
    
    
template<class T>
void RemoveExchangingWithLast(Vector<T> & array, size_t index)
{
    array[index] = array[array.size() - 1];
    array.pop_back();
}

template<class T>
T ParseStringTo(const String & str)
{
    T result;
    std::stringstream stream(str);
    stream >> result;
    return result;
}

template<class T>
bool ParseFromString(const String & str, T& result)
{
    std::stringstream stream (str);
    stream >> result;
    return (stream.eof() == true && stream.fail() == false);
}

template<class T>
void Swap(T & v1, T & v2)
{
    T temp = v1;
    v1 = v2;
    v2 = temp;
}

template <class T, std::size_t size>
class CircularArray
{
public:
    T& Next()
    {
        T& ret = elements[currentIndex];

        if ((++currentIndex) == elements.size())
            currentIndex = 0;

        return ret;
    }

    std::array<T, size> elements;

protected:
    std::size_t currentIndex = 0;
};

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	
uint64 EglGetCurrentContext();
	
#endif

// Open the URL in external browser.
void OpenURL(const String& url);

String GenerateGUID();

#ifdef __DAVAENGINE_WIN_UAP__
template <typename T>
T WaitAsync(Windows::Foundation::IAsyncOperation<T>^ async_operation)
{
    return concurrency::create_task(async_operation).get();
}
#endif

};

#endif // __DAVAENGINE_UTILS_H__

