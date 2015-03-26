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



#include "AndroidLayer.h"
#include "Platform/TemplateAndroid/WebViewControlAndroid.h"
#include "Platform/TemplateAndroid/JniHelpers.h"
#include "Render/Image/ImageConvert.h"

extern "C"
{
	int Java_com_dava_framework_JNIWebView_OnUrlChange(JNIEnv* env, jobject classthis, int id, jstring jUrl)
	{
		int res = 0;
		char url[1024];
		DAVA::JNI::CreateStringFromJni(env, jUrl, url);
		res = DAVA::JniWebView::URLChanged(id, url);
		return res;
	}

	void Java_com_dava_framework_JNIWebView_OnPageLoaded(JNIEnv* env, jobject classthis, int id, jintArray pixels, int width, int height)
	{
		if (nullptr == pixels)
		{
			DAVA::JniWebView::PageLoaded(id, 0, 0, 0);
		} else
		{
			jboolean isCopy = JNI_FALSE;
			jint* rawData = env->GetIntArrayElements(pixels, &isCopy);

			DVASSERT(rawData);
			DVASSERT(width);
			DVASSERT(height);
			DVASSERT(env->GetArrayLength(pixels) == width * height); // ARGB

            uint32 pitch = width * PixelFormatDescriptor::GetPixelFormatSizeInBytes(DAVA::FORMAT_RGBA8888);
            DAVA::ImageConvert::SwapChannels(DAVA::FORMAT_RGBA8888, rawData, width, height, pitch);

			DAVA::JniWebView::PageLoaded(id, rawData, width, height);

			env->ReleaseIntArrayElements(pixels, rawData, 0);
		}

	}

	void Java_com_dava_framework_JNIWebView_OnExecuteJScript(JNIEnv* env, jobject classthis, int id, jstring jResult)
	{
		// string with result can be large with JSON inside

	    // Returns the length in bytes of the modified UTF-8
	    // representation of a string.
	    // http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/functions.html
	    size_t size = env->GetStringUTFLength(jResult);
	    const char* utf8Data = env->GetStringUTFChars(jResult, NULL);

	    DAVA::String str(utf8Data, size);

		DAVA::JniWebView::OnExecuteJScript(id, str);

		// http://stackoverflow.com/questions/5859673/should-you-call-releasestringutfchars-if-getstringutfchars-returned-a-copy
		env->ReleaseStringUTFChars(jResult, utf8Data);
	}

};
