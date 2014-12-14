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

#include "JniHelpers.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"

namespace DAVA
{


namespace JNI
{

JavaVM *GetJVM()
{
    CorePlatformAndroid *core = static_cast<DAVA::CorePlatformAndroid *>(Core::Instance());
    AndroidSystemDelegate* delegate = core->GetAndroidSystemDelegate();
    return delegate->GetVM();
}

JNIEnv *GetEnv()
{
    JNIEnv *env;
    JavaVM *vm = GetJVM();

    if (NULL == vm || JNI_EDETACHED == vm->GetEnv((void**)&env, JNI_VERSION_1_6))
    {
        Logger::Error("runtime_error(Thread is not attached to JNI)");
    }
    DVASSERT(NULL != env);
    return env;
}

String ParamToString(JObjectType type)
{
	switch (type)
	{
	case VOID: return "V";
	case INT: return "I";
	case STRING: return "Ljava/lang/String;";
	case OBJECT: return "[Ljava/lang/Object;";
	default: return "";
	}
}


JavaClass::JavaClass(const String &className)
{
    jvm = GetJVM();
    JNIEnv *env = GetEnv();
    javaClass = env->FindClass(className.c_str());
    if (NULL != javaClass)
    {
    	javaClass = static_cast<jclass>(env->NewGlobalRef(javaClass));
    }

    name = className;

    DVASSERT(NULL != javaClass);
}

JavaClass::~JavaClass()
{
    GetEnv()->DeleteGlobalRef(javaClass);
}


jmethodID JavaClass::GetStaticMethod(JObjectType ret, String name, JObjectType p1, JObjectType p2, JObjectType p3, JObjectType p4, JObjectType p5, JObjectType p6)
{
	Vector<JObjectType> params;

	do
	{
		if (VOID == p1)
			break;
		params.push_back(p1);
		if (VOID == p2)
			break;
		params.push_back(p2);
		if (VOID == p3)
			break;
		params.push_back(p3);
		if (VOID == p4)
			break;
		params.push_back(p4);
		if (VOID == p5)
			break;
		params.push_back(p5);
		if (VOID == p6)
			break;
		params.push_back(p6);
	}while(false);

	return GetMethod(true, ret, name, params);
}
jmethodID JavaClass::GetMethod(JObjectType ret, String name, JObjectType p1, JObjectType p2, JObjectType p3, JObjectType p4, JObjectType p5, JObjectType p6)
{
	Vector<JObjectType> params;

	do
	{
		if (VOID == p1)
			break;
		params.push_back(p1);
		if (VOID == p2)
			break;
		params.push_back(p2);
		if (VOID == p3)
			break;
		params.push_back(p3);
		if (VOID == p4)
			break;
		params.push_back(p4);
		if (VOID == p5)
			break;
		params.push_back(p5);
		if (VOID == p6)
			break;
		params.push_back(p6);
	}while(false);

	return GetMethod(false, ret, name, params);
}

jmethodID JavaClass::GetMethod(bool isStatic, JObjectType ret, String name, Vector<JObjectType> &params)
{
	String spec = "(";
	for (uint32 i = 0; i < params.size(); ++i)
	{
		JObjectType type = params[i];
		spec += ParamToString(type);
	}

	spec += ")";
	spec += ParamToString(ret);

	Logger::Debug("Spec string %s", spec.c_str());

	jmethodID method = NULL;

	if (isStatic)
	{
		method = GetEnv()->GetStaticMethodID(javaClass, name.c_str(), spec.c_str());
	}
	else
	{
		method = GetEnv()->GetMethodID(javaClass, name.c_str(), spec.c_str());
	}

	if (JNI_TRUE == GetEnv()->ExceptionCheck())
	{
		Logger::Error("[JNI] Cannot take method ID for %s::%s", this->name.c_str(), name.c_str());
	}

	DVASSERT(NULL != method);

	return method;
}

jmethodID JavaClass::GetStaticMethod1(String name, String specs)
{
	return GetEnv()->GetStaticMethodID(javaClass, name.c_str(), specs.c_str());
}
}


}
#endif
