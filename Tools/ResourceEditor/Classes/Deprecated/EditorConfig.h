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

using namespace DAVA;

class PropertyDescription : public BaseObject
{
protected:
	~PropertyDescription(){}
public:
	PropertyDescription() : BaseObject(), type(0) {};

	String name;
	int32 type;
	VariantType defaultValue;
	Vector<String> comboValues;
    Vector<Color> colorListValues;
};

class EditorConfig: public Singleton<EditorConfig>
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

    void ParseConfig(const FilePath &filePath);

	const Vector<String> & GetProjectPropertyNames();
	const Vector<String> & GetComboPropertyValues(const String & nameStr);
    const Vector<Color> & GetColorPropertyValues(const String& nameStr);

	bool HasProperty(const String &propertyName);
	int32 GetPropertyValueType(const String &propertyName);
	VariantType *GetPropertyDefaultValue(const String &propertyName);

protected:
	void ClearConfig();

	PropertyDescription* GetPropertyDescription(const String &propertyName);

	int32 GetValueTypeFromPropertyType(int32 propertyType);
	int32 ParseType(const String &typeStr);

	Vector<String> propertyNames;
	Map<String, PropertyDescription*> properties;
	Vector<String> empty;
    Vector<Color> emptyColors;
};



#endif // __EDITOR_CONFIG_H__