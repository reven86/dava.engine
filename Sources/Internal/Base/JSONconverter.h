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

class JSONconverter
{
public:
    JSONconverter();
    JSONconverter(const JSONconverter &converter) = default;
    JSONconverter(JSONconverter &&converter);
    template <typename T>
    Vector<typename std::enable_if<std::is_pointer<T>::value, T>::type> GetPointers() const
    {
        Vector<T>  returnVec;
        for (auto ptr : pointers)
        {
            returnVec.push_back(reinterpret_cast<T>(ptr));
        }
        return returnVec;
    }
    template <typename T>
    JSONconverter(const typename std::enable_if<std::is_pointer<T>::value, T>::type pointer_)
    {
        typeName = typeid(pointer_).name();
        pointers.push_back(static_cast<void*>(pointer_));
        text = FromPointer(pointer_);
    }
    template <typename Container>
    JSONconverter(const Container &cont)
    {
        typeName = typeid(*(cont.begin())).name();
        for (auto pointer : cont)
        {
            pointers.push_back(static_cast<void*>(pointer));
        }
        text = FromPointerList(cont);
    }
    template <>
    JSONconverter(const DAVA::String &str)
        : JSONconverter(ParseString(str))
    {

    }

    static JSONconverter ParseString(const DAVA::String &str);

    JSONconverter& operator = (const JSONconverter &result) = default;
    JSONconverter& operator = (JSONconverter &&result);

    template <typename T>
    static DAVA::String FromPointer(const typename std::enable_if<std::is_pointer<T>::value, T>::type pointer_)
    {
        DAVA::String text = "{";
        text += typeid(pointer_).name();
        text += " : ";
        std::stringstream ss;
        ss << static_cast<void*>(pointer_);
        text += ss.str();
        text += "\n}";
        return text;
    }
    template <typename Container>
    static DAVA::String FromPointerList(Container &&cont)
    {
        typename std::enable_if<is_ref_to_ptr<decltype(*(cont.begin()))>::value>::type; //dummy for compilation error. You must ust Container of pointers to solve it
        DAVA::String text = "{";
        text += typeid(*(cont.begin())).name();
        text += " : ";
        text += "[\n";
        std::stringstream ss;
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
        text += ss.str();
        text += "\n]";
        text += "\n}";
        return text;
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
    DAVA::Vector<void*> pointers;

    DAVA::String typeName;
    DAVA::String text;
};
}
#endif // __DAVAENGINE_JSON_CONVERTER_H_
