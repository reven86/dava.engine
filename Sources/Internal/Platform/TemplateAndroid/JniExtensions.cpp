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

JniExtension::JniExtension()
{
	CorePlatformAndroid *core = (CorePlatformAndroid *)Core::Instance();
	AndroidSystemDelegate* delegate = core->GetAndroidSystemDelegate();
	vm = delegate->GetVM();
}

JniExtension::~JniExtension()
{
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

	jmethodID mid = GetEnvironment()->GetStaticMethodID(javaClass, methodName, paramCode);

	if (!mid)
	{
		Logger::Error("get method id of %s.%s%s error ", GetJavaClassName(), methodName, paramCode);
	}

	return mid;
}

JNIEnv *JniExtension::GetEnvironment() const
{
	// right way to take JNIEnv
	// JNIEnv is valid only for the thread where it was gotten.
	// we shouldn't store JNIEnv.

	JNIEnv *env;
	if (JNI_EDETACHED == vm->GetEnv((void**)&env, JNI_VERSION_1_6))
	{
		Logger::Error("runtime_error(Thread is not attached to JNI)");
	}
	return env;
};

Rect JniExtension::V2P(const Rect& srcRect) const
{
	Vector2 offset = VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset();
	Rect rect = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(srcRect);

	rect += offset;
	return rect;
}

}//namespace DAVA
