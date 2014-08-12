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
class YamlNode;
class VariantType;
class YamlNodeKeyValuePair;
    
/**
	\defgroup yaml Yaml configs
 */


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