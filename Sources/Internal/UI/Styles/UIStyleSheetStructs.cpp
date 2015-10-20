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

#include "UI/Styles/UIStyleSheetStructs.h"

#include "Utils/Utils.h"

namespace DAVA
{

void UIStyleSheetClassSet::AddClass(const FastName& clazz)
{
    auto it = std::find_if(classes.begin(), classes.end(), [&clazz](UIStyleSheetClass& cl) {
        return cl.clazz == clazz && !cl.tag.IsValid();
    });
    
    if (it == classes.end())
    {
        classes.push_back(UIStyleSheetClass(FastName(), clazz));
    }
}

void UIStyleSheetClassSet::RemoveClass(const FastName& clazz)
{
    auto it = std::find_if(classes.begin(), classes.end(), [&clazz](UIStyleSheetClass& cl) {
        return cl.clazz == clazz && !cl.tag.IsValid();
    });
    
    if (it != classes.end())
    {
        *it = classes.back();
        classes.pop_back();
    }
}

bool UIStyleSheetClassSet::HasClass(const FastName& clazz) const
{
    auto it = std::find_if(classes.begin(), classes.end(), [&clazz](UIStyleSheetClass& cl) {
        return cl.clazz == clazz;
    });
    
    return it != classes.end();
}

void UIStyleSheetClassSet::SetTaggedClass(const FastName& tag, const FastName& clazz)
{
    auto it = std::find_if(classes.begin(), classes.end(), [&tag](UIStyleSheetClass& cl) {
        return cl.tag == tag;
    });
    
    if (it != classes.end())
    {
        it->clazz = clazz;
    }
    else
    {
        classes.push_back(UIStyleSheetClass(tag, clazz));
    }
}

void UIStyleSheetClassSet::ResetTaggedClass(const FastName& tag)
{
    auto it = std::find_if(classes.begin(), classes.end(), [&tag](UIStyleSheetClass& cl) {
        return cl.tag == tag;
    });
    
    if (it != classes.end())
    {
        *it = classes.back();
        classes.pop_back();
    }
}

void UIStyleSheetClassSet::RemoveAllClasses()
{
    classes.clear();
}

String UIStyleSheetClassSet::GetClassesAsString() const
{
    String result;
    for (size_t i = 0; i < classes.size(); i++)
    {
        if (i != 0)
        {
            result += " ";
        }
        if (classes[i].tag.IsValid())
        {
            result += classes[i].tag.c_str();
            result += "=";
        }
        
        result += classes[i].clazz.c_str();
    }
    return result;
}

void UIStyleSheetClassSet::SetClassesFromString(const String &classesStr)
{
    Vector<String> tokens;
    Split(classesStr, " ", tokens);
    
    classes.clear();
    for (String &token : tokens)
    {
        Vector<String> pair;
        Split(token, "=", pair);
        if (pair.size() > 1)
        {
            classes.push_back(UIStyleSheetClass(FastName(pair[0]), FastName(pair[1])));
        }
        else if (pair.size() > 0)
        {
            classes.push_back(UIStyleSheetClass(FastName(), FastName(pair[0])));
        }
    }
    
}
}
