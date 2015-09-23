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


#include "QtTools/ConsoleWidget/PointerSerializer.h"
#include <regex>
#include <sstream>


PointerSerializer::PointerSerializer(PointerSerializer&& converter)
    : pointers(std::move(converter.pointers))
    , typeName(std::move(converter.typeName))    
    , text(std::move(converter.text))
{

}

PointerSerializer::PointerSerializer(const DAVA::String &str)
    : PointerSerializer(ParseString(str))
{
}

PointerSerializer PointerSerializer::ParseString(const DAVA::String &str)
{
    std::regex rgx(GetRegex());
    std::smatch sm;
    DAVA::String::const_iterator cit = str.cbegin();

    while (std::regex_search(cit, str.cend(), sm, rgx)) 
    {
        if (sm.size() == 3) // original text, left and righ
        {
            DAVA::String data = sm[2];
            std::smatch sm2;
            DAVA::String::const_iterator cit2 = data.begin();
            std::regex rgx2("0?[xX]?[0-9a-fA-F]+");
            DAVA::Vector<void*> pointers;
            while (regex_search(cit2, data.cend(), sm2, rgx2))
            {
                for (auto m : sm2)
                {
                    DAVA::StringStream ssout(m);
                    DAVA::pointer_size px;
                    ssout >> std::hex >> px; // on MAC std::stringstream >> void* truncate address to 3 bytes, even if this sstream created from valid void*;
                    void *ptr;
                    ptr = reinterpret_cast<void*>(px);
                    pointers.push_back(ptr);
                }
                cit2 = sm2[0].second;
            }
            if (!pointers.empty())
            {
                PointerSerializer converter;
                converter.text = sm[0];
                DAVA::String type = sm[1];
                while (!type.empty() && ::isspace(type.back()))
                {
                    type.pop_back();
                }
                converter.typeName = type;
                converter.pointers = std::move(pointers);
                return converter;
            }
        }
        cit = sm[0].second;
    }
    return PointerSerializer();
}

const char* PointerSerializer::GetRegex()
{
    return R"(\{\s*([\:\s\w\*\&]+)\s*\:\s*([\,\[\]\w\s]*)\s*\})";
}

PointerSerializer& PointerSerializer::operator = (PointerSerializer&& converter)
{
    if (this != &converter)
    {
        pointers = std::move(converter.pointers);
        typeName = std::move(converter.typeName);
        text = std::move(converter.text);
    }
    return *this;
}

DAVA::String PointerSerializer::CleanUpString(const DAVA::String& input)
{
	return std::regex_replace(input, std::regex(GetRegex()), std::string());
}
