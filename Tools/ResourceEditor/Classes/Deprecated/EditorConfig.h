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


#ifndef __EDITOR_CONFIG_H__
#define __EDITOR_CONFIG_H__

#include "DAVAEngine.h"

class PropertyDescription : public DAVA::BaseObject
{
protected:
    ~PropertyDescription()
    {
    }

public:
    PropertyDescription()
        : DAVA::BaseObject()
        , type(0){};

    DAVA::String name;
    DAVA::int32 type;
    DAVA::VariantType defaultValue;
    DAVA::Vector<DAVA::String> comboValues;
    DAVA::Vector<DAVA::Color> colorListValues;
};

class EditorConfig : public DAVA::Singleton<EditorConfig>
{
public:
    EditorConfig();
    virtual ~EditorConfig();

    enum ePropertyType
    {
        PT_NONE = 0,
        PT_BOOL,
        PT_INT,
        PT_FLOAT,
        PT_STRING,
        PT_COMBOBOX,
        PT_COLOR_LIST,

        PROPERTY_TYPES_COUNT
    };

    void ParseConfig(const DAVA::FilePath& filePath);

    const DAVA::Vector<DAVA::String>& GetProjectPropertyNames();
    const DAVA::Vector<DAVA::String>& GetComboPropertyValues(const DAVA::String& nameStr);
    const DAVA::Vector<DAVA::Color>& GetColorPropertyValues(const DAVA::String& nameStr);

    bool HasProperty(const DAVA::String& propertyName);
    DAVA::int32 GetPropertyValueType(const DAVA::String& propertyName);
    DAVA::VariantType* GetPropertyDefaultValue(const DAVA::String& propertyName);

protected:
    void ClearConfig();

    PropertyDescription* GetPropertyDescription(const DAVA::String& propertyName);

    DAVA::int32 GetValueTypeFromPropertyType(DAVA::int32 propertyType);
    DAVA::int32 ParseType(const DAVA::String& typeStr);

    DAVA::Vector<DAVA::String> propertyNames;
    DAVA::Map<DAVA::String, PropertyDescription*> properties;
    DAVA::Vector<DAVA::String> empty;
    DAVA::Vector<DAVA::Color> emptyColors;
};



#endif // __EDITOR_CONFIG_H__