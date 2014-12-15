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

namespace DAVA
{

namespace JNI
{
enum JMRetType
{
	VOID,
	STRING,
	STRING_ARR,
	BOOLEAN,
	BOOLEAN_ARR,
	INT,
	INT_ARR,
	OBJECT,
	OBJECT_ARR,
};

JavaVM *GetJVM();
JNIEnv *GetEnv();

template<typename T>
struct TypeName
{ };

template<> struct TypeName<void>
{
	operator const char *() const {return value.c_str();}
	operator String() const {return value;}

	String value = "V";
	JMRetType type = VOID;
};
template<> struct TypeName<jint>
{
	operator const char *() const {return value.c_str();}
	operator String() const {return value;}

	String value = "I";
	JMRetType type = INT;
};
template<> struct TypeName<jintArray>
{
	operator const char *() const {return value.c_str();}
	operator String() const {return value;}

	String value = "[I";
	JMRetType type = INT_ARR;
};
template<> struct TypeName<jstring>
{
	operator const char *() const {return value.c_str();}
	operator String() const {return value;}

	String value = "Ljava/lang/String;";
	JMRetType type = STRING;
};
template<> struct TypeName<jobject>
{
	operator const char *() const {return value.c_str();}
	operator String() const {return value;}

	String value = "Ljava/lang/Object;";
	JMRetType type = OBJECT;
};
template<> struct TypeName<jobjectArray>
{
	operator const char *() const {return value.c_str();}
	operator String() const {return value;}

	String value = "[Ljava/lang/Object;";
	JMRetType type = OBJECT_ARR;
};

template<> struct TypeName<jboolean>
{
	operator const char *() const {return value.c_str();}
	operator String() const {return value;}

	String value = "Z";
	JMRetType type = BOOLEAN;
};
template<> struct TypeName<jbooleanArray>
{
	operator const char *() const {return value.c_str();}
	operator String() const {return value;}

	String value = "[Z";
	JMRetType type = BOOLEAN_ARR;
};

class SpecString
{
public:
	template<class T>
	static String FromTypes();

	template<class Ret, class P1>
	static String FromTypes();

	template<class Ret, class P1, class P2>
	static String FromTypes();

	template<class Ret, class P1, class P2, class P3>
	static String FromTypes();

	template<class Ret, class P1, class P2, class P3, class P4>
	static String FromTypes();

	template<class Ret, class P1, class P2, class P3, class P4, class P5>
	static String FromTypes();

	template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
	static String FromTypes();
};

template<class Ret>
String SpecString::FromTypes()
{
	return String("(V)")
			+ TypeName<Ret>();
}

template<class Ret, class P1>
String SpecString::FromTypes()
{
	return String("(")
			+ TypeName<P1>().value
			+ String(")")
			+ TypeName<Ret>().value;
}

template<class Ret, class P1, class P2>
String SpecString::FromTypes()
{
	return String("(")
			+ TypeName<P1>().value
			+ TypeName<P2>().value
			+ String(")")
			+ TypeName<Ret>().value;
}

template<class Ret, class P1, class P2, class P3>
String SpecString::FromTypes()
{
	return String("(")
			+ TypeName<P1>().value
			+ TypeName<P2>().value
			+ TypeName<P3>().value
			+ String(")")
			+ TypeName<Ret>().value;
}
template<class Ret, class P1, class P2, class P3, class P4>
String SpecString::FromTypes()
{
	return String("(")
			+ TypeName<P1>().value
			+ TypeName<P2>().value
			+ TypeName<P3>().value
			+ TypeName<P4>().value
			+ String(")")
			+ TypeName<Ret>().value;
}

template<class Ret, class P1, class P2, class P3, class P4, class P5>
String SpecString::FromTypes()
{
	return String("(")
			+ TypeName<P1>().value
			+ TypeName<P2>().value
			+ TypeName<P3>().value
			+ TypeName<P4>().value
			+ TypeName<P5>().value
			+ String(")")
			+ TypeName<Ret>().value;
}

template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
String SpecString::FromTypes()
{
	return String("(")
			+ TypeName<P1>().value
			+ TypeName<P2>().value
			+ TypeName<P3>().value
			+ TypeName<P4>().value
			+ TypeName<P5>().value
			+ TypeName<P6>().value
			+ String(")")
			+ TypeName<Ret>().value;
}

/*
	JAVA Class caller implementations!
*/

template<class T>
class MethodCaller
{

};

// all possible function calls in one switch
#define ResolvedCall(Static, javaClass, javaMethod, ...)\
switch(TypeName<Ret>().type)\
{\
	case VOID: GetEnv()->Call##Static##VoidMethod(javaClass, javaMethod, __VA_ARGS__); return Ret();\
	case OBJECT:return (Ret)(GetEnv()->Call##Static##ObjectMethod(javaClass, javaMethod, __VA_ARGS__));\
	case OBJECT_ARR: return (Ret)(GetEnv()->Call##Static##ObjectMethod(javaClass, javaMethod, __VA_ARGS__));\
	case INT: return (Ret)(GetEnv()->Call##Static##IntMethod(javaClass, javaMethod, __VA_ARGS__));\
	case INT_ARR: return (Ret)(GetEnv()->Call##Static##IntMethod(javaClass, javaMethod, __VA_ARGS__));\
	case BOOLEAN: return (Ret)(GetEnv()->Call##Static##BooleanMethod(javaClass, javaMethod, __VA_ARGS__));\
	case BOOLEAN_ARR: return (Ret)(GetEnv()->Call##Static##BooleanMethod(javaClass, javaMethod, __VA_ARGS__));\
}

template<class Ret, class P1>
class MethodCaller1
{
public:
	static Ret Call(jclass javaClass, jmethodID javaMethod, bool isStatic, P1 p1)
	{
		if (isStatic)
			ResolvedCall(Static, javaClass, javaMethod, p1)
		else
			ResolvedCall(, javaClass, javaMethod, p1)
	}

};

template<class Ret, class P1, class P2>
class MethodCaller2
{
public:
	static Ret Call(jclass javaClass, jmethodID javaMethod, bool isStatic, P1 p1, P2 p2)
	{
		if (isStatic)
			ResolvedCall(Static, javaClass, javaMethod, p1, p2)
		else
			ResolvedCall(, javaClass, javaMethod, p1, p2)
	}

};

template<class Ret, class P1, class P2, class P3>
class MethodCaller3
{
public:
	static Ret Call(jclass javaClass, jmethodID javaMethod, bool isStatic, P1 p1, P2 p2, P3 p3)
	{
		if (isStatic)
			ResolvedCall(Static, javaClass, javaMethod, p1, p2, p3)
		else
			ResolvedCall(, javaClass, javaMethod, p1, p2, p3)
	}

};

template<class Ret, class P1, class P2, class P3, class P4>
class MethodCaller4
{
public:
	static Ret Call(jclass javaClass, jmethodID javaMethod, bool isStatic, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		if (isStatic)
			ResolvedCall(Static, javaClass, javaMethod, p1, p2, p3, p4)
		else
			ResolvedCall(, javaClass, javaMethod, p1, p2, p3, p4)
	}

};

template<class Ret, class P1, class P2, class P3, class P4, class P5>
class MethodCaller5
{
public:
	static Ret Call(jclass javaClass, jmethodID javaMethod, bool isStatic, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		if (isStatic)
			ResolvedCall(Static, javaClass, javaMethod, p1, p2, p3, p4, p5)
		else
			ResolvedCall(, javaClass, javaMethod, p1, p2, p3, p4, p5)
	}

};

template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
class MethodCaller6
{
public:
	static Ret Call(jclass javaClass, jmethodID javaMethod, bool isStatic, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
	{
		if (isStatic)
			ResolvedCall(Static, javaClass, javaMethod, p1, p2, p3, p4, p5, p6)
		else
			ResolvedCall(, javaClass, javaMethod, p1, p2, p3, p4, p5, p6)
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

    template<class Ret, class P1>
    Function<Ret (P1)> GetMethod(String name);

    template<class Ret, class P1>
    Function<Ret (P1)> GetStaticMethod(String name);

    template<class Ret, class P1, class P2>
    Function<Ret (P1, P2)> GetMethod(String name);

    template<class Ret, class P1, class P2>
    Function<Ret (P1, P2)> GetStaticMethod(String name);

    template<class Ret, class P1, class P2, class P3>
    Function<Ret (P1, P2, P3)> GetMethod(String name);

    template<class Ret, class P1, class P2, class P3>
	Function<Ret (P1, P2, P3)> GetStaticMethod(String name);

    template<class Ret, class P1, class P2, class P3, class P4>
    Function<Ret (P1, P2, P3, P4)> GetMethod(String name);

    template<class Ret, class P1, class P2, class P3, class P4>
	Function<Ret (P1, P2, P3, P4)> GetStaticMethod(String name);

    template<class Ret, class P1, class P2, class P3, class P4, class P5>
    Function<Ret (P1, P2, P3, P4, P5)> GetMethod(String name);

    template<class Ret, class P1, class P2, class P3, class P4, class P5>
	Function<Ret (P1, P2, P3, P4, P5)> GetStaticMethod(String name);

    template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
    Function<Ret (P1, P2, P3, P4, P5, P6)> GetMethod(String name);

    template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
	Function<Ret (P1, P2, P3, P4, P5, P6)> GetStaticMethod(String name);

private:
    JavaVM *jvm;
    jclass javaClass;
    String name;
};

inline JavaClass::operator jclass() const
{
	return javaClass;
}

template<class Ret, class P1>
Function<Ret (P1)> JavaClass::GetMethod(String name)
{
	String parametersString = SpecString::FromTypes<Ret, P1>();
	jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(), parametersString.c_str());
	return Bind(&MethodCaller1<Ret, P1>::Call, javaClass, javaMethod, false, _1);
}

template<class Ret, class P1>
Function<Ret (P1)> JavaClass::GetStaticMethod(String name)
{
	String parametersString = SpecString::FromTypes<Ret, P1>();
	jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(), parametersString.c_str());
	return Bind(&MethodCaller1<Ret, P1>::Call, javaClass, javaMethod, true, _1);
}

template<class Ret, class P1, class P2>
Function<Ret (P1, P2)> JavaClass::GetMethod(String name)
{
	String parametersString = SpecString::FromTypes<Ret, P1, P2>();
	jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(), parametersString.c_str());
	return Bind(&MethodCaller2<Ret, P1, P2>::Call, javaClass, javaMethod, false, _1, _2);
}

template<class Ret, class P1, class P2>
Function<Ret (P1, P2)> JavaClass::GetStaticMethod(String name)
{
	String parametersString = SpecString::FromTypes<Ret, P1, P2>();
	jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(), parametersString.c_str());
	return Bind(&MethodCaller2<Ret, P1, P2>::Call, javaClass, javaMethod, true, _1, _2);
}

template<class Ret, class P1, class P2, class P3>
Function<Ret (P1, P2, P3)> JavaClass::GetMethod(String name)
{
	String parametersString = SpecString::FromTypes<Ret, P1, P2, P3>();
	jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(), parametersString.c_str());
	return Bind(&MethodCaller3<Ret, P1, P2, P3>::Call, javaClass, javaMethod, false, _1, _2, _3);
}

template<class Ret, class P1, class P2, class P3>
Function<Ret (P1, P2, P3)> JavaClass::GetStaticMethod(String name)
{
	String parametersString = SpecString::FromTypes<Ret, P1, P2, P3>();
	jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(), parametersString.c_str());
	return Bind(&MethodCaller3<Ret, P1, P2, P3>::Call, javaClass, javaMethod, true, _1, _2, _3);
}

template<class Ret, class P1, class P2, class P3, class P4>
Function<Ret (P1, P2, P3, P4)> JavaClass::GetMethod(String name)
{
	String parametersString = SpecString::FromTypes<Ret, P1, P2, P3, P4>();
	jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(), parametersString.c_str());
	return Bind(&MethodCaller4<Ret, P1, P2, P3, P4>::Call, javaClass, javaMethod, false, _1, _2, _3, _4);
}

template<class Ret, class P1, class P2, class P3, class P4>
Function<Ret (P1, P2, P3, P4)> JavaClass::GetStaticMethod(String name)
{
	String parametersString = SpecString::FromTypes<Ret, P1, P2, P3, P4>();
	jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(), parametersString.c_str());
	return Bind(&MethodCaller4<Ret, P1, P2, P3, P4>::Call, javaClass, javaMethod, true, _1, _2, _3, _4);
}

template<class Ret, class P1, class P2, class P3, class P4, class P5>
Function<Ret (P1, P2, P3, P4, P5)> JavaClass::GetMethod(String name)
{
	String parametersString = SpecString::FromTypes<Ret, P1, P2, P3, P4, P5>();
	jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(), parametersString.c_str());
	return Bind(&MethodCaller5<Ret, P1, P2, P3, P4, P5>::Call, javaClass, javaMethod, false, _1, _2, _3, _4, _5);
}

template<class Ret, class P1, class P2, class P3, class P4, class P5>
Function<Ret (P1, P2, P3, P4, P5)> JavaClass::GetStaticMethod(String name)
{
	String parametersString = SpecString::FromTypes<Ret, P1, P2, P3, P4, P5>();
	jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(), parametersString.c_str());
	return Bind(&MethodCaller5<Ret, P1, P2, P3, P4, P5>::Call, javaClass, javaMethod, true, _1, _2, _3, _4, _5);
}

template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
Function<Ret (P1, P2, P3, P4, P5, P6)> JavaClass::GetMethod(String name)
{
	String parametersString = SpecString::FromTypes<Ret, P1, P2, P3, P4, P5, P6>();
	jmethodID javaMethod = GetEnv()->GetMethodID(javaClass, name.c_str(), parametersString.c_str());
	return Bind(&MethodCaller6<Ret, P1, P2, P3, P4, P5, P6>::Call, javaClass, javaMethod, false, _1, _2, _3, _4, _5, _6);
}

template<class Ret, class P1, class P2, class P3, class P4, class P5, class P6>
Function<Ret (P1, P2, P3, P4, P5, P6)> JavaClass::GetStaticMethod(String name)
{
	String parametersString = SpecString::FromTypes<Ret, P1, P2, P3, P4, P5, P6>();
	jmethodID javaMethod = GetEnv()->GetStaticMethodID(javaClass, name.c_str(), parametersString.c_str());
	return Bind(&MethodCaller6<Ret, P1, P2, P3, P4, P5, P6>::Call, javaClass, javaMethod, true, _1, _2, _3, _4, _5, _6);
}


}
}

#endif

#endif
