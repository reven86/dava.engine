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


#ifndef __JNI_HELPERS_H__
#define __JNI_HELPERS_H__

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__)
#include <jni.h>
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"
#include "FileSystem/Logger.h"
#include "Base/Function.h"
#include "Base/Bind.h"

namespace DAVA
{

namespace JNI
{

enum JObjectType
{
	VOID,
	STRING,
	BOOL,
	INT,
	OBJECT,
};

JavaVM *GetJVM();
JNIEnv *GetEnv();

String ParamToString(JObjectType type);

template <class Ret, class P1, class P2, class P3>
class JavaMethod;

class JavaClass
{
public:
    JavaClass(const String &className);
    ~JavaClass();

    template<class Ret, class P1, class P2, class P3>
    JavaMethod<Ret, P1, P2, P3> GetMethod(String name);

    jmethodID GetStaticMethod(JObjectType ret, String name, JObjectType p1 = VOID, JObjectType p2 = VOID, JObjectType p3 = VOID, JObjectType p4 = VOID, JObjectType p5 = VOID, JObjectType p6 = VOID);
    jmethodID GetMethod(JObjectType ret, String name, JObjectType p1 = VOID, JObjectType p2 = VOID, JObjectType p3 = VOID, JObjectType p4 = VOID, JObjectType p5 = VOID, JObjectType p6 = VOID);
    jmethodID GetMethod(bool isStatic, JObjectType ret, String name, Vector<JObjectType> &params);


    jmethodID GetStaticMethod1(String name, String specs);

    inline operator jclass() const;

private:
    JavaVM *jvm;
    jclass javaClass;
    String name;
};

inline JavaClass::operator jclass() const
{
	return javaClass;
}

template<class Ret, class P1, class P2, class P3>
JavaMethod<Ret, P1, P2, P3> JavaClass::GetMethod(String name)
{
	Vector<JObjectType> params;
	params.push_back(INT);
	params.push_back(STRING);
	params.push_back(STRING);

	return JavaMethod<Ret, P1, P2, P3>(GetMethod(true, VOID, name, params), javaClass, true);
}

template<class Ret, class P1, class P2, class P3>
class JavaMethod
{
public:
	JavaMethod(jmethodID mid, jclass c, bool isStatic);

	inline operator jmethodID() const;

	Ret operator ()(P1 p1, P2 p2, P3 p3);

private:
	bool isStatic;
	jclass javaClass;
	jmethodID javaMethod;
};

template<class Ret, class P1, class P2, class P3>
JavaMethod<Ret, P1, P2, P3>::JavaMethod(jmethodID mid, jclass c, bool isStatic)
{
	this->isStatic = isStatic;
	javaClass = c;
	javaMethod = mid;
}

template<class Ret, class P1, class P2, class P3>
inline JavaMethod<Ret, P1, P2, P3>::operator jmethodID() const
{
	return javaMethod;
}

template<class Ret, class P1, class P2, class P3>
Ret JavaMethod<Ret, P1, P2, P3>::operator ()(P1 p1, P2 p2, P3 p3)
{
	if (isStatic)
	{
		return static_cast<Ret>(GetEnv()->CallStaticVoidMethod(javaClass, javaMethod, p1, p2, p3));
	}
	else
	{
		return static_cast<Ret>(GetEnv()->CallVoidMethod(javaClass, javaMethod, p1, p2, p3));
	}
}

}
}

#endif

#endif
