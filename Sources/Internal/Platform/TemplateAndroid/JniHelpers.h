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
#include "Debug/DVAssert.h"
#include "Base/Function.h"
#include "Base/Bind.h"
#include "Math/Rect.h"


// placed into the Global namespace and not in JNI to use it like a jstring
class jstringArray
{
public:
    jstringArray(const jobjectArray &arr);

    inline operator jobjectArray() const;

private:
    jobjectArray obj;
};

inline jstringArray::operator jobjectArray() const
{
    return obj;
}

namespace DAVA
{

namespace JNI
{

/*
	Supported return arguments types.
	If you want to use some unsupported for now type, you need to:
	1. Add new instance of template<> struct TypeMetrics<your New Used type> for your new type.

	If unsupoorted type is Ret type:
	2. Add new template<> class JniCall<you class> and implement all methods as in existed classes
 */

JavaVM *GetJVM();
JNIEnv *GetEnv();

void AttachCurrentThreadToJVM();
void DetachCurrentThreadFromJVM();

Rect V2P(const Rect& rect);

bool CreateStringFromJni(JNIEnv *env, jstring jniString, char *generalString);
inline bool CreateStringFromJni(jstring jniString, char *generalString)
{
	return CreateStringFromJni(GetEnv(), jniString, generalString);
}

void CreateStringFromJni(JNIEnv *env, jstring jniString, String& string);
inline void CreateStringFromJni(jstring jniString, String& string)
{
	CreateStringFromJni(GetEnv(), jniString, string);
}

void CreateWStringFromJni(JNIEnv *env, jstring jniString, WideString& string);
inline void CreateWStringFromJni(jstring jniString, WideString& string)
{
	CreateWStringFromJni(GetEnv(), jniString, string);
}

jstring CreateJString(JNIEnv *env, const DAVA::WideString& string);
inline jstring CreateJString(const DAVA::WideString& string)
{
	return CreateJString(GetEnv(), string);
}


#define DeclareTypeString(str)\
	operator const char *() const {return value.c_str();}\
	operator String() const {return value;}\
	const String value = str;\
    const String rvalue = str;

#define DeclareTypeStringWithRet(str, rstr)\
    operator const char *() const {return value.c_str();}\
    operator String() const {return value;}\
    const String value = str;\
    const String rvalue = rstr;

template<class T>
struct TypeMetrics
{ };

template<> struct TypeMetrics<void>
{
	DeclareTypeString("V");
};

template<> struct TypeMetrics<jint>
{
	DeclareTypeString("I");
};

template<> struct TypeMetrics<jintArray>
{
	DeclareTypeString("[I");
};

template<> struct TypeMetrics<jfloat>
{
	DeclareTypeString("F");
};

template<> struct TypeMetrics<jfloatArray>
{
	DeclareTypeString("[F");
};

template<> struct TypeMetrics<jdouble>
{
    DeclareTypeString("D");
};

template<> struct TypeMetrics<jdoubleArray>
{
    DeclareTypeString("[D");
};

template<> struct TypeMetrics<jlong>
{
	DeclareTypeString("J");
};

template<> struct TypeMetrics<jlongArray>
{
	DeclareTypeString("[J");
};

template<> struct TypeMetrics<jstring>
{
	DeclareTypeString("Ljava/lang/String;");
};

template<> struct TypeMetrics<jstringArray>
{
    DeclareTypeStringWithRet("[Ljava/lang/String;", "[Ljava/lang/Object;");
};

template<> struct TypeMetrics<jobject>
{
	DeclareTypeString("Ljava/lang/Object;");
};

template<> struct TypeMetrics<jobjectArray>
{
	DeclareTypeString("[Ljava/lang/Object;");
};

template<> struct TypeMetrics<jboolean>
{
	DeclareTypeString("Z");
};

template<> struct TypeMetrics<jbooleanArray>
{
	DeclareTypeString("[Z");
};

class SignatureString
{
public:
	template<class T>
	inline static const String &FromTypes();

	template<class Ret, class P1>
	inline static const String &FromTypes();

	template<class Ret, class P1, class P2>
	inline static const String &FromTypes();

	template<class Ret, class P1, class P2, class P3>
	inline static const String &FromTypes();

	template<class Ret, class P1, class P2, class P3, class P4>
	inline static const String &FromTypes();

	template<class Ret, class P1, class P2, class P3, class P4, class P5>
	inline static const String &FromTypes();

	template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
	inline static const String &FromTypes();
};

template<class Ret>
const String &SignatureString::FromTypes()
{
	static String ret = String("()")
            + TypeMetrics<Ret>().rvalue;
	return ret;
}

template<class Ret, class P1>
const String &SignatureString::FromTypes()
{
    static String ret = String("(")
            + TypeMetrics<P1>().value
            + String(")")
            + TypeMetrics<Ret>().rvalue;
    return ret;
}

template<class Ret, class P1, class P2>
const String &SignatureString::FromTypes()
{
    static String ret = String("(")
			+ TypeMetrics<P1>().value
			+ TypeMetrics<P2>().value
			+ String(")")
			+ TypeMetrics<Ret>().rvalue;
    return ret;
}

template<class Ret, class P1, class P2, class P3>
const String &SignatureString::FromTypes()
{
    static String ret = String("(")
			+ TypeMetrics<P1>().value
			+ TypeMetrics<P2>().value
			+ TypeMetrics<P3>().value
			+ String(")")
			+ TypeMetrics<Ret>().rvalue;
    return ret;
}
template<class Ret, class P1, class P2, class P3, class P4>
const String &SignatureString::FromTypes()
{
    static String ret = String("(")
			+ TypeMetrics<P1>().value
			+ TypeMetrics<P2>().value
			+ TypeMetrics<P3>().value
			+ TypeMetrics<P4>().value
			+ String(")")
			+ TypeMetrics<Ret>().rvalue;
    return ret;
}

template<class Ret, class P1, class P2, class P3, class P4, class P5>
const String &SignatureString::FromTypes()
{
    static String ret = String("(")
			+ TypeMetrics<P1>().value
			+ TypeMetrics<P2>().value
			+ TypeMetrics<P3>().value
			+ TypeMetrics<P4>().value
			+ TypeMetrics<P5>().value
			+ String(")")
			+ TypeMetrics<Ret>().rvalue;
    return ret;
}

template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
const String &SignatureString::FromTypes()
{
    static String ret = String("(")
			+ TypeMetrics<P1>().value
			+ TypeMetrics<P2>().value
			+ TypeMetrics<P3>().value
			+ TypeMetrics<P4>().value
			+ TypeMetrics<P5>().value
			+ TypeMetrics<P6>().value
			+ String(")")
			+ TypeMetrics<Ret>().rvalue;
    return ret;
}

/*
	JAVA Class caller implementations!
*/

template<class T>
class MethodCaller
{

};

// all possible function calls in one switch
// we have warnings for boolean types - it is because Ret type could not be converted to jboolean and jbooleanarray,
// but there is returns at each string and when Ret=jboolean - conversion is ok.
// Later we can split this code to separate callers...

template <class Ret>
struct JniCall
{
};

template<> struct JniCall<void>
{
	inline static void Call(jobject javaObject, jmethodID javaMethod)
	{
		GetEnv()->CallVoidMethod(javaObject, javaMethod);
	}

	inline static void CallStatic(jclass javaClass, jmethodID javaMethod)
	{
		GetEnv()->CallStaticVoidMethod(javaClass, javaMethod);
	}

	template<class P1>
	inline static void Call(jobject javaObject, jmethodID javaMethod, P1 p1)
	{
		GetEnv()->CallVoidMethod(javaObject, javaMethod, p1);
	}

	template<class P1>
	inline static void CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
	{
		GetEnv()->CallStaticVoidMethod(javaClass, javaMethod, p1);
	}

	template<class P1, class P2>
	inline static void Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
	{
		GetEnv()->CallVoidMethod(javaObject, javaMethod, p1, p2);
	}

	template<class P1, class P2>
	inline static void CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
	{
		GetEnv()->CallStaticVoidMethod(javaClass, javaMethod, p1, p2);
	}

	template<class P1, class P2, class P3>
	inline static void Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		GetEnv()->CallVoidMethod(javaObject, javaMethod, p1, p2, p3);
	}

	template<class P1, class P2, class P3>
	inline static void CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		GetEnv()->CallStaticVoidMethod(javaClass, javaMethod, p1, p2, p3);
	}

	template<class P1, class P2, class P3, class P4>
	inline static void Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		GetEnv()->CallVoidMethod(javaObject, javaMethod, p1, p2, p3, p4);
	}

	template<class P1, class P2, class P3, class P4>
	inline static void CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		GetEnv()->CallStaticVoidMethod(javaClass, javaMethod, p1, p2, p3, p4);
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static void Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		GetEnv()->CallVoidMethod(javaObject, javaMethod, p1, p2, p3, p4, p5);
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static void CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		GetEnv()->CallStaticVoidMethod(javaClass, javaMethod, p1, p2, p3, p4, p5);
	}
};

template<> struct JniCall<jint>
{

    inline static jint Call(jobject javaObject, jmethodID javaMethod)
    {
        return (jint)(GetEnv()->CallIntMethod(javaObject, javaMethod));
    }

    inline static jint CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        return (jint)(GetEnv()->CallStaticIntMethod(javaClass, javaMethod));
    }
    template<class P1>
    inline static jint Call(jobject javaObject, jmethodID javaMethod, P1 p1)
    {
        return (jint)(GetEnv()->CallIntMethod(javaObject, javaMethod, p1));
    }

    template<class P1>
    inline static jint CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
    {
        return (jint)(GetEnv()->CallStaticIntMethod(javaClass, javaMethod, p1));
    }

    template<class P1, class P2>
    inline static jint Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
    {
        return (jint)(GetEnv()->CallIntMethod(javaObject, javaMethod, p1, p2));
    }

    template<class P1, class P2>
    inline static jint CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
    {
        return (jint)(GetEnv()->CallStaticIntMethod(javaClass, javaMethod, p1, p2));
    }

    template<class P1, class P2, class P3>
    inline static jint Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
    {
        return (jint)(GetEnv()->CallIntMethod(javaObject, javaMethod, p1, p2, p3));
    }

    template<class P1, class P2, class P3>
    inline static jint CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
    {
        return (jint)(GetEnv()->CallStaticIntMethod(javaClass, javaMethod, p1, p2, p3));
    }

    template<class P1, class P2, class P3, class P4>
    inline static jint Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
    {
        return (jint)(GetEnv()->CallIntMethod(javaObject, javaMethod, p1, p2, p3, p4));
    }

    template<class P1, class P2, class P3, class P4>
    inline static jint CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
    {
        return (jint)(GetEnv()->CallStaticIntMethod(javaClass, javaMethod, p1, p2, p3, p4));
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jint Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        return (jint)(GetEnv()->CallIntMethod(javaObject, javaMethod, p1, p2, p3, p4, p5));
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jint CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        return (jint)(GetEnv()->CallStaticIntMethod(javaClass, javaMethod, p1, p2, p3, p4, p5));
    }
};

template<> struct JniCall<jintArray>
{
    inline static jintArray Call(jobject javaObject, jmethodID javaMethod)
    {
        return (jintArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod));
    }

    inline static jintArray CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        return (jintArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod));
    }
    template<class P1>
    inline static jintArray Call(jobject javaObject, jmethodID javaMethod, P1 p1)
    {
        return (jintArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1));
    }

    template<class P1>
    inline static jintArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
    {
        return (jintArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1));
    }

    template<class P1, class P2>
    inline static jintArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
    {
        return (jintArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2));
    }

    template<class P1, class P2>
    inline static jintArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
    {
        return (jintArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2));
    }

    template<class P1, class P2, class P3>
    inline static jintArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
    {
        return (jintArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3));
    }

    template<class P1, class P2, class P3>
    inline static jintArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
    {
        return (jintArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3));
    }

    template<class P1, class P2, class P3, class P4>
    inline static jintArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
    {
        return (jintArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4));
    }

    template<class P1, class P2, class P3, class P4>
    inline static jintArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
    {
        return (jintArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4));
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jintArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        return (jintArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4, p5));
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jintArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        return (jintArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4, p5));
    }
};


template<> struct JniCall<jfloat>
{
	inline static jfloat Call(jobject javaObject, jmethodID javaMethod)
	{
		return (jfloat)(GetEnv()->CallFloatMethod(javaObject, javaMethod));
	}

	inline static jfloat CallStatic(jclass javaClass, jmethodID javaMethod)
	{
		return (jfloat)(GetEnv()->CallStaticFloatMethod(javaClass, javaMethod));
	}
	template<class P1>
	inline static jfloat Call(jobject javaObject, jmethodID javaMethod, P1 p1)
	{
		return (jfloat)(GetEnv()->CallFloatMethod(javaObject, javaMethod, p1));
	}

	template<class P1>
	inline static jfloat CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
	{
		return (jfloat)(GetEnv()->CallStaticFloatMethod(javaClass, javaMethod, p1));
	}

	template<class P1, class P2>
	inline static jfloat Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jfloat)(GetEnv()->CallFloatMethod(javaObject, javaMethod, p1, p2));
	}

	template<class P1, class P2>
	inline static jfloat CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jfloat)(GetEnv()->CallStaticFloatMethod(javaClass, javaMethod, p1, p2));
	}

	template<class P1, class P2, class P3>
	inline static jfloat Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jfloat)(GetEnv()->CallFloatMethod(javaObject, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3>
	inline static jfloat CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jfloat)(GetEnv()->CallStaticFloatMethod(javaClass, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jfloat Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jfloat)(GetEnv()->CallFloatMethod(javaObject, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jfloat CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jfloat)(GetEnv()->CallStaticFloatMethod(javaClass, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jfloat Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jfloat)(GetEnv()->CallFloatMethod(javaObject, javaMethod, p1, p2, p3, p4, p5));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jfloat CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jfloat)(GetEnv()->CallStaticFloatMethod(javaClass, javaMethod, p1, p2, p3, p4, p5));
	}
};

template<> struct JniCall<jfloatArray>
{
	inline static jfloatArray Call(jobject javaObject, jmethodID javaMethod)
	{
		return (jfloatArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod));
	}

	inline static jfloatArray CallStatic(jclass javaClass, jmethodID javaMethod)
	{
		return (jfloatArray)(GetEnv()->CallObjectMethod(javaClass, javaMethod));
	}
	template<class P1>
	inline static jfloatArray Call(jobject javaObject, jmethodID javaMethod, P1 p1)
	{
		return (jfloatArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1));
	}

	template<class P1>
	inline static jfloatArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
	{
		return (jfloatArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1));
	}

	template<class P1, class P2>
	inline static jfloatArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jfloatArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2));
	}

	template<class P1, class P2>
	inline static jfloatArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jfloatArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2));
	}

	template<class P1, class P2, class P3>
	inline static jfloatArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jfloatArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3>
	inline static jfloatArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jfloatArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jfloatArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jfloatArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jfloatArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jfloatArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jfloatArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jfloatArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4, p5));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jfloatArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jfloatArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4, p5));
	}
};

template<> struct JniCall<jdouble>
{
	inline static jdouble Call(jobject javaObject, jmethodID javaMethod)
	{
		return (jdouble)(GetEnv()->CallDoubleMethod(javaObject, javaMethod));
	}

	inline static jdouble CallStatic(jclass javaClass, jmethodID javaMethod)
	{
		return (jdouble)(GetEnv()->CallStaticDoubleMethod(javaClass, javaMethod));
	}
	template<class P1>
	inline static jdouble Call(jobject javaObject, jmethodID javaMethod, P1 p1)
	{
		return (jdouble)(GetEnv()->CallDoubleMethod(javaObject, javaMethod, p1));
	}

	template<class P1>
	inline static jdouble CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
	{
		return (jdouble)(GetEnv()->CallStaticDoubleMethod(javaClass, javaMethod, p1));
	}

	template<class P1, class P2>
	inline static jdouble Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jdouble)(GetEnv()->CallDoubleMethod(javaObject, javaMethod, p1, p2));
	}

	template<class P1, class P2>
	inline static jdouble CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jdouble)(GetEnv()->CallStaticDoubleMethod(javaClass, javaMethod, p1, p2));
	}

	template<class P1, class P2, class P3>
	inline static jdouble Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jdouble)(GetEnv()->CallDoubleMethod(javaObject, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3>
	inline static jdouble CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jdouble)(GetEnv()->CallStaticDoubleMethod(javaClass, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jdouble Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jdouble)(GetEnv()->CallDoubleMethod(javaObject, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jdouble CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jdouble)(GetEnv()->CallStaticDoubleMethod(javaClass, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jdouble Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jdouble)(GetEnv()->CallDoubleMethod(javaObject, javaMethod, p1, p2, p3, p4, p5));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jdouble CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jdouble)(GetEnv()->CallStaticDoubleMethod(javaClass, javaMethod, p1, p2, p3, p4, p5));
	}
};

template<> struct JniCall<jdoubleArray>
{
	inline static jdoubleArray Call(jobject javaObject, jmethodID javaMethod)
	{
		return (jdoubleArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod));
	}

	inline static jdoubleArray CallStatic(jclass javaClass, jmethodID javaMethod)
	{
		return (jdoubleArray)(GetEnv()->CallObjectMethod(javaClass, javaMethod));
	}
	template<class P1>
	inline static jdoubleArray Call(jobject javaObject, jmethodID javaMethod, P1 p1)
	{
		return (jdoubleArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1));
	}

	template<class P1>
	inline static jdoubleArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
	{
		return (jdoubleArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1));
	}

	template<class P1, class P2>
	inline static jdoubleArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jdoubleArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2));
	}

	template<class P1, class P2>
	inline static jdoubleArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jdoubleArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2));
	}

	template<class P1, class P2, class P3>
	inline static jdoubleArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jdoubleArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3>
	inline static jdoubleArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jdoubleArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jdoubleArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jdoubleArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jdoubleArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jdoubleArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jdoubleArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jdoubleArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4, p5));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jdoubleArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jdoubleArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4, p5));
	}
};

template<> struct JniCall<jlong>
{
	inline static jlong Call(jobject javaObject, jmethodID javaMethod)
	{
		return (jlong)(GetEnv()->CallLongMethod(javaObject, javaMethod));
	}

	inline static jlong CallStatic(jclass javaClass, jmethodID javaMethod)
	{
		return (jlong)(GetEnv()->CallStaticLongMethod(javaClass, javaMethod));
	}
	template<class P1>
	inline static jlong Call(jobject javaObject, jmethodID javaMethod, P1 p1)
	{
		return (jlong)(GetEnv()->CallLongMethod(javaObject, javaMethod, p1));
	}

	template<class P1>
	inline static jlong CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
	{
		return (jlong)(GetEnv()->CallStaticLongMethod(javaClass, javaMethod, p1));
	}

	template<class P1, class P2>
	inline static jlong Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jlong)(GetEnv()->CallLongMethod(javaObject, javaMethod, p1, p2));
	}

	template<class P1, class P2>
	inline static jlong CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jlong)(GetEnv()->CallStaticLongMethod(javaClass, javaMethod, p1, p2));
	}

	template<class P1, class P2, class P3>
	inline static jlong Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jlong)(GetEnv()->CallLongMethod(javaObject, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3>
	inline static jlong CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jlong)(GetEnv()->CallStaticLongMethod(javaClass, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jlong Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jlong)(GetEnv()->CallLongMethod(javaObject, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jlong CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jlong)(GetEnv()->CallStaticLongMethod(javaClass, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jlong Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jlong)(GetEnv()->CallLongMethod(javaObject, javaMethod, p1, p2, p3, p4, p5));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jlong CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jlong)(GetEnv()->CallStaticLongMethod(javaClass, javaMethod, p1, p2, p3, p4, p5));
	}
};

template<> struct JniCall<jlongArray>
{
	inline static jlongArray Call(jobject javaObject, jmethodID javaMethod)
	{
		return (jlongArray)GetEnv()->CallObjectMethod(javaObject, javaMethod);
	}

	inline static jlongArray CallStatic(jclass javaClass, jmethodID javaMethod)
	{
		return (jlongArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod));
	}
	template<class P1>
	inline static jlongArray Call(jobject javaObject, jmethodID javaMethod, P1 p1)
	{
		return (jlongArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1));
	}

	template<class P1>
	inline static jlongArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
	{
		return (jlongArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1));
	}

	template<class P1, class P2>
	inline static jlongArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jlongArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2));
	}

	template<class P1, class P2>
	inline static jlongArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jlongArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2));
	}

	template<class P1, class P2, class P3>
	inline static jlongArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jlongArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3>
	inline static jlongArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jlongArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jlongArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jlongArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jlongArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jlongArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jlongArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jlongArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4, p5));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jlongArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jlongArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4, p5));
	}
};

template<> struct JniCall<jboolean>
{
	inline static jboolean Call(jobject javaObject, jmethodID javaMethod)
	{
		return (jboolean)(GetEnv()->CallBooleanMethod(javaObject, javaMethod));
	}

	inline static jboolean CallStatic(jclass javaClass, jmethodID javaMethod)
	{
		return (jboolean)(GetEnv()->CallStaticBooleanMethod(javaClass, javaMethod));
	}
	template<class P1>
	inline static jboolean Call(jobject javaObject, jmethodID javaMethod, P1 p1)
	{
		return (jboolean)(GetEnv()->CallBooleanMethod(javaObject, javaMethod, p1));
	}

	template<class P1>
	inline static jboolean CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
	{
		return (jboolean)(GetEnv()->CallStaticBooleanMethod(javaClass, javaMethod, p1));
	}

	template<class P1, class P2>
	inline static jboolean Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jboolean)(GetEnv()->CallBooleanMethod(javaObject, javaMethod, p1, p2));
	}

	template<class P1, class P2>
	inline static jboolean CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jboolean)(GetEnv()->CallStaticBooleanMethod(javaClass, javaMethod, p1, p2));
	}

	template<class P1, class P2, class P3>
	inline static jboolean Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jboolean)(GetEnv()->CallBooleanMethod(javaObject, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3>
	inline static jboolean CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jboolean)(GetEnv()->CallStaticBooleanMethod(javaClass, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jboolean Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jboolean)(GetEnv()->CallBooleanMethod(javaObject, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jboolean CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jboolean)(GetEnv()->CallStaticBooleanMethod(javaClass, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jboolean Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jboolean)(GetEnv()->CallBooleanMethod(javaObject, javaMethod, p1, p2, p3, p4, p5));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jboolean CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jboolean)(GetEnv()->CallStaticBooleanMethod(javaClass, javaMethod, p1, p2, p3, p4, p5));
	}
};

template<> struct JniCall<jbooleanArray>
{
	inline static jbooleanArray Call(jobject javaObject, jmethodID javaMethod)
	{
		return (jbooleanArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod));
	}

	inline static jbooleanArray CallStatic(jclass javaClass, jmethodID javaMethod)
	{
		return (jbooleanArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod));
	}
	template<class P1>
	inline static jbooleanArray Call(jobject javaObject, jmethodID javaMethod, P1 p1)
	{
		return (jbooleanArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1));
	}

	template<class P1>
	inline static jbooleanArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
	{
		return (jbooleanArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1));
	}

	template<class P1, class P2>
	inline static jbooleanArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jbooleanArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2));
	}

	template<class P1, class P2>
	inline static jbooleanArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jbooleanArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2));
	}

	template<class P1, class P2, class P3>
	inline static jbooleanArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jbooleanArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3>
	inline static jbooleanArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jbooleanArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jbooleanArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jbooleanArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jbooleanArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jbooleanArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jbooleanArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jbooleanArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4, p5));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jbooleanArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jbooleanArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4, p5));
	}
};

template<> struct JniCall<jobject>
{
	inline static jobject Call(jobject javaObject, jmethodID javaMethod)
	{
		return (jobject)(GetEnv()->CallObjectMethod(javaObject, javaMethod));
	}

	inline static jobject CallStatic(jclass javaClass, jmethodID javaMethod)
	{
		return (jobject)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod));
	}
	template<class P1>
	inline static jobject Call(jobject javaObject, jmethodID javaMethod, P1 p1)
	{
		return (jobject)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1));
	}

	template<class P1>
	inline static jobject CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
	{
		return (jobject)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1));
	}

	template<class P1, class P2>
	inline static jobject Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jobject)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2));
	}

	template<class P1, class P2>
	inline static jobject CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jobject)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2));
	}

	template<class P1, class P2, class P3>
	inline static jobject Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jobject)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3>
	inline static jobject CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jobject)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jobject Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jobject)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jobject CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jobject)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jobject Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jobject)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4, p5));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jobject CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jobject)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4, p5));
	}
};

template<> struct JniCall<jobjectArray>
{
	inline static jobjectArray Call(jobject javaObject, jmethodID javaMethod)
	{
		return (jobjectArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod));
	}

	inline static jobjectArray CallStatic(jclass javaClass, jmethodID javaMethod)
	{
		return (jobjectArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod));
	}
	template<class P1>
	inline static jobjectArray Call(jobject javaObject, jmethodID javaMethod, P1 p1)
	{
		return (jobjectArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1));
	}

	template<class P1>
	inline static jobjectArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
	{
		return (jobjectArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1));
	}

	template<class P1, class P2>
	inline static jobjectArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jobjectArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2));
	}

	template<class P1, class P2>
	inline static jobjectArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jobjectArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2));
	}

	template<class P1, class P2, class P3>
	inline static jobjectArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jobjectArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3>
	inline static jobjectArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jobjectArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jobjectArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jobjectArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jobjectArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jobjectArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jobjectArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jobjectArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4, p5));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jobjectArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jobjectArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4, p5));
	}
};

template<> struct JniCall<jstring>
{
	inline static jstring Call(jobject javaObject, jmethodID javaMethod)
	{
		return (jstring)(GetEnv()->CallObjectMethod(javaObject, javaMethod));
	}

	inline static jstring CallStatic(jclass javaClass, jmethodID javaMethod)
	{
		return (jstring)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod));
	}
	template<class P1>
	inline static jstring Call(jobject javaObject, jmethodID javaMethod, P1 p1)
	{
		return (jstring)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1));
	}

	template<class P1>
	inline static jstring CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
	{
		return (jstring)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1));
	}

	template<class P1, class P2>
	inline static jstring Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jstring)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2));
	}

	template<class P1, class P2>
	inline static jstring CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return (jstring)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2));
	}

	template<class P1, class P2, class P3>
	inline static jstring Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jstring)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3>
	inline static jstring CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return (jstring)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jstring Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jstring)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4>
	inline static jstring CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (jstring)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jstring Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jstring)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4, p5));
	}

	template<class P1, class P2, class P3, class P4, class P5>
	inline static jstring CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (jstring)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4, p5));
	}
};

template<> struct JniCall<jstringArray>
{
    inline static jstringArray Call(jobject javaObject, jmethodID javaMethod)
    {
        return (jobjectArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod));
    }

    inline static jstringArray CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        return (jobjectArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod));
    }
    template<class P1>
    inline static jstringArray Call(jobject javaObject, jmethodID javaMethod, P1 p1)
    {
        return (jobjectArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1));
    }

    template<class P1>
    inline static jstringArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
    {
        return (jobjectArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1));
    }

    template<class P1, class P2>
    inline static jstringArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
    {
        return (jobjectArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2));
    }

    template<class P1, class P2>
    inline static jstringArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
    {
        return (jobjectArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2));
    }

    template<class P1, class P2, class P3>
    inline static jstringArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
    {
        return (jobjectArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3));
    }

    template<class P1, class P2, class P3>
    inline static jstringArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
    {
        return (jobjectArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3));
    }

    template<class P1, class P2, class P3, class P4>
    inline static jstringArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
    {
        return (jobjectArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4));
    }

    template<class P1, class P2, class P3, class P4>
    inline static jstringArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
    {
        return (jobjectArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4));
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jstringArray Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        return (jobjectArray)(GetEnv()->CallObjectMethod(javaObject, javaMethod, p1, p2, p3, p4, p5));
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jstringArray CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        return (jobjectArray)(GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, p1, p2, p3, p4, p5));
    }
};

template<class Ret>
class MethodCaller0
{
public:
	inline static Ret Call(jobject javaObject, jmethodID javaMethod)
	{
		return JniCall<Ret>::Call(javaObject, javaMethod);
	}

	inline static Ret CallStatic(jclass javaClass, jmethodID javaMethod)
	{
		return JniCall<Ret>::CallStatic(javaClass, javaMethod);
	}
};

template<class Ret, class P1>
class MethodCaller1
{
public:
	inline static Ret Call(jobject javaObject, jmethodID javaMethod, P1 p1)
	{
		return JniCall<Ret>::Call(javaObject, javaMethod, p1);
	}

	inline static Ret CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
	{
		return JniCall<Ret>::CallStatic(javaClass, javaMethod, p1);
	}

};

template<class Ret, class P1, class P2>
class MethodCaller2
{
public:
	inline static Ret Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return JniCall<Ret>::Call(javaObject, javaMethod, p1, p2);
	}

	inline static Ret CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2)
	{
		return JniCall<Ret>::CallStatic(javaClass, javaMethod, p1, p2);
	}

};

template<class Ret, class P1, class P2, class P3>
class MethodCaller3
{
public:
	inline static Ret Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return JniCall<Ret>::Call(javaObject, javaMethod, p1, p2, p3);
	}

	inline static Ret CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
	{
		return JniCall<Ret>::CallStatic(javaClass, javaMethod, p1, p2, p3);
	}

};

template<class Ret, class P1, class P2, class P3, class P4>
class MethodCaller4
{
public:
	inline static Ret Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return JniCall<Ret>::Call(javaObject, javaMethod, p1, p2, p3, p4);
	}

	inline static Ret CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return JniCall<Ret>::CallStatic(javaClass, javaMethod, p1, p2, p3, p4);
	}

};

template<class Ret, class P1, class P2, class P3, class P4, class P5>
class MethodCaller5
{
public:
	inline static Ret Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return JniCall<Ret>::Call(javaObject, javaMethod, p1, p2, p3, p4, p5);
	}

	inline static Ret CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return JniCall<Ret>::CallStatic(javaClass, javaMethod, p1, p2, p3, p4, p5);
	}

};

template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
class MethodCaller6
{
public:
	inline static Ret Call(jobject javaObject, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
	{
		return JniCall<Ret>::Call(javaObject, javaMethod, p1, p2, p3, p4, p5, p6);
	}

	inline static Ret CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
	{
		return JniCall<Ret>::CallStatic(javaClass, javaMethod, p1, p2, p3, p4, p5, p6);
	}

};

/*
	Java Class implementation
*/
class JavaClass
{
public:
    JavaClass(const String &className);
    ~JavaClass();

    inline operator jclass() const;

    template<class Ret>
    Function<Ret (jobject)> GetMethod(String name) const;

    template<class Ret>
    Function<Ret (void)> GetStaticMethod(String name) const;

    template<class Ret, class P1>
    Function<Ret (jobject, P1)> GetMethod(String name) const;

    template<class Ret, class P1>
    Function<Ret (P1)> GetStaticMethod(String name) const;

    template<class Ret, class P1, class P2>
    Function<Ret (jobject, P1, P2)> GetMethod(String name) const;

    template<class Ret, class P1, class P2>
    Function<Ret (P1, P2)> GetStaticMethod(String name) const;

    template<class Ret, class P1, class P2, class P3>
    Function<Ret (jobject, P1, P2, P3)> GetMethod(String name) const;

    template<class Ret, class P1, class P2, class P3>
	Function<Ret (P1, P2, P3)> GetStaticMethod(String name) const;

    template<class Ret, class P1, class P2, class P3, class P4>
    Function<Ret (jobject, P1, P2, P3, P4)> GetMethod(String name) const;

    template<class Ret, class P1, class P2, class P3, class P4>
	Function<Ret (P1, P2, P3, P4)> GetStaticMethod(String name) const;

    template<class Ret, class P1, class P2, class P3, class P4, class P5>
    Function<Ret (jobject, P1, P2, P3, P4, P5)> GetMethod(String name) const;

    template<class Ret, class P1, class P2, class P3, class P4, class P5>
	Function<Ret (P1, P2, P3, P4, P5)> GetStaticMethod(String name) const;

private:
    template <class T>
    void CheckOperationResult(T value, String name) const;
    void FindJavaClass(String name);

private:
    jclass javaClass;
    String name;
};

inline JavaClass::operator jclass() const
{
	return javaClass;
}

template<class Ret>
Function<Ret (jobject)> JavaClass::GetMethod(String name) const
{
	String parametersString = SignatureString::FromTypes<Ret>();
	jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(), parametersString.c_str());
	CheckOperationResult(javaMethod, name);
	return Bind(&MethodCaller0<Ret>::Call, _1, javaMethod);
}

template<class Ret>
Function<Ret (void)> JavaClass::GetStaticMethod(String name) const
{
	String parametersString = SignatureString::FromTypes<Ret>();
	jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(), parametersString.c_str());
	CheckOperationResult(javaMethod, name);
	return Bind(&MethodCaller0<Ret>::CallStatic, javaClass, javaMethod);
}

template<class Ret, class P1>
Function<Ret (jobject, P1)> JavaClass::GetMethod(String name) const
{
	String parametersString = SignatureString::FromTypes<Ret, P1>();
	jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(), parametersString.c_str());
	CheckOperationResult(javaMethod, name);
	return Bind(&MethodCaller1<Ret, P1>::Call, _1, javaMethod, _2);
}

template<class Ret, class P1>
Function<Ret (P1)> JavaClass::GetStaticMethod(String name) const
{
	String parametersString = SignatureString::FromTypes<Ret, P1>();
	jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(), parametersString.c_str());
	CheckOperationResult(javaMethod, name);
	return Bind(&MethodCaller1<Ret, P1>::CallStatic, javaClass, javaMethod, _1);
}

template<class Ret, class P1, class P2>
Function<Ret (jobject, P1, P2)> JavaClass::GetMethod(String name) const
{
	String parametersString = SignatureString::FromTypes<Ret, P1, P2>();
	jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(), parametersString.c_str());
	CheckOperationResult(javaMethod, name);
	return Bind(&MethodCaller2<Ret, P1, P2>::Call,  _1, javaMethod, _2, _3);
}

template<class Ret, class P1, class P2>
Function<Ret (P1, P2)> JavaClass::GetStaticMethod(String name) const
{
	String parametersString = SignatureString::FromTypes<Ret, P1, P2>();
	jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(), parametersString.c_str());
	CheckOperationResult(javaMethod, name);
	return Bind(&MethodCaller2<Ret, P1, P2>::CallStatic, javaClass, javaMethod, _1, _2);
}

template<class Ret, class P1, class P2, class P3>
Function<Ret (jobject, P1, P2, P3)> JavaClass::GetMethod(String name) const
{
	String parametersString = SignatureString::FromTypes<Ret, P1, P2, P3>();
	jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(), parametersString.c_str());
	CheckOperationResult(javaMethod, name);
	return Bind(&MethodCaller3<Ret, P1, P2, P3>::Call,  _1, javaMethod, _2, _3, _4);
}

template<class Ret, class P1, class P2, class P3>
Function<Ret (P1, P2, P3)> JavaClass::GetStaticMethod(String name) const
{
	String parametersString = SignatureString::FromTypes<Ret, P1, P2, P3>();
	jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(), parametersString.c_str());
	CheckOperationResult(javaMethod, name);
	return Bind(&MethodCaller3<Ret, P1, P2, P3>::CallStatic, javaClass, javaMethod, _1, _2, _3);
}

template<class Ret, class P1, class P2, class P3, class P4>
Function<Ret (jobject, P1, P2, P3, P4)> JavaClass::GetMethod(String name) const
{
	String parametersString = SignatureString::FromTypes<Ret, P1, P2, P3, P4>();
	jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(), parametersString.c_str());
	CheckOperationResult(javaMethod, name);
	return Bind(&MethodCaller4<Ret, P1, P2, P3, P4>::Call, _1, javaMethod, _2, _3, _4, _5);
}

template<class Ret, class P1, class P2, class P3, class P4>
Function<Ret (P1, P2, P3, P4)> JavaClass::GetStaticMethod(String name) const
{
	String parametersString = SignatureString::FromTypes<Ret, P1, P2, P3, P4>();
	jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(), parametersString.c_str());
	CheckOperationResult(javaMethod, name);
	return Bind(&MethodCaller4<Ret, P1, P2, P3, P4>::CallStatic, javaClass, javaMethod, _1, _2, _3, _4);
}

template<class Ret, class P1, class P2, class P3, class P4, class P5>
Function<Ret (jobject, P1, P2, P3, P4, P5)> JavaClass::GetMethod(String name) const
{
	String parametersString = SignatureString::FromTypes<Ret, P1, P2, P3, P4, P5>();
	jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(), parametersString.c_str());
	CheckOperationResult(javaMethod, name);
	return Bind(&MethodCaller5<Ret, P1, P2, P3, P4, P5>::Call,  _1, javaMethod, _2, _3, _4, _5, _6);
}

template<class Ret, class P1, class P2, class P3, class P4, class P5>
Function<Ret (P1, P2, P3, P4, P5)> JavaClass::GetStaticMethod(String name) const
{
	String parametersString = SignatureString::FromTypes<Ret, P1, P2, P3, P4, P5>();
	jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(), parametersString.c_str());
	CheckOperationResult(javaMethod, name);
	return Bind(&MethodCaller5<Ret, P1, P2, P3, P4, P5>::CallStatic, javaClass, javaMethod, _1, _2, _3, _4, _5);
}

template <class T>
void JavaClass::CheckOperationResult(T value, String name) const
{
	if (NULL != value)
		return;

	JNIEnv *env = GetEnv();
    if (env->ExceptionOccurred())
    {
    	env->ExceptionDescribe();
    	env->ExceptionClear();

    	String ErrorStr = Format("Didn't find %s", name.c_str());

    	Logger::Error(ErrorStr.c_str());

    	DVASSERT_MSG(NULL != value, ErrorStr.c_str());
    }
}

}
}

#endif

#endif
