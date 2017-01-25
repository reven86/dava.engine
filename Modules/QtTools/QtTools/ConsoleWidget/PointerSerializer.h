#ifndef __QT_TOOLS_POINTER_SERIALIZER_H__
#define __QT_TOOLS_POINTER_SERIALIZER_H__

#include "Base/BaseTypes.h"

class PointerSerializer
{
public:
    PointerSerializer() = default;
    PointerSerializer(const PointerSerializer& converter) = default;
    PointerSerializer(PointerSerializer&& converter);
    template <typename T>
    PointerSerializer(const T pointer_);
    template <typename Container>
    PointerSerializer(const Container& cont);
    PointerSerializer(const DAVA::String& str);

    static PointerSerializer ParseString(const DAVA::String& str);
    template <typename T>
    bool CanConvert() const;
    bool IsValid() const;

    static const char* GetRegex();
    const DAVA::String& GetText() const;
    template <typename T>
    DAVA::Vector<T> GetPointers() const;

    PointerSerializer& operator=(const PointerSerializer& result) = default;
    PointerSerializer& operator=(PointerSerializer&& result);

    template <typename T>
    static DAVA::String FromPointer(const T pointer_);
    template <typename Container>
    static DAVA::String FromPointerList(Container&& cont);

    static DAVA::String CleanUpString(const DAVA::String& input);

private:
    DAVA::Vector<void*> pointers;

    DAVA::String typeName;
    DAVA::String text;
};

inline const DAVA::String& PointerSerializer::GetText() const
{
    return text;
}

template <typename T>
DAVA::Vector<T> PointerSerializer::GetPointers() const
{
    static_assert(std::is_pointer<T>::value, "works only for vector of pointers");
    DAVA::Vector<T> returnVec;
    returnVec.reserve(pointers.size());
    for (auto ptr : pointers)
    {
        returnVec.push_back(static_cast<T>(ptr));
    }
    return returnVec;
}

template <typename T>
PointerSerializer::PointerSerializer(const T pointer_)
{
    static_assert(std::is_pointer<T>::value, "works only for vector of pointers");
    typeName = typeid(pointer_).name();
    pointers.push_back(static_cast<void*>(pointer_));
    text = FromPointer(pointer_);
}

template <typename Container>
PointerSerializer::PointerSerializer(const Container& cont)
{
    using T = typename std::remove_reference<Container>::type::value_type;
    static_assert(std::is_pointer<T>::value, "works only for vector of pointers");
    typeName = typeid(T).name();
    for (auto pointer : cont)
    {
        pointers.push_back(static_cast<void*>(pointer));
    }
    text = FromPointerList(cont);
}

template <typename T>
DAVA::String PointerSerializer::FromPointer(const T pointer_)
{
    DAVA::StringStream ss;
    ss << "{"
       << typeid(pointer_).name()
       << " : "
       << static_cast<void*>(pointer_)
       << " }";

    return ss.str();
}

template <typename Container>
DAVA::String PointerSerializer::FromPointerList(Container&& cont)
{
    using T = typename std::remove_reference<Container>::type::value_type;
    static_assert(std::is_pointer<T>::value, "works only for vector of pointers");
    DAVA::StringStream ss;
    ss << "{"
       << typeid(T).name()
       << " : "
       << "[\n";

    auto it = std::begin(cont);
    auto begin_it = std::begin(cont);
    auto end_it = std::end(cont);
    while (it != end_it)
    {
        if (it != begin_it)
        {
            ss << ",\n";
        }
        ss << static_cast<void*>(*it);
        ++it;
    }
    ss << "\n]"
       << "\n}";

    return ss.str();
}

template <typename T>
inline bool PointerSerializer::CanConvert() const
{
    return typeid(T).name() == typeName;
}

inline bool PointerSerializer::IsValid() const
{
    return !typeName.empty();
}
#endif // __QT_TOOLS_POINTER_SERIALIZER_H__
