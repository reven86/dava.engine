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
#include "Functional/Function.h"
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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wclass-varargs"

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
private:
    template<class Ret>
    static const String& GenRecursive()
    {
        static String r = String(")") + TypeMetrics<Ret>().rvalue;
        return r;
    }

    template<class P1, class P2, class... POthers>
    static const String& GenRecursive()
    {
        static String r = TypeMetrics<P1>().value + GenRecursive<P2, POthers...>();
        return r;

    }

public:
    template<class Ret, class... POthers>
    inline static const String &FromTypes()
    {
        static String ret = String("(") + GenRecursive<POthers..., Ret>();
        return ret;
    }
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
    template<class... Parameters>
    inline static void Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        GetEnv()->CallVoidMethod(javaObject, javaMethod, std::forward<Parameters>(params)...);
        DAVA_JNI_EXCEPTION_CHECK
    }

    template<class... Parameters>
    inline static void CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        GetEnv()->CallStaticVoidMethod(javaClass, javaMethod, std::forward<Parameters>(params)...);
        DAVA_JNI_EXCEPTION_CHECK
    }
};

template<> struct JniCall<jint>
{

    template<class... Parameters>
    inline static jint Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jint r = (jint) (GetEnv()->CallIntMethod(javaObject, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jint CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jint r = (jint) (GetEnv()->CallStaticIntMethod(javaClass, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jintArray>
{
    template<class... Parameters>
    inline static jintArray Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jintArray r = (jintArray) (GetEnv()->CallObjectMethod(javaObject, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jintArray CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jintArray r = (jintArray) (GetEnv()->CallStaticObjectMethod(javaClass,
                javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jfloat>
{
    template<class... Parameters>
    inline static jfloat Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jfloat r = (jfloat) (GetEnv()->CallFloatMethod(javaObject, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jfloat CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jfloat r = (jfloat) (GetEnv()->CallStaticFloatMethod(javaClass, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jfloatArray>
{
    template<class... Parameters>
    inline static jfloatArray Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jfloatArray r = (jfloatArray) (GetEnv()->CallObjectMethod(javaObject, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jfloatArray CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jfloatArray r = (jfloatArray) (GetEnv()->CallObjectMethod(javaClass, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jdouble>
{
    template<class... Parameters>
    inline static jdouble Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jdouble r = (jdouble) (GetEnv()->CallDoubleMethod(javaObject, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jdouble CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jdouble r = (jdouble) (GetEnv()->CallStaticDoubleMethod(javaClass, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jdoubleArray>
{
    template<class... Parameters>
    inline static jdoubleArray Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jdoubleArray r = (jdoubleArray) (GetEnv()->CallObjectMethod(javaObject, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jdoubleArray CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jdoubleArray r = (jdoubleArray) (GetEnv()->CallObjectMethod(javaClass, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jlong>
{
    template<class... Parameters>
    inline static jlong Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jlong r = (jlong) (GetEnv()->CallLongMethod(javaObject, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jlong CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jlong r = (jlong) (GetEnv()->CallStaticLongMethod(javaClass, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jlongArray>
{
    template<class... Parameters>
    inline static jlongArray Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jlongArray r = (jlongArray) GetEnv()->CallObjectMethod(javaObject, javaMethod, std::forward<Parameters>(params)...);
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jlongArray CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jlongArray r = (jlongArray) (GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jboolean>
{
    template<class... Parameters>
    inline static jboolean Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jboolean r = (jboolean) (GetEnv()->CallBooleanMethod(javaObject, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jboolean CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jboolean r = (jboolean) (GetEnv()->CallStaticBooleanMethod(javaClass, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jbooleanArray>
{
    template<class... Parameters>
    inline static jbooleanArray Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jbooleanArray r = (jbooleanArray) (GetEnv()->CallObjectMethod(javaObject, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jbooleanArray CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jbooleanArray r = (jbooleanArray) (GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jobject>
{
    template<class... Parameters>
    inline static jobject Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jobject r = (jobject) (GetEnv()->CallObjectMethod(javaObject, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jobject CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jobject r = (jobject) (GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jobjectArray>
{
    template<class... Parameters>
    inline static jobjectArray Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallObjectMethod(javaObject, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jobjectArray CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jstring>
{
    template<class... Parameters>
    inline static jstring Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jstring r = (jstring) (GetEnv()->CallObjectMethod(javaObject, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jstring CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jstring r = (jstring) (GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jstringArray>
{
    template<class... Parameters>
    inline static jstringArray Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallObjectMethod(javaObject, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jstringArray CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jobjectArray r = (jobjectArray) (GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jbyte>
{
    template<class... Parameters>
    inline static jbyte Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jbyte r = (jbyte) (GetEnv()->CallBooleanMethod(javaObject, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jbyte CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jbyte r = (jbyte) (GetEnv()->CallStaticBooleanMethod(javaClass, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<> struct JniCall<jbyteArray>
{
    template<class... Parameters>
    inline static jbyteArray Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        jbyteArray r = (jbyteArray) (GetEnv()->CallObjectMethod(javaObject, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }

    template<class... Parameters>
    inline static jbyteArray CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        jbyteArray r = (jbyteArray) (GetEnv()->CallStaticObjectMethod(javaClass, javaMethod, std::forward<Parameters>(params)...));
        DAVA_JNI_EXCEPTION_CHECK
        return r;
    }
};

template<class Ret, class... Parameters>
class MethodCaller
{
public:
    inline static Ret Call(jobject javaObject, jmethodID javaMethod, Parameters... params)
    {
        return JniCall<Ret>::Call(javaObject, javaMethod, std::forward<Parameters>(params)...);
    }

    inline static Ret CallStatic(jclass javaClass, jmethodID javaMethod, Parameters... params)
    {
        return JniCall<Ret>::CallStatic(javaClass, javaMethod, std::forward<Parameters>(params)...);
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

    template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
    Function<Ret(jobject, P1, P2, P3, P4, P5, P6)> GetMethod(String name) const;

    template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
    Function<Ret(P1, P2, P3, P4, P5, P6)> GetStaticMethod(String name) const;

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
    return Bind(&MethodCaller<Ret>::Call, _1, javaMethod);
}

template<class Ret>
Function<Ret(void)> JavaClass::GetStaticMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret>();
    jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller<Ret>::CallStatic, javaClass, javaMethod);
}

template<class Ret, class P1>
Function<Ret(jobject, P1)> JavaClass::GetMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1>();
    jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller<Ret, P1>::Call, _1, javaMethod, _2);
}

template<class Ret, class P1>
Function<Ret(P1)> JavaClass::GetStaticMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1>();
    jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller<Ret, P1>::CallStatic, javaClass, javaMethod, _1);
}

template<class Ret, class P1, class P2>
Function<Ret(jobject, P1, P2)> JavaClass::GetMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1, P2>();
    jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller<Ret, P1, P2>::Call, _1, javaMethod, _2, _3);
}

template<class Ret, class P1, class P2>
Function<Ret(P1, P2)> JavaClass::GetStaticMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1, P2>();
    jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller<Ret, P1, P2>::CallStatic, javaClass, javaMethod,
            _1, _2);
}

template<class Ret, class P1, class P2, class P3>
Function<Ret(jobject, P1, P2, P3)> JavaClass::GetMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1, P2, P3>();
    jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller<Ret, P1, P2, P3>::Call, _1, javaMethod, _2, _3,
            _4);
}

template<class Ret, class P1, class P2, class P3>
Function<Ret(P1, P2, P3)> JavaClass::GetStaticMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1, P2, P3>();
    jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller<Ret, P1, P2, P3>::CallStatic, javaClass,
            javaMethod, _1, _2, _3);
}

template<class Ret, class P1, class P2, class P3, class P4>
Function<Ret(jobject, P1, P2, P3, P4)> JavaClass::GetMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1, P2, P3, P4>();
    jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller<Ret, P1, P2, P3, P4>::Call, _1, javaMethod, _2,
            _3, _4, _5);
}

template<class Ret, class P1, class P2, class P3, class P4>
Function<Ret(P1, P2, P3, P4)> JavaClass::GetStaticMethod(String name) const
{
    String parametersString = SignatureString::FromTypes<Ret, P1, P2, P3, P4>();
    jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller<Ret, P1, P2, P3, P4>::CallStatic, javaClass,
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
    return Bind(&MethodCaller<Ret, P1, P2, P3, P4, P5>::Call, _1, javaMethod,
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
    return Bind(&MethodCaller<Ret, P1, P2, P3, P4, P5>::CallStatic, javaClass,
            javaMethod, _1, _2, _3, _4, _5);
}

template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
Function<Ret(jobject, P1, P2, P3, P4, P5, P6)> JavaClass::GetMethod(String name) const
{
    String parametersString =
            SignatureString::FromTypes<Ret, P1, P2, P3, P4, P5, P6>();
    jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller<Ret, P1, P2, P3, P4, P5, P6>::Call, _1,
            javaMethod, _1, _2, _3, _4, _5, _6);
}
template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
Function<Ret(P1, P2, P3, P4, P5, P6)> JavaClass::GetStaticMethod(String name) const
{
    String parametersString =
            SignatureString::FromTypes<Ret, P1, P2, P3, P4, P5, P6>();
    jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(),
            parametersString.c_str());
    DAVA_JNI_EXCEPTION_CHECK
    return Bind(&MethodCaller<Ret, P1, P2, P3, P4, P5, P6>::CallStatic, javaClass,
            javaMethod, _1, _2, _3, _4, _5, _6);
}

} // end namespace JNI
} // end namespace DAVA

#pragma clang diagnostic pop

#endif

#endif
