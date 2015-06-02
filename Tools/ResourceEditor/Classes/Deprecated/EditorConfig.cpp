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


#include "EditorConfig.h"
#include "ControlsFactory.h"

EditorConfig::EditorConfig()
{
	empty.push_back("none");
}
    
EditorConfig::~EditorConfig()
{
	ClearConfig();
}

void EditorConfig::ClearConfig()
{
	propertyNames.clear();
	Map<String, PropertyDescription*>::iterator it = properties.begin();
	Map<String, PropertyDescription*>::iterator propEnd = properties.end();
	for(; it != propEnd; ++it)
	{
		SafeRelease(it->second);
	}
	properties.clear();
}

int32 EditorConfig::ParseType(const String &typeStr)
{
	if(typeStr == "Bool")
	{
		return PT_BOOL;
	}
	if(typeStr == "Int")
	{
		return PT_INT;
	}
	if(typeStr == "Float")
	{
		return PT_FLOAT;
	}

	if(typeStr == "String")
	{
		return PT_STRING;
	}
	if(typeStr == "Combobox")
	{
		return PT_COMBOBOX;
	}
    
    if(typeStr == "ColorList")
    {
        return PT_COLOR_LIST;
    }
	return PT_NONE;
}

void EditorConfig::ParseConfig(const FilePath &filePath)
{
	ClearConfig();

    YamlParser *parser = YamlParser::Create(filePath);
    if(parser)
    {
		YamlNode *rootNode = parser->GetRootNode();
		if(rootNode)
		{
			const Vector<YamlNode*> &yamlNodes = rootNode->AsVector();
			int32 propertiesCount = yamlNodes.size();
			for(int32 i = 0; i < propertiesCount; ++i)
			{
				YamlNode *propertyNode = yamlNodes[i];
				if(propertyNode)
				{
					const YamlNode *nameNode = propertyNode->Get("name");
					const YamlNode *typeNode = propertyNode->Get("type");
					const YamlNode *defaultNode = propertyNode->Get("default");
					if(nameNode && typeNode)
					{
						const String &nameStr = nameNode->AsString();
						const String &typeStr = typeNode->AsString();
						int32 type = ParseType(typeStr);
						if(type)
						{
							bool isOk = true;
							for(uint32 n = 0; n < propertyNames.size(); ++n)
							{
								if(propertyNames[n] == nameStr)
								{
									isOk = false;
									Logger::Error("EditorConfig::ParseConfig %s ERROR property %d property %s already exists", filePath.GetAbsolutePathname().c_str(), i, nameStr.c_str());
									break;
								}
							}

							if(isOk)
							{
								properties[nameStr] = new PropertyDescription();
								properties[nameStr]->name = nameStr;
								properties[nameStr]->type = type;
								switch(type)
								{
								case PT_BOOL:
									{
										bool defaultValue = false;
										if(defaultNode)
										{
											defaultValue = defaultNode->AsBool();
										}
										properties[nameStr]->defaultValue.SetBool(defaultValue);
									}
									break;
								case PT_INT:
									{
										int32 defaultValue = 0;
										if(defaultNode)
										{
											defaultValue = defaultNode->AsInt();
										}
										properties[nameStr]->defaultValue.SetInt32(defaultValue);
									}
									break;
								case PT_STRING:
									{
										String defaultValue;
										if(defaultNode)
										{
											defaultValue = defaultNode->AsString();
										}
										properties[nameStr]->defaultValue.SetString(defaultValue);
									}
									break;
								case PT_FLOAT:
									{
										float32 defaultValue = 0.0f;
										if(defaultNode)
										{
											defaultValue = defaultNode->AsFloat();
										}
										properties[nameStr]->defaultValue.SetFloat(defaultValue);
									}
									break;
								case PT_COMBOBOX:
									{
										int32 defaultValue = 0;
										if(defaultNode)
										{
											defaultValue = defaultNode->AsInt();
										}
										properties[nameStr]->defaultValue.SetInt32(defaultValue);

										const YamlNode *comboNode = propertyNode->Get("list");
										if(comboNode)
										{
											const Vector<YamlNode*> &comboValueNodes = comboNode->AsVector();
											int32 comboValuesCount = comboValueNodes.size();
											for(int32 i = 0; i < comboValuesCount; ++i)
											{
												properties[nameStr]->comboValues.push_back(comboValueNodes[i]->AsString());
											}
										}
									}
									break;
                                case PT_COLOR_LIST:
                                    {
                                        int32 defaultValue = 0;
                                        if(defaultNode)
                                        {
                                            defaultValue = defaultNode->AsInt();
                                        }
                                        properties[nameStr]->defaultValue.SetInt32(defaultValue);
                                        
                                        const YamlNode *colorListNode = propertyNode->Get("list");
                                        if(colorListNode)
                                        {
                                            const Vector<YamlNode*> &colorListNodes = colorListNode->AsVector();
                                            int32 colorListValuesCount = colorListNodes.size();
                                            for(int32 i = 0; i < colorListValuesCount; ++i)
                                            {
                                                const YamlNode* colorNode = colorListNodes[i];
                                                if(!colorNode || colorNode->GetCount() != 4)
                                                    continue;
                                                
                                                Color color(colorNode->Get(0)->AsFloat()/255.f,
                                                            colorNode->Get(1)->AsFloat()/255.f,
                                                            colorNode->Get(2)->AsFloat()/255.f,
                                                            colorNode->Get(3)->AsFloat()/255.f);

                                                properties[nameStr]->colorListValues.push_back(color);
                                            }
                                        }
                                    }
                                    break;
								}
								propertyNames.push_back(nameStr);
							} //isOk
						}
						else
						{
							Logger::Error("EditorConfig::ParseConfig %s ERROR property %d unknown type %s", filePath.GetAbsolutePathname().c_str(), i, typeStr.c_str());
						}
					}
					else
					{
						Logger::Error("EditorConfig::ParseConfig %s ERROR property %d type or name is missing", filePath.GetAbsolutePathname().c_str(), i);
					}
				}
				else
				{
					Logger::Error("EditorConfig::ParseConfig %s ERROR property %d is missing", filePath.GetAbsolutePathname().c_str(), i);
				}
			}
		} 
		// else file is empty - ok, no custom properties

		parser->Release();
	}
	// else file not found - ok, no custom properties
}

const Vector<String> &EditorConfig::GetProjectPropertyNames()
{
	return propertyNames;
}

const Vector<String> & EditorConfig::GetComboPropertyValues(const String & nameStr)
{
	if (properties.find(nameStr) != properties.end())
		return properties[nameStr]->comboValues;
	else
	{
		return empty;
	}
}

const Vector<Color> & EditorConfig::GetColorPropertyValues(const String& nameStr)
{
    if (properties.find(nameStr) != properties.end())
        return properties[nameStr]->colorListValues;
    else
        return emptyColors;
}

PropertyDescription* EditorConfig::GetPropertyDescription(const String &propertyName)
{
	Map<String, PropertyDescription*>::iterator findIt = properties.find(propertyName);
	if(findIt != properties.end())
	{
		return findIt->second;
	}
	return NULL;
}

bool EditorConfig::HasProperty(const String &propertyName)
{
	return (GetPropertyDescription(propertyName) != NULL);
}

int32 EditorConfig::GetValueTypeFromPropertyType(int32 propertyType)
{
	int32 type = VariantType::TYPE_NONE;
	switch(propertyType)
	{
	case PT_BOOL:
		type = VariantType::TYPE_BOOLEAN;
		break;
	case PT_INT:
	case PT_COMBOBOX:
    case PT_COLOR_LIST:
		type = VariantType::TYPE_INT32;
		break;
	case PT_STRING:
		type = VariantType::TYPE_STRING;
		break;
	case PT_FLOAT:
		type = VariantType::TYPE_FLOAT;
		break;
	}
	return type;
}

int32 EditorConfig::GetPropertyValueType(const String &propertyName)
{
	int32 type = VariantType::TYPE_NONE;
	PropertyDescription *propertyDescription = GetPropertyDescription(propertyName);
	if(propertyDescription)
	{
		type = GetValueTypeFromPropertyType(propertyDescription->type);
	}
	return type;
}

VariantType *EditorConfig::GetPropertyDefaultValue(const String &propertyName)
{
	VariantType *defaultValue = NULL;
	PropertyDescription *propertyDescription = GetPropertyDescription(propertyName);
	if(propertyDescription)
	{
		defaultValue = &propertyDescription->defaultValue;
	}
	return defaultValue;
}
