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


#ifndef __DAVAENGINE_YAML_DOM_PARSER_H__
#define __DAVAENGINE_YAML_DOM_PARSER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include <stack>
#include "FileSystem/File.h"
#include "Base/FastName.h"

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

	// Representation Type. For example, lists can be represented as ["val1", "val2", "val3"] or
	//- val1
	//- val2
	//- val3
	// See also DF-2389 for details.
	enum eRepresentationType
	{
		
		REPRESENT_AS_DEFAULT,
		REPRESENT_ARRAY_AS_SINGLE_LINE,
		REPRESENT_ARRAY_AS_MULTI_LINE
	};

    // Predefined node name to store Relative Depth.
    static const char8* SAVE_INDEX_NAME;

protected:
	virtual ~YamlNode();
public:
	YamlNode(eType type, eRepresentationType repType = REPRESENT_AS_DEFAULT);

	
	void Print(int32 identation);
    void PrintToFile(DAVA::File* file, uint32 identationDepth = 0) const;
	
	bool			AsBool() const;
	int32			AsInt() const;//left for old code
    int32			AsInt32() const;
    uint32			AsUInt32() const;
    int64			AsInt64() const;
    uint64			AsUInt64() const;
	float32			AsFloat() const;
	const String &	AsString() const;
	const WideString & AsWString() const;
	const Vector<YamlNode*> & AsVector() const;
    const MultiMap<String, YamlNode*> & AsMap() const;
    FastName AsFastName() const;
	
	/*
		These functions work only if type of node is array
		All values must be integer or float to perform this conversion
	 */
	Vector2			AsPoint() const;//Dizz: this one exists cause of Boroda
	Vector2			AsVector2() const;
	Vector3			AsVector3() const;
  	Vector4			AsVector4() const;
	Color			AsColor() const;
	Rect			AsRect() const;
    VariantType     AsVariantType() const;
	
	const YamlNode *		Get(const String & name) const;
	const YamlNode *		Get(int32 index) const;
	const String &	GetItemKeyName(int32 index) const;
	
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
    
	eType			GetType() const { return type; }
	int32			GetCount() const;
	bool			IsWideString() const { return isWideString; };

    void            InitFromKeyedArchive(KeyedArchive* archive);
    void            InitFromVariantType(VariantType* varType);
    
protected:
    void            FillContentAccordingToVariantTypeValue(VariantType* varType);
    void            ProcessMatrix(const float32* array,uint32 dimension);
    void            ProcessVector(const float32 array[],uint32 dimension);
    bool            IsContainingMap() const;
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
	eRepresentationType		representationType;
	WideString				stringValue;
	String					 nwStringValue;
	Vector<YamlNode*>		 objectArray;
    MultiMap<String, YamlNode*>	objectMap;
	bool					isWideString;
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

public:
    // This method just creates the YAML parser.
    static YamlParser   * Create();

    // This method creates the parser and parses the input file.
    static YamlParser * Create(const FilePath & fileName)
    {
        return YamlParser::CreateAndParse(fileName);
    }

    // This method creates the parser and parses the data string.
    static YamlParser * CreateAndParseString(const String & data)
    {
        return YamlParser::CreateAndParse(data);
    }

    // Save to YAML file. "saveLevelOneMapsOnly" is a special mode where
    // root node is skipped and only nodes with YamlNode::TYPE_MAP are saved
    // on a root level.
    // TODO! replace this flag to the separate method!
	bool SaveToYamlFile(const FilePath & fileName, const YamlNode * rootNode, bool saveLevelOneMapsOnly, uint32 attr = File::CREATE | File::WRITE);
    
	// Save the strings list (needed for Localization).
	bool SaveStringsList(const FilePath & fileName, YamlNode * rootNode, uint32 attr = File::CREATE | File::WRITE);

	// Get the root node.
	YamlNode			* GetRootNode() const;
	
	struct YamlDataHolder
	{
		uint32 fileSize;
		uint32 dataOffset;
		uint8 * data;
	};

protected:
    template<typename T> static YamlParser * CreateAndParse(const T & data)
    {
        YamlParser * parser = new YamlParser();
        if (parser)
        {
            bool parseResult = parser->Parse(data);
            if(!parseResult)
            {
                SafeRelease(parser);
                return 0;
            }
        }
        return parser;
    }

    bool Parse(const String & fileName);
    bool Parse(const FilePath & fileName);
    bool Parse(YamlDataHolder * dataHolder);

private:
	YamlNode			* GetNodeByPath(const String & path);
	bool                SaveNodeRecursive(File* fileToSave, const String& nodeName, const YamlNode* currentNode, int16 depth) const;

    // Order the YAML node with type "Map" according to the depth.
    void OrderMapYamlNode(const MultiMap<String, YamlNode*>& mapNodes, Vector<YamlNodeKeyValuePair> &sortedChildren ) const;

    // Write different Yaml node types to the file.
    bool WriteScalarNodeToYamlFile(File* fileToSave, const String& nodeName, const YamlNode* currentNode, int16 depth) const;
    bool WriteArrayNodeToYamlFile(File* fileToSave, const String& nodeName,
                                  const YamlNode* currentNode, int16 depth) const;
    bool WriteMapNodeToYamlFile(File* fileToSave, const String& mapNodeName, int16 depth) const;

    bool WriteStringToYamlFile(File* fileToSave, const String& stringToWrite) const;
	bool WriteStringToYamlFile(File* fileToSave, const WideString& stringToWrite) const;

	bool WriteStringListNodeToYamlFie(File* fileToSave, const String& nodeName, const YamlNode* currentNode) const;

    // Recursively get the array node representation string.
    String GetArrayNodeRepresentation(const String& nodeName, const YamlNode* currentNode, int16 depth, bool writeAsOuterNode = true) const;
	String GetArrayNodeRepresentationMultiline(const String& nodeName, const YamlNode* currentNode, int16 depth) const;

    // Return the identation string of the appropriate depth.
    String PrepareIdentedString(int16 depth) const;

	// Replace \n characters to "\n" string.
	WideString ReplaceLineEndings(const WideString& rawString) const;

	YamlNode			* rootObject;
	String				lastMapKey;
	
	Stack<YamlNode *> objectStack;
};

};

#endif // __DAVAENGINE_JSON_PARSER_H__