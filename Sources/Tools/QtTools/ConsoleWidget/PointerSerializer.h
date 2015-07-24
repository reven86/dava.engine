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


#ifndef __QT_TOOLS_POINTER_SERIALIZER_H__
#define __QT_TOOLS_POINTER_SERIALIZER_H__

#include <string>
#include <vector>
#include <sstream>

class PointerSerializer
{
public:
    PointerSerializer() = default;
    PointerSerializer(const PointerSerializer &converter) = default;
    PointerSerializer(PointerSerializer &&converter);
    static PointerSerializer ParseString(const std::string &str);
    static const char* GetRegex();
    const std::string& GetText() const;
    PointerSerializer& operator = (const PointerSerializer &result) = default;
    PointerSerializer& operator = (PointerSerializer &&result);
    
    template <typename T>
    std::vector<T> GetPointers() const;
    template <typename T>
    PointerSerializer(const T pointer_);
    template <typename Container>
    PointerSerializer(const Container &cont);

    PointerSerializer(const std::string &str);

    template <typename T>
    static std::string FromPointer(const T pointer_);
    template <typename Container>
    static std::string FromPointerList(Container &&cont);
    
    template <typename T>
    bool CanConvert() const;
    bool IsValid() const;
private:
    std::vector<void*> pointers;

    std::string typeName;
    std::string text;
};

inline const std::string& PointerSerializer::GetText() const
{
    return text;
}

template <typename T>
std::vector<T> PointerSerializer::GetPointers() const
{
    static_assert(std::is_pointer<T>::value, "works only for vector of pointers");
    std::vector<T>  returnVec(pointers.size());
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
PointerSerializer::PointerSerializer(const Container &cont)
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
std::string PointerSerializer::FromPointer(const T pointer_)
{
    std::stringstream ss;
    ss << "{"
        << typeid(pointer_).name()
        << " : "
        << static_cast<void*>(pointer_)
        << " }";

    return ss.str();
}

template <typename Container>
std::string PointerSerializer::FromPointerList(Container &&cont)
{
    using T = typename std::remove_reference<Container>::type::value_type;
    static_assert(std::is_pointer<T>::value, "works only for vector of pointers");
    std::stringstream ss;
    ss << "{"
        << typeid(T).name()
        << " : "
        << "[\n";

    auto it = std::cbegin(cont);
    auto begin_it = std::cbegin(cont);
    auto end_it = std::cend(cont);
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
