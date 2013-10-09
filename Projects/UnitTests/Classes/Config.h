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


#ifndef __CONFIG_H__
#define __CONFIG_H__

//#define DATABASE_IP               String("10.128.128.131")
#define DATABASE_IP               String("by2-buildmachine.wargaming.net")
//#define DATABASE_IP                 String("127.0.0.1")
#define DATAPASE_PORT               27017

#define DATABASE_NAME               String("UnitTests")
#define DATABASE_COLLECTION         String("UnitTestsResult")

#if defined (__DAVAENGINE_MACOS__)
    #define PLATFORM_NAME           String("MacOS")
#elif defined (__DAVAENGINE_IPHONE__)
    #define PLATFORM_NAME           String("iPhone")
#elif defined (__DAVAENGINE_WIN32__)
    #define PLATFORM_NAME           String("Win32")
#elif defined (__DAVAENGINE_ANDROID__)
    #define PLATFORM_NAME           String("Android")
#else
    #define PLATFORM_NAME           String("Unknown")
#endif //PLATFORMS    


#endif // __CONFIG_H__
