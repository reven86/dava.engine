#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

namespace DAVA
{
namespace JNI
{
////////  TypeSignature  /////////////////////////////////////////////////
template <typename T, typename = void>
struct TypeSignature;

template <>
struct TypeSignature<void>
{
    static const char8* value()
    {
        return "V";
    }
};
template <>
struct TypeSignature<jboolean>
{
    static const char8* value()
    {
        return "Z";
    }
};
template <>
struct TypeSignature<jbyte>
{
    static const char8* value()
    {
        return "B";
    }
};
template <>
struct TypeSignature<jchar>
{
    static const char8* value()
    {
        return "C";
    }
};
template <>
struct TypeSignature<jshort>
{
    static const char8* value()
    {
        return "S";
    }
};
template <>
struct TypeSignature<jint>
{
    static const char8* value()
    {
        return "I";
    }
};
template <>
struct TypeSignature<jlong>
{
    static const char8* value()
    {
        return "J";
    }
};
template <>
struct TypeSignature<jfloat>
{
    static const char8* value()
    {
        return "F";
    }
};
template <>
struct TypeSignature<jdouble>
{
    static const char8* value()
    {
        return "D";
    }
};

template <>
struct TypeSignature<jobject>
{
    static const char8* value()
    {
        return "Ljava/lang/Object;";
    }
};
template <>
struct TypeSignature<jstring>
{
    static const char8* value()
    {
        return "Ljava/lang/String;";
    }
};
template <>
struct TypeSignature<jarray>
{
    static const char8* value()
    {
        return "Ljava/lang/Object;";
    }
};
template <>
struct TypeSignature<jobjectArray>
{
    static const char8* value()
    {
        return "[Ljava/lang/Object;";
    }
};
template <>
struct TypeSignature<jbooleanArray>
{
    static const char8* value()
    {
        return "[Ljava/lang/Object;";
    }
};
template <>
struct TypeSignature<jbyteArray>
{
    static const char8* value()
    {
        return "[Ljava/lang/Object;";
    }
};
template <>
struct TypeSignature<jcharArray>
{
    static const char8* value()
    {
        return "[Ljava/lang/Object;";
    }
};
template <>
struct TypeSignature<jshortArray>
{
    static const char8* value()
    {
        return "[Ljava/lang/Object;";
    }
};
template <>
struct TypeSignature<jintArray>
{
    static const char8* value()
    {
        return "[Ljava/lang/Object;";
    }
};
template <>
struct TypeSignature<jlongArray>
{
    static const char8* value()
    {
        return "[Ljava/lang/Object;";
    }
};
template <>
struct TypeSignature<jfloatArray>
{
    static const char8* value()
    {
        return "[Ljava/lang/Object;";
    }
};
template <>
struct TypeSignature<jdoubleArray>
{
    static const char8* value()
    {
        return "[Ljava/lang/Object;";
    }
};

template <typename...>
struct BuildTypeSignature;

template <typename T, typename... Tail>
struct BuildTypeSignature<T, Tail...>
{
    static String value(String s)
    {
        s += TypeSignature<T>::value();
        s = BuildTypeSignature<Tail...>::value(s);
        return s;
    }
};

template <>
struct BuildTypeSignature<>
{
    static String value(String s)
    {
        return s;
    }
};

template <typename R, typename... Args>
struct TypeSignature<R(Args...)>
{
    static const char8* value()
    {
        static String v(String("(") + BuildTypeSignature<Args...>::value(String()) + String(")") + TypeSignature<R>::value());
        return v.c_str();
    }
};

////////  TypedMethod  ///////////////////////////////////////////////////
template <typename R, typename = void>
struct TypedMethod;

template <typename R>
struct TypedMethod<R, std::enable_if_t<std::is_base_of<std::remove_pointer_t<jobject>, std::remove_pointer_t<R>>::value>>
{
    static constexpr auto Call = &JNIEnv::CallObjectMethod;
    static constexpr auto CallStatic = &JNIEnv::CallStaticObjectMethod;
};

template <>
struct TypedMethod<void>
{
    static constexpr auto Call = &JNIEnv::CallVoidMethod;
    static constexpr auto CallStatic = &JNIEnv::CallStaticVoidMethod;
};

template <>
struct TypedMethod<jboolean>
{
    static constexpr auto Call = &JNIEnv::CallBooleanMethod;
    static constexpr auto CallStatic = &JNIEnv::CallStaticBooleanMethod;
};

template <>
struct TypedMethod<jbyte>
{
    static constexpr auto Call = &JNIEnv::CallByteMethod;
    static constexpr auto CallStatic = &JNIEnv::CallStaticByteMethod;
};

template <>
struct TypedMethod<jchar>
{
    static constexpr auto Call = &JNIEnv::CallCharMethod;
    static constexpr auto CallStatic = &JNIEnv::CallStaticCharMethod;
};

template <>
struct TypedMethod<jshort>
{
    static constexpr auto Call = &JNIEnv::CallShortMethod;
    static constexpr auto CallStatic = &JNIEnv::CallStaticShortMethod;
};

template <>
struct TypedMethod<jint>
{
    static constexpr auto Call = &JNIEnv::CallIntMethod;
    static constexpr auto CallStatic = &JNIEnv::CallStaticIntMethod;
};

template <>
struct TypedMethod<jlong>
{
    static constexpr auto Call = &JNIEnv::CallLongMethod;
    static constexpr auto CallStatic = &JNIEnv::CallStaticLongMethod;
};

template <>
struct TypedMethod<jfloat>
{
    static constexpr auto Call = &JNIEnv::CallFloatMethod;
    static constexpr auto CallStatic = &JNIEnv::CallStaticFloatMethod;
};

template <>
struct TypedMethod<jdouble>
{
    static constexpr auto Call = &JNIEnv::CallDoubleMethod;
    static constexpr auto CallStatic = &JNIEnv::CallStaticDoubleMethod;
};

} // namespace JNI
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__
