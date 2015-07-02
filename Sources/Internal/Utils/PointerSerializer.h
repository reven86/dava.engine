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


#ifndef __DAVAENGINE_JSON_CONVERTER_H_
#define __DAVAENGINE_JSON_CONVERTER_H_

#include "Base/BaseTypes.h"

namespace
{
    template <typename T> struct is_ref_to_ptr
    {
        static const bool value = false;
    };

    template <typename T> struct is_ref_to_ptr<T*&>
    {
        static const bool value = true;
    };
}

namespace DAVA
{

class PointerSerializer
{
public:
    PointerSerializer() = default;
    PointerSerializer(const PointerSerializer &converter) = default;
    PointerSerializer(PointerSerializer &&converter);
    static PointerSerializer ParseString(const String &str);
    static const char* GetRegex();
    PointerSerializer& operator = (const PointerSerializer &result) = default;
    PointerSerializer& operator = (PointerSerializer &&result);
    
    template <typename T>
    Vector<T> GetPointers() const
    {
        static_assert(std::is_pointer<T>::value, "works only for vector of pointers");
        Vector<T>  returnVec(pointers.size());
        for (auto ptr : pointers)
        {
            returnVec.push_back(reinterpret_cast<T>(ptr));
        }
        return returnVec;
    }
    template <typename T>
    PointerSerializer(const T pointer_)
    {
        static_assert(std::is_pointer<T>::value, "works only for vector of pointers");
        typeName = typeid(pointer_).name();
        pointers.push_back(static_cast<void*>(pointer_));
        text = FromPointer(pointer_);
    }
    template <typename Container>
    PointerSerializer(const Container &cont)
    {
        typeName = typeid(*(cont.begin())).name();
        for (auto pointer : cont)
        {
            pointers.push_back(static_cast<void*>(pointer));
        }
        text = FromPointerList(cont);
    }

    PointerSerializer(const String &str)
        : PointerSerializer(ParseString(str))
    {
    }

    template <typename T>
    static String FromPointer(const T pointer_)
    {
        DAVA::StringStream ss;
        ss << "{"
            << typeid(pointer_).name()
            << " : "
            << static_cast<void*>(pointer_)
            <<" }";
        return ss.str();
    }
    template <typename Container>
    static String FromPointerList(Container &&cont)
    {
        static_assert(std::is_pointer<decltype(*(cont.begin()))>::value, "works only for vector of pointers");
        DAVA::StringStream ss;
        ss << "{"
            << typeid(*(cont.begin())).name()
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
    bool CanConvert() const
    {
        return typeid(T).name() == typeName;
    }
    bool IsValid() const
    {
        return !typeName.empty();
    }
private:
    Vector<void*> pointers;

    String typeName;
    String text;
};
}
#endif // __DAVAENGINE_JSON_CONVERTER_H_
