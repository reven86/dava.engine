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

#define DAVA_JNI_EXCEPTION_CHECK \
{\
    JNIEnv *env = JNI::GetEnv();\
    jthrowable e = env->ExceptionOccurred();\
    if (nullptr != e)\
    {\
        /*first you SHOULD clear exception state in VM*/ \
        env->ExceptionClear();\
        jmethodID toString = env->GetMethodID(\
                env->FindClass("java/lang/Object"),\
                "toString", "()Ljava/lang/String;");\
        jstring estring = (jstring) env->CallObjectMethod(e, toString);\
        jboolean isCopy = false;\
        const char* utf = env->GetStringUTFChars(estring, &isCopy);\
        String error(utf);\
        env->ReleaseStringUTFChars(estring, utf);\
        DVASSERT_MSG(false, error.c_str());\
    }\
}

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
{
};

template<> struct TypeMetrics<void>
{
    DeclareTypeString("V")
};

template<> struct TypeMetrics<jint>
{
    DeclareTypeString("I")
};

template<> struct TypeMetrics<jintArray>
{
    DeclareTypeString("[I")
};

template<> struct TypeMetrics<jfloat>
{
    DeclareTypeString("F")
};

template<> struct TypeMetrics<jfloatArray>
{
    DeclareTypeString("[F")
};

template<> struct TypeMetrics<jdouble>
{
    DeclareTypeString("D")
};

template<> struct TypeMetrics<jdoubleArray>
{
    DeclareTypeString("[D")
};

template<> struct TypeMetrics<jlong>
{
    DeclareTypeString("J")
};

template<> struct TypeMetrics<jlongArray>
{
    DeclareTypeString("[J")
};

template<> struct TypeMetrics<jstring>
{
    DeclareTypeString("Ljava/lang/String;")
};

template<> struct TypeMetrics<jstringArray>
{
    DeclareTypeStringWithRet("[Ljava/lang/String;", "[Ljava/lang/Object;")
};

template<> struct TypeMetrics<jobject>
{
    DeclareTypeString("Ljava/lang/Object;")
};

template<> struct TypeMetrics<jobjectArray>
{
    DeclareTypeString("[Ljava/lang/Object;")
};

template<> struct TypeMetrics<jboolean>
{
    DeclareTypeString("Z")
};

template<> struct TypeMetrics<jbooleanArray>
{
    DeclareTypeString("[Z")
};

template<> struct TypeMetrics<jbyte>
{
	DeclareTypeString("B");
};

template<> struct TypeMetrics<jbyteArray>
{
	DeclareTypeString("[B");
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

    template<class Ret, class P1, class P2, class P3, class P4, class P5,
            class P6>
    inline static const String &FromTypes();
};

template<class Ret>
const String &SignatureString::FromTypes()
{
    static String ret = String("()") + TypeMetrics<Ret>().rvalue;
    return ret;
}

template<class Ret, class P1>
const String &SignatureString::FromTypes()
{
    static String ret = String("(") + TypeMetrics<P1>().value + String(")")
            + TypeMetrics<Ret>().rvalue;
    return ret;
}

template<class Ret, class P1, class P2>
const String &SignatureString::FromTypes()
{
    static String ret = String("(") + TypeMetrics<P1>().value
            + TypeMetrics<P2>().value + String(")") + TypeMetrics<Ret>().rvalue;
    return ret;
}

template<class Ret, class P1, class P2, class P3>
const String &SignatureString::FromTypes()
{
    static String ret = String("(") + TypeMetrics<P1>().value
            + TypeMetrics<P2>().value + TypeMetrics<P3>().value + String(")")
            + TypeMetrics<Ret>().rvalue;
    return ret;
}
template<class Ret, class P1, class P2, class P3, class P4>
const String &SignatureString::FromTypes()
{
    static String ret = String("(") + TypeMetrics<P1>().value
            + TypeMetrics<P2>().value + TypeMetrics<P3>().value
            + TypeMetrics<P4>().value + String(")") + TypeMetrics<Ret>().rvalue;
    return ret;
}

template<class Ret, class P1, class P2, class P3, class P4, class P5>
const String &SignatureString::FromTypes()
{
    static String ret = String("(") + TypeMetrics<P1>().value
            + TypeMetrics<P2>().value + TypeMetrics<P3>().value
            + TypeMetrics<P4>().value + TypeMetrics<P5>().value + String(")")
            + TypeMetrics<Ret>().rvalue;
    return ret;
}

template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
const String &SignatureString::FromTypes()
{
    static String ret = String("(") + TypeMetrics<P1>().value
            + TypeMetrics<P2>().value + TypeMetrics<P3>().value
            + TypeMetrics<P4>().value + TypeMetrics<P5>().value
            + TypeMetrics<P6>().value + String(")") + TypeMetrics<Ret>().rvalue;
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

template<class Ret>
struct JniCall
{
};

template<> struct JniCall<void>
{
    inline static void Call(jobject javaObject, jmethodID javaMethod)
    {
        GetEnv()->CallVoidMethod(javaObject, javaMethod);
        DAVA_JNI_EXCEPTION_CHECK
    }

    inline static void CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        GetEnv()->CallStaticVoidMethod(javaClass, javaMethod);
        DAVA_JNI_EXCEPTION_CHECK
    }

    template<class P1>
    inline static void Call(jobject javaObject, jmethodID javaMethod, P1 p1)
    {
        GetEnv()->CallVoidMethod(javaObject, javaMethod, p1);
        DAVA_JNI_EXCEPTION_CHECK
    }

    template<class P1>
    inline static void CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
    {
        GetEnv()->CallStaticVoidMethod(javaClass, javaMethod, p1);
        DAVA_JNI_EXCEPTION_CHECK
    }

    template<class P1, class P2>
    inline static void Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2)
    {
        GetEnv()->CallVoidMethod(javaObject, javaMethod, p1, p2);
        DAVA_JNI_EXCEPTION_CHECK
    }

    template<class P1, class P2>
    inline static void CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1,
            P2 p2)
    {
        GetEnv()->CallStaticVoidMethod(javaClass, javaMethod, p1, p2);
        DAVA_JNI_EXCEPTION_CHECK
    }

    template<class P1, class P2, class P3>
    inline static void Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3)
    {
        GetEnv()->CallVoidMethod(javaObject, javaMethod, p1, p2, p3);
        DAVA_JNI_EXCEPTION_CHECK
    }

    template<class P1, class P2, class P3>
    inline static void CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3)
    {
        GetEnv()->CallStaticVoidMethod(javaClass, javaMethod, p1, p2, p3);
        DAVA_JNI_EXCEPTION_CHECK
    }

    template<class P1, class P2, class P3, class P4>
    inline static void Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4)
    {
        GetEnv()->CallVoidMethod(javaObject, javaMethod, p1, p2, p3, p4);
        DAVA_JNI_EXCEPTION_CHECK
    }

    template<class P1, class P2, class P3, class P4>
    inline static void CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4)
    {
        GetEnv()->CallStaticVoidMethod(javaClass, javaMethod, p1, p2, p3, p4);
        DAVA_JNI_EXCEPTION_CHECK
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static void Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4, P5 p5)
    {
        GetEnv()->CallVoidMethod(javaObject, javaMethod, p1, p2, p3, p4, p5);
        DAVA_JNI_EXCEPTION_CHECK
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static void CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4, P5 p5)
    {
        GetEnv()->CallStaticVoidMethod(javaClass, javaMethod, p1, p2, p3, p4,
                p5);
        DAVA_JNI_EXCEPTION_CHECK
    }
};

template<> struct JniCall<jint>
{

    inline static jint Call(jobject javaObject, jmethodID javaMethod)
    {
        jint r = (jint) (GetEnv()->CallIntMethod(javaObject, javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jint CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        jint r = (jint) (GetEnv()->CallStaticIntMethod(javaClass, javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jint Call(jobject javaObject, jmethodID javaMethod, P1 p1)
    {
        jint r = (jint) (GetEnv()->CallIntMethod(javaObject, javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jint CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1)
    {
        jint r = (jint) (GetEnv()->CallStaticIntMethod(javaClass, javaMethod,
                p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jint Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2)
    {
        jint r =
                (jint) (GetEnv()->CallIntMethod(javaObject, javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jint CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1,
            P2 p2)
    {
        jint r = (jint) (GetEnv()->CallStaticIntMethod(javaClass, javaMethod,
                p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jint Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3)
    {
        jint r = (jint) (GetEnv()->CallIntMethod(javaObject, javaMethod, p1, p2,
                p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jint CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3)
    {
        jint r = (jint) (GetEnv()->CallStaticIntMethod(javaClass, javaMethod,
                p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jint Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4)
    {
        jint r = (jint) (GetEnv()->CallIntMethod(javaObject, javaMethod, p1, p2,
                p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jint CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4)
    {
        jint r = (jint) (GetEnv()->CallStaticIntMethod(javaClass, javaMethod,
                p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jint Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jint r = (jint) (GetEnv()->CallIntMethod(javaObject, javaMethod, p1, p2,
                p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jint CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jint r = (jint) (GetEnv()->CallStaticIntMethod(javaClass, javaMethod,
                p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jintArray>
{
    inline static jintArray Call(jobject javaObject, jmethodID javaMethod)
    {
        jintArray r = (jintArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jintArray CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        jintArray r = (jintArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jintArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1)
    {
        jintArray r = (jintArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jintArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1)
    {
        jintArray r = (jintArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jintArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jintArray r = (jintArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jintArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jintArray r = (jintArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jintArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jintArray r = (jintArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jintArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jintArray r = (jintArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jintArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jintArray r = (jintArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jintArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jintArray r = (jintArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jintArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jintArray r = (jintArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jintArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jintArray r = (jintArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jfloat>
{
    inline static jfloat Call(jobject javaObject, jmethodID javaMethod)
    {
        jfloat r = (jfloat) (GetEnv()->CallFloatMethod(javaObject, javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jfloat CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        jfloat r = (jfloat) (GetEnv()->CallStaticFloatMethod(javaClass,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jfloat Call(jobject javaObject, jmethodID javaMethod, P1 p1)
    {
        jfloat r = (jfloat) (GetEnv()->CallFloatMethod(javaObject, javaMethod,
                p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jfloat CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1)
    {
        jfloat r = (jfloat) (GetEnv()->CallStaticFloatMethod(javaClass,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jfloat Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2)
    {
        jfloat r = (jfloat) (GetEnv()->CallFloatMethod(javaObject, javaMethod,
                p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jfloat CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jfloat r = (jfloat) (GetEnv()->CallStaticFloatMethod(javaClass,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jfloat Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3)
    {
        jfloat r = (jfloat) (GetEnv()->CallFloatMethod(javaObject, javaMethod,
                p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jfloat CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jfloat r = (jfloat) (GetEnv()->CallStaticFloatMethod(javaClass,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jfloat Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4)
    {
        jfloat r = (jfloat) (GetEnv()->CallFloatMethod(javaObject, javaMethod,
                p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jfloat CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jfloat r = (jfloat) (GetEnv()->CallStaticFloatMethod(javaClass,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jfloat Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jfloat r = (jfloat) (GetEnv()->CallFloatMethod(javaObject, javaMethod,
                p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jfloat CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jfloat r = (jfloat) (GetEnv()->CallStaticFloatMethod(javaClass,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jfloatArray>
{
    inline static jfloatArray Call(jobject javaObject, jmethodID javaMethod)
    {
        jfloatArray r = (jfloatArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jfloatArray CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        jfloatArray r = (jfloatArray) (GetEnv()->CallObjectMethod(javaClass,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jfloatArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1)
    {
        jfloatArray r = (jfloatArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jfloatArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1)
    {
        jfloatArray r = (jfloatArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jfloatArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jfloatArray r = (jfloatArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jfloatArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jfloatArray r = (jfloatArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jfloatArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jfloatArray r = (jfloatArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jfloatArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jfloatArray r = (jfloatArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jfloatArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jfloatArray r = (jfloatArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jfloatArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jfloatArray r = (jfloatArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jfloatArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jfloatArray r = (jfloatArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jfloatArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jfloatArray r = (jfloatArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jdouble>
{
    inline static jdouble Call(jobject javaObject, jmethodID javaMethod)
    {
        jdouble r = (jdouble) (GetEnv()->CallDoubleMethod(javaObject,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jdouble CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        jdouble r = (jdouble) (GetEnv()->CallStaticDoubleMethod(javaClass,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jdouble Call(jobject javaObject, jmethodID javaMethod, P1 p1)
    {
        jdouble r = (jdouble) (GetEnv()->CallDoubleMethod(javaObject,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jdouble CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1)
    {
        jdouble r = (jdouble) (GetEnv()->CallStaticDoubleMethod(javaClass,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jdouble Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2)
    {
        jdouble r = (jdouble) (GetEnv()->CallDoubleMethod(javaObject,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jdouble CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jdouble r = (jdouble) (GetEnv()->CallStaticDoubleMethod(javaClass,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jdouble Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3)
    {
        jdouble r = (jdouble) (GetEnv()->CallDoubleMethod(javaObject,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jdouble CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jdouble r = (jdouble) (GetEnv()->CallStaticDoubleMethod(javaClass,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jdouble Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4)
    {
        jdouble r = (jdouble) (GetEnv()->CallDoubleMethod(javaObject,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jdouble CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jdouble r = (jdouble) (GetEnv()->CallStaticDoubleMethod(javaClass,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jdouble Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jdouble r = (jdouble) (GetEnv()->CallDoubleMethod(javaObject,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jdouble CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jdouble r = (jdouble) (GetEnv()->CallStaticDoubleMethod(javaClass,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jdoubleArray>
{
    inline static jdoubleArray Call(jobject javaObject, jmethodID javaMethod)
    {
        jdoubleArray r = (jdoubleArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jdoubleArray CallStatic(jclass javaClass,
            jmethodID javaMethod)
    {
        jdoubleArray r = (jdoubleArray) (GetEnv()->CallObjectMethod(javaClass,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jdoubleArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1)
    {
        jdoubleArray r = (jdoubleArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jdoubleArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1)
    {
        jdoubleArray r = (jdoubleArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jdoubleArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jdoubleArray r = (jdoubleArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jdoubleArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2)
    {
        jdoubleArray r = (jdoubleArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jdoubleArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jdoubleArray r = (jdoubleArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jdoubleArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
    {
        jdoubleArray r = (jdoubleArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jdoubleArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jdoubleArray r = (jdoubleArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jdoubleArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jdoubleArray r = (jdoubleArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jdoubleArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jdoubleArray r = (jdoubleArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jdoubleArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jdoubleArray r = (jdoubleArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jlong>
{
    inline static jlong Call(jobject javaObject, jmethodID javaMethod)
    {
        jlong r = (jlong) (GetEnv()->CallLongMethod(javaObject, javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jlong CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        jlong r =
                (jlong) (GetEnv()->CallStaticLongMethod(javaClass, javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jlong Call(jobject javaObject, jmethodID javaMethod, P1 p1)
    {
        jlong r = (jlong) (GetEnv()->CallLongMethod(javaObject, javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jlong CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1)
    {
        jlong r = (jlong) (GetEnv()->CallStaticLongMethod(javaClass, javaMethod,
                p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jlong Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2)
    {
        jlong r = (jlong) (GetEnv()->CallLongMethod(javaObject, javaMethod, p1,
                p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jlong CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jlong r = (jlong) (GetEnv()->CallStaticLongMethod(javaClass, javaMethod,
                p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jlong Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3)
    {
        jlong r = (jlong) (GetEnv()->CallLongMethod(javaObject, javaMethod, p1,
                p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jlong CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jlong r = (jlong) (GetEnv()->CallStaticLongMethod(javaClass, javaMethod,
                p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jlong Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4)
    {
        jlong r = (jlong) (GetEnv()->CallLongMethod(javaObject, javaMethod, p1,
                p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jlong CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jlong r = (jlong) (GetEnv()->CallStaticLongMethod(javaClass, javaMethod,
                p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jlong Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jlong r = (jlong) (GetEnv()->CallLongMethod(javaObject, javaMethod, p1,
                p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jlong CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jlong r = (jlong) (GetEnv()->CallStaticLongMethod(javaClass, javaMethod,
                p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jlongArray>
{
    inline static jlongArray Call(jobject javaObject, jmethodID javaMethod)
    {
        jlongArray r = (jlongArray) GetEnv()->CallObjectMethod(javaObject,
                javaMethod);
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jlongArray CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        jlongArray r = (jlongArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jlongArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1)
    {
        jlongArray r = (jlongArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jlongArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1)
    {
        jlongArray r = (jlongArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jlongArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jlongArray r = (jlongArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jlongArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jlongArray r = (jlongArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jlongArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jlongArray r = (jlongArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jlongArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jlongArray r = (jlongArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jlongArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jlongArray r = (jlongArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jlongArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jlongArray r = (jlongArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jlongArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jlongArray r = (jlongArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jlongArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jlongArray r = (jlongArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jboolean>
{
    inline static jboolean Call(jobject javaObject, jmethodID javaMethod)
    {
        jboolean r = (jboolean) (GetEnv()->CallBooleanMethod(javaObject,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jboolean CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        jboolean r = (jboolean) (GetEnv()->CallStaticBooleanMethod(javaClass,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jboolean Call(jobject javaObject, jmethodID javaMethod, P1 p1)
    {
        jboolean r = (jboolean) (GetEnv()->CallBooleanMethod(javaObject,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jboolean CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1)
    {
        jboolean r = (jboolean) (GetEnv()->CallStaticBooleanMethod(javaClass,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jboolean Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2)
    {
        jboolean r = (jboolean) (GetEnv()->CallBooleanMethod(javaObject,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jboolean CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jboolean r = (jboolean) (GetEnv()->CallStaticBooleanMethod(javaClass,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jboolean Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3)
    {
        jboolean r = (jboolean) (GetEnv()->CallBooleanMethod(javaObject,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jboolean CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jboolean r = (jboolean) (GetEnv()->CallStaticBooleanMethod(javaClass,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jboolean Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4)
    {
        jboolean r = (jboolean) (GetEnv()->CallBooleanMethod(javaObject,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jboolean CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jboolean r = (jboolean) (GetEnv()->CallStaticBooleanMethod(javaClass,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jboolean Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jboolean r = (jboolean) (GetEnv()->CallBooleanMethod(javaObject,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jboolean CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jboolean r = (jboolean) (GetEnv()->CallStaticBooleanMethod(javaClass,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jbooleanArray>
{
    inline static jbooleanArray Call(jobject javaObject, jmethodID javaMethod)
    {
        jbooleanArray r = (jbooleanArray) (GetEnv()->CallObjectMethod(
                javaObject, javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jbooleanArray CallStatic(jclass javaClass,
            jmethodID javaMethod)
    {
        jbooleanArray r = (jbooleanArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jbooleanArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1)
    {
        jbooleanArray r = (jbooleanArray) (GetEnv()->CallObjectMethod(
                javaObject, javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jbooleanArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1)
    {
        jbooleanArray r = (jbooleanArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jbooleanArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jbooleanArray r = (jbooleanArray) (GetEnv()->CallObjectMethod(
                javaObject, javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jbooleanArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2)
    {
        jbooleanArray r = (jbooleanArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jbooleanArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jbooleanArray r = (jbooleanArray) (GetEnv()->CallObjectMethod(
                javaObject, javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jbooleanArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
    {
        jbooleanArray r = (jbooleanArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jbooleanArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jbooleanArray r = (jbooleanArray) (GetEnv()->CallObjectMethod(
                javaObject, javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jbooleanArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jbooleanArray r = (jbooleanArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jbooleanArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jbooleanArray r = (jbooleanArray) (GetEnv()->CallObjectMethod(
                javaObject, javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jbooleanArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jbooleanArray r = (jbooleanArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jobject>
{
    inline static jobject Call(jobject javaObject, jmethodID javaMethod)
    {
        jobject r = (jobject) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jobject CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        jobject r = (jobject) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jobject Call(jobject javaObject, jmethodID javaMethod, P1 p1)
    {
        jobject r = (jobject) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jobject CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1)
    {
        jobject r = (jobject) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jobject Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2)
    {
        jobject r = (jobject) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jobject CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jobject r = (jobject) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jobject Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3)
    {
        jobject r = (jobject) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jobject CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jobject r = (jobject) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jobject Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4)
    {
        jobject r = (jobject) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jobject CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jobject r = (jobject) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jobject Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jobject r = (jobject) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jobject CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jobject r = (jobject) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jobjectArray>
{
    inline static jobjectArray Call(jobject javaObject, jmethodID javaMethod)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jobjectArray CallStatic(jclass javaClass,
            jmethodID javaMethod)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jobjectArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jobjectArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jobjectArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jobjectArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jobjectArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jobjectArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jobjectArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jobjectArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jobjectArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jobjectArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jstring>
{
    inline static jstring Call(jobject javaObject, jmethodID javaMethod)
    {
        jstring r = (jstring) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jstring CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        jstring r = (jstring) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jstring Call(jobject javaObject, jmethodID javaMethod, P1 p1)
    {
        jstring r = (jstring) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jstring CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1)
    {
        jstring r = (jstring) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jstring Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2)
    {
        jstring r = (jstring) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jstring CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jstring r = (jstring) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jstring Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3)
    {
        jstring r = (jstring) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jstring CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jstring r = (jstring) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jstring Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4)
    {
        jstring r = (jstring) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jstring CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jstring r = (jstring) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jstring Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jstring r = (jstring) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jstring CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jstring r = (jstring) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jstringArray>
{
    inline static jstringArray Call(jobject javaObject, jmethodID javaMethod)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jstringArray CallStatic(jclass javaClass,
            jmethodID javaMethod)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jstringArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jstringArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jstringArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jstringArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jstringArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jstringArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2, P3 p3)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jstringArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jstringArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jstringArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jstringArray CallStatic(jclass javaClass,
            jmethodID javaMethod, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallStaticObjectMethod(
                javaClass, javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jbyte>
{
    inline static jbyte Call(jobject javaObject, jmethodID javaMethod)
    {
        jbyte r = (jbyte) (GetEnv()->CallBooleanMethod(javaObject, javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jbyte CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        jbyte r = (jbyte) (GetEnv()->CallStaticBooleanMethod(javaClass,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jbyte Call(jobject javaObject, jmethodID javaMethod, P1 p1)
    {
        jbyte r = (jbyte) (GetEnv()->CallBooleanMethod(javaObject, javaMethod,
                p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jbyte CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1)
    {
        jbyte r = (jbyte) (GetEnv()->CallStaticBooleanMethod(javaClass,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jbyte Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2)
    {
        jbyte r = (jbyte) (GetEnv()->CallBooleanMethod(javaObject, javaMethod,
                p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jbyte CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jbyte r = (jbyte) (GetEnv()->CallStaticBooleanMethod(javaClass,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jbyte Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3)
    {
        jbyte r = (jbyte) (GetEnv()->CallBooleanMethod(javaObject, javaMethod,
                p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jbyte CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jbyte r = (jbyte) (GetEnv()->CallStaticBooleanMethod(javaClass,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jbyte Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4)
    {
        jbyte r = (jbyte) (GetEnv()->CallBooleanMethod(javaObject, javaMethod,
                p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jbyte CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jbyte r = (jbyte) (GetEnv()->CallStaticBooleanMethod(javaClass,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jbyte Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jbyte r = (jbyte) (GetEnv()->CallBooleanMethod(javaObject, javaMethod,
                p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jbyte CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jbyte r = (jbyte) (GetEnv()->CallStaticBooleanMethod(javaClass,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jbyteArray>
{
    inline static jbyteArray Call(jobject javaObject, jmethodID javaMethod)
    {
        jbyteArray r = (jbyteArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    inline static jbyteArray CallStatic(jclass javaClass, jmethodID javaMethod)
    {
        jbyteArray r = (jbyteArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
    template<class P1>
    inline static jbyteArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1)
    {
        jbyteArray r = (jbyteArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1>
    inline static jbyteArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1)
    {
        jbyteArray r = (jbyteArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jbyteArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jbyteArray r = (jbyteArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2>
    inline static jbyteArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2)
    {
        jbyteArray r = (jbyteArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jbyteArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jbyteArray r = (jbyteArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3>
    inline static jbyteArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3)
    {
        jbyteArray r = (jbyteArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2, p3));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jbyteArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jbyteArray r = (jbyteArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4>
    inline static jbyteArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4)
    {
        jbyteArray r = (jbyteArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2, p3, p4));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jbyteArray Call(jobject javaObject, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jbyteArray r = (jbyteArray) (GetEnv()->CallObjectMethod(javaObject,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class P1, class P2, class P3, class P4, class P5>
    inline static jbyteArray CallStatic(jclass javaClass, jmethodID javaMethod,
            P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        jbyteArray r = (jbyteArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, p1, p2, p3, p4, p5));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
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
    inline static Ret Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2)
    {
        return JniCall<Ret>::Call(javaObject, javaMethod, p1, p2);
    }

    inline static Ret CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1,
            P2 p2)
    {
        return JniCall<Ret>::CallStatic(javaClass, javaMethod, p1, p2);
    }

};

template<class Ret, class P1, class P2, class P3>
class MethodCaller3
{
public:
    inline static Ret Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3)
    {
        return JniCall<Ret>::Call(javaObject, javaMethod, p1, p2, p3);
    }

    inline static Ret CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3)
    {
        return JniCall<Ret>::CallStatic(javaClass, javaMethod, p1, p2, p3);
    }

};

template<class Ret, class P1, class P2, class P3, class P4>
class MethodCaller4
{
public:
    inline static Ret Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4)
    {
        return JniCall<Ret>::Call(javaObject, javaMethod, p1, p2, p3, p4);
    }

    inline static Ret CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4)
    {
        return JniCall<Ret>::CallStatic(javaClass, javaMethod, p1, p2, p3, p4);
    }

};

template<class Ret, class P1, class P2, class P3, class P4, class P5>
class MethodCaller5
{
public:
    inline static Ret Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4, P5 p5)
    {
        return JniCall<Ret>::Call(javaObject, javaMethod, p1, p2, p3, p4, p5);
    }

    inline static Ret CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4, P5 p5)
    {
        return JniCall<Ret>::CallStatic(javaClass, javaMethod, p1, p2, p3, p4,
                p5);
    }

};

template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
class MethodCaller6
{
public:
    inline static Ret Call(jobject javaObject, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    {
        return JniCall<Ret>::Call(javaObject, javaMethod, p1, p2, p3, p4, p5,
                p6);
    }

    inline static Ret CallStatic(jclass javaClass, jmethodID javaMethod, P1 p1,
            P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    {
        return JniCall<Ret>::CallStatic(javaClass, javaMethod, p1, p2, p3, p4,
                p5, p6);
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
    Function<Ret(jobject)> GetMethod(String name) const;

    template<class Ret>
    Function<Ret(void)> GetStaticMethod(String name) const;

    template<class Ret, class P1>
    Function<Ret(jobject, P1)> GetMethod(String name) const;

    template<class Ret, class P1>
    Function<Ret(P1)> GetStaticMethod(String name) const;

    template<class Ret, class P1, class P2>
    Function<Ret(jobject, P1, P2)> GetMethod(String name) const;

    template<class Ret, class P1, class P2>
    Function<Ret(P1, P2)> GetStaticMethod(String name) const;

    template<class Ret, class P1, class P2, class P3>
    Function<Ret(jobject, P1, P2, P3)> GetMethod(String name) const;

    template<class Ret, class P1, class P2, class P3>
    Function<Ret(P1, P2, P3)> GetStaticMethod(String name) const;

    template<class Ret, class P1, class P2, class P3, class P4>
    Function<Ret(jobject, P1, P2, P3, P4)> GetMethod(String name) const;

    template<class Ret, class P1, class P2, class P3, class P4>
    Function<Ret(P1, P2, P3, P4)> GetStaticMethod(String name) const;

    template<class Ret, class P1, class P2, class P3, class P4, class P5>
    Function<Ret(jobject, P1, P2, P3, P4, P5)> GetMethod(String name) const;

    template<class Ret, class P1, class P2, class P3, class P4, class P5>
    Function<Ret(P1, P2, P3, P4, P5)> GetStaticMethod(String name) const;

private:
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
Function<Ret(jobject)> JavaClass::GetMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret>();
    jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller0<Ret>::Call, _1, javaMethod);
}

template<class Ret>
Function<Ret(void)> JavaClass::GetStaticMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret>();
    jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller0<Ret>::CallStatic, javaClass, javaMethod);
}

template<class Ret, class P1>
Function<Ret(jobject, P1)> JavaClass::GetMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1>();
    jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller1<Ret, P1>::Call, _1, javaMethod, _2);
}

template<class Ret, class P1>
Function<Ret(P1)> JavaClass::GetStaticMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1>();
    jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller1<Ret, P1>::CallStatic, javaClass, javaMethod, _1);
}

template<class Ret, class P1, class P2>
Function<Ret(jobject, P1, P2)> JavaClass::GetMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1, P2>();
    jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller2<Ret, P1, P2>::Call, _1, javaMethod, _2, _3);
}

template<class Ret, class P1, class P2>
Function<Ret(P1, P2)> JavaClass::GetStaticMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1, P2>();
    jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller2<Ret, P1, P2>::CallStatic, javaClass, javaMethod,
            _1, _2);
}

template<class Ret, class P1, class P2, class P3>
Function<Ret(jobject, P1, P2, P3)> JavaClass::GetMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1, P2, P3>();
    jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller3<Ret, P1, P2, P3>::Call, _1, javaMethod, _2, _3,
            _4);
}

template<class Ret, class P1, class P2, class P3>
Function<Ret(P1, P2, P3)> JavaClass::GetStaticMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1, P2, P3>();
    jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller3<Ret, P1, P2, P3>::CallStatic, javaClass,
            javaMethod, _1, _2, _3);
}

template<class Ret, class P1, class P2, class P3, class P4>
Function<Ret(jobject, P1, P2, P3, P4)> JavaClass::GetMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1, P2, P3, P4>();
    jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller4<Ret, P1, P2, P3, P4>::Call, _1, javaMethod, _2,
            _3, _4, _5);
}

template<class Ret, class P1, class P2, class P3, class P4>
Function<Ret(P1, P2, P3, P4)> JavaClass::GetStaticMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1, P2, P3, P4>();
    jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller4<Ret, P1, P2, P3, P4>::CallStatic, javaClass,
            javaMethod, _1, _2, _3, _4);
}

template<class Ret, class P1, class P2, class P3, class P4, class P5>
Function<Ret(jobject, P1, P2, P3, P4, P5)> JavaClass::GetMethod(
        String name) const
{
    String parametersString =
            SignatureString::FromTypes<Ret, P1, P2, P3, P4, P5>();
    jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller5<Ret, P1, P2, P3, P4, P5>::Call, _1, javaMethod,
            _2, _3, _4, _5, _6);
}

template<class Ret, class P1, class P2, class P3, class P4, class P5>
Function<Ret(P1, P2, P3, P4, P5)> JavaClass::GetStaticMethod(String name) const
{
    String parametersString =
            SignatureString::FromTypes<Ret, P1, P2, P3, P4, P5>();
    jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller5<Ret, P1, P2, P3, P4, P5>::CallStatic, javaClass,
            javaMethod, _1, _2, _3, _4, _5);
}

} // end namespace JNI
} // end namespace DAVA

#endif

#endif
