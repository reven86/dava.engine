/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#ifndef __DAVAENGINE_YAML_DOM_PARSER_H__
#define __DAVAENGINE_YAML_DOM_PARSER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include <stack>
#include "FileSystem/File.h"

namespace DAVA 
{
class YamlParser;
class VariantType;
class YamlNodeKeyValuePair;
    
/**
	\defgroup yaml Yaml configs
 */
/** 
	\ingroup yaml
	\brief this class is base yaml node that is used for everything connected with yaml
*/
class YamlNode : public BaseObject
{
public:
	enum eType
	{
		TYPE_STRING = 0,
		TYPE_ARRAY,
		TYPE_MAP,
	};

    // Predefined node name to store Relative Depth.
    static const char8* YAML_NODE_RELATIVE_DEPTH_NAME;

	YamlNode(eType type);
	virtual ~YamlNode();
	
	void Print(int32 identation);
    void PrintToFile(DAVA::File* file, uint32 identationDepth = 0);
	
	bool			AsBool();
	int32			AsInt();//left for old code
    int32			AsInt32();
    uint32			AsUInt32();
    int64			AsInt64();
    uint64			AsUInt64();
	float32			AsFloat();
	const String &	AsString();
	const WideString & AsWString();
	Vector<YamlNode*> & AsVector();
    MultiMap<String, YamlNode*> & AsMap();
	
	/*
		These functions work only if type of node is array
		All values must be integer or float to perform this conversion
	 */
	Vector2			AsPoint();//Dizz: this one exists cause of Boroda
	Vector2			AsVector2();
	Vector3			AsVector3();
  	Vector4			AsVector4();
	Color			AsColor();
	Rect			AsRect();	
    VariantType     AsVariantType();
	
	YamlNode *		Get(const String & name);
	YamlNode *		Get(int32 index); 
	const String &	GetItemKeyName(int32 index); 
	
	// "Adders". These methods ADD node to the map, even in case the node with the same name is added.
    void            Add(const String& name, bool value);
    void            Add(const String& name, int32 value);
    void            Add(const String& name, float32 value);

    void            Add(const String& name, const char8* value);
    void            Add(const String& name, const String& value);
    void            Add(const String& name, const WideString& value);

    void            Add(const String& name, const Vector2& value);
    void            Add(const String& name, const Vector3& value);
	void            Add(const String& name, const Vector4& value);

    void            Add(const String& name, VariantType* varType);

	// Specific adder for the whole node.
	void            AddNodeToMap(const String& name, YamlNode* node);


    // "Setters". These methods REPLACE node in the map in case the node with the same name exists.
    void            Set(const String& name, bool value);
    void            Set(const String& name, int32 value);
    void            Set(const String& name, float32 value);

    void            Set(const String& name, const char8* value);
    void            Set(const String& name, const String& value);
    void            Set(const String& name, const WideString& value);

    void            Set(const String& name, const Vector2& value);
    void            Set(const String& name, const Vector3& value);
    void            Set(const String& name, const Vector4& value);

    void            Set(const String& name, VariantType* varType);

	// Specific setter for the whole node.
	void            SetNodeToMap(const String& name, YamlNode* node);

    // Setters for Map/Array nodes.
    void            AddNodeToArray(YamlNode* node);

    // Add the values to the current node of type Array.
    void            AddValueToArray(int32 value);
    void            AddValueToArray(float32 value);
    void            AddValueToArray(const String& value);
    void            AddValueToArray(const Vector2& value);
    void            AddValueToArray(const Vector3& value);
    void            AddValueToArray(const Vector4& value);
    void            AddValueToArray(VariantType* value);
    
    // Remove node value from map
    void            RemoveNodeFromMap(const String & name);
    
	eType			GetType() { return type; }
	int32			GetCount();

    void            InitFromKeyedArchive(KeyedArchive* archive);
    void            InitFromVariantType(VariantType* varType);
    
protected:
    void            FillContentAccordingToVariantTypeValue(VariantType* varType);
    void            ProcessMatrix(const float32* array,uint32 dimension);
    void            ProcessVector(const float32 array[],uint32 dimension);
    bool            IsContainingMap();
    String          FloatToCuttedString(float f);
    
	// Internal setters, which can both add or replace value in the map.
    void            InternalSet(const String& name, bool value, bool rewritePreviousValue);
    void            InternalSet(const String& name, int32 value, bool rewritePreviousValue);
    void            InternalSet(const String& name, float32 value, bool rewritePreviousValue);
    void            InternalSet(const String& name, const char8* value, bool rewritePreviousValue);
    void            InternalSet(const String& name, const String& value, bool rewritePreviousValue);
    void            InternalSet(const String& name, const WideString& value, bool rewritePreviousValue);
    void            InternalSet(const String& name, const Vector2& value, bool rewritePreviousValue);
    void            InternalSet(const String& name, const Vector3& value, bool rewritePreviousValue);
    void            InternalSet(const String& name, const Vector4& value, bool rewritePreviousValue);
    void            InternalSet(const String& name, VariantType* varType, bool rewritePreviousValue);
	
    void            InternalSetNodeToMap(const String& name, YamlNode* node, bool rewritePreviousValue);

private:
    
	int						mapIndex;
	int						mapCount;
	eType					type;
	WideString				stringValue;
	String					 nwStringValue;
	Vector<YamlNode*>		 objectArray;
    MultiMap<String, YamlNode*>	objectMap;
	friend class YamlParser;
};

/** 
	\ingroup yaml
	\brief this class is yaml parser and it used if you want to parse yaml file
 */
class YamlParser : public BaseObject
{
protected:
	YamlParser();
	virtual ~YamlParser();

	bool Parse(const FilePath & fileName);
	
public:
    // This method just creates the YAML parser.
    static YamlParser   * Create();
    
    // This method creates the parser and parses the input file.
	static YamlParser	* Create(const FilePath & fileName);
	
    // Save to YAML file.
	bool SaveToYamlFile(const FilePath & fileName, YamlNode * rootNode, bool skipRootNode, uint32 attr = File::CREATE | File::WRITE);
    
	// Save the strings list (needed for Localization).
	bool SaveStringsList(const FilePath & fileName, YamlNode * rootNode, uint32 attr = File::CREATE | File::WRITE);

	// Get the root node.
	YamlNode			* GetRootNode();
	
	struct YamlDataHolder
	{
		uint32 fileSize;
		uint32 dataOffset;
		uint8 * data;
	};

private:
	YamlNode			* GetNodeByPath(const String & path);
	bool                SaveNodeRecursive(File* fileToSave, const String& nodeName, YamlNode* currentNode, int16 depth);

    // Order the YAML node with type "Map" according to the depth.
    Vector<YamlNodeKeyValuePair> OrderMapYamlNode(const MultiMap<String, YamlNode*>& mapNodes);

    // Write different Yaml node types to the file.
    bool WriteScalarNodeToYamlFile(File* fileToSave, const String& nodeName, const String& nodeValue, int16 depth);
    bool WriteArrayNodeToYamlFile(File* fileToSave, const String& nodeName,
                                  YamlNode* currentNode, int16 depth);
    bool WriteMapNodeToYamlFile(File* fileToSave, const String& mapNodeName, int16 depth);

    bool WriteStringToYamlFile(File* fileToSave, const String& stringToWrite);
	bool WriteStringToYamlFile(File* fileToSave, const WideString& stringToWrite);

	bool WriteStringListNodeToYamlFie(File* fileToSave, const String& nodeName, YamlNode* currentNode);

    // Recursively get the array node representation string.
    String GetArrayNodeRepresentation(const String& nodeName, YamlNode* currentNode, int16 depth, bool writeAsOuterNode = true);

    // Return the identation string of the appropriate depth.
    String PrepareIdentedString(int16 depth);

	// Replace \n characters to "\n" string.
	WideString ReplaceLineEndings(const WideString& rawString) const;

	YamlNode			* rootObject;
	String				lastMapKey;
	
	Stack<YamlNode *> objectStack;

	YamlDataHolder dataHolder;
};

};

#endif // __DAVAENGINE_JSON_PARSER_H__