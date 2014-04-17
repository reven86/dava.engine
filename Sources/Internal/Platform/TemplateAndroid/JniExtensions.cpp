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



#include "JniExtensions.h"
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"

namespace DAVA
{

JniExtension::JniExtension() :
	isAttached(false)
{
	CorePlatformAndroid *core = (CorePlatformAndroid *)Core::Instance();
	AndroidSystemDelegate* delegate = core->GetAndroidSystemDelegate();
	vm = delegate->GetVM();

	jint res = JNI_OK;

	res = vm->GetEnv((void **)&env,JNI_VERSION_1_6);

	if (env == NULL && !Thread::IsMainThread())
	{
		res = vm->AttachCurrentThread(&env, NULL);
		if (res == JNI_OK)
		{
			isAttached = true;
		}
		else
		{
			Logger::Error("Failed to AttachCurrentThread: res:%d", res);
		}
	}

	if (res != JNI_OK)
	{
		Logger::Error("Failed to get the environment using GetEnv()");
		env = NULL;
	}
}

JniExtension::~JniExtension()
{
	if (isAttached && !Thread::IsMainThread())
	{
		jint res = vm->DetachCurrentThread();
		if (res != JNI_OK)
			Logger::Error("Failed to DetachCurrentThread: res:%d", res);
	}
}

void JniExtension::SetJavaClass(JNIEnv* env, const char* className, jclass* gJavaClass, const char** gJavaClassName)
{
	*gJavaClass = (jclass) env->NewGlobalRef(env->FindClass(className));
	if (gJavaClassName)
		*gJavaClassName = className;
}

jmethodID JniExtension::GetMethodID(const char *methodName, const char *paramCode) const
{
	jclass javaClass = GetJavaClass();
	DVASSERT(javaClass && "Not initialized Java class");
	if (!javaClass)
		return 0;

	jmethodID mid = env->GetStaticMethodID(javaClass, methodName, paramCode);

	if (!mid)
	{
		Logger::Error("get method id of %s.%s error ", GetJavaClassName(), methodName);
	}

	return mid;
}

Rect JniExtension::V2P(const Rect& srcRect) const
{
	Vector2 offset = Core::Instance()->GetPhysicalDrawOffset();
	float32 v2p = Core::Instance()->GetVirtualToPhysicalFactor();
	Rect rect = srcRect;
	rect.x *= v2p;
	rect.y *= v2p;
	rect.dx *= v2p;
	rect.dy *= v2p;

	rect += offset;
	return rect;
}

}//namespace DAVA
