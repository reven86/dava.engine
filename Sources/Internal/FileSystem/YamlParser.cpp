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


#include "FileSystem/YamlParser.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/Logger.h"
#include "Utils/Utils.h"
#include "yaml/yaml.h"


namespace DAVA 
{

bool YamlParser::Parse(const String& data)
{
    YamlDataHolder dataHolder;
    dataHolder.fileSize = static_cast<uint32>(data.size());
    dataHolder.data = (uint8 *)data.c_str();
    dataHolder.dataOffset = 0;    

    return Parse(&dataHolder);
}

bool YamlParser::Parse(const FilePath & pathName)
{
    File * yamlFile = File::Create(pathName, File::OPEN | File::READ);
    if (!yamlFile)
    {
        Logger::Error("[YamlParser::Parse] Can't create file: %s", pathName.GetAbsolutePathname().c_str());
        return false;
    }

    YamlDataHolder dataHolder;
    dataHolder.fileSize = yamlFile->GetSize();
    dataHolder.data = new uint8[dataHolder.fileSize];
    dataHolder.dataOffset = 0;
    yamlFile->Read(dataHolder.data, dataHolder.fileSize);
    SafeRelease(yamlFile);

    bool result = Parse(&dataHolder);
    SafeDeleteArray(dataHolder.data);
    return result;
}

bool YamlParser::Parse(YamlDataHolder * dataHolder)
{
	yaml_parser_t parser;
	yaml_event_t event;
	
	int done = 0;
	
	/* Create the Parser object. */
	yaml_parser_initialize(&parser);
	
	yaml_parser_set_encoding(&parser, YAML_UTF8_ENCODING);
	
	/* Set a string input. */
	//yaml_parser_set_input_string(&parser, (const unsigned char*)pathName.c_str(), pathName.length());
		
	yaml_parser_set_input(&parser, read_handler, dataHolder);

	String lastMapKey;
	bool isKeyPresent = false;

	/* Read the event sequence. */
	while (!done) 
	{
		
		/* Get the next event. */
		if (!yaml_parser_parse(&parser, &event))
		{
			Logger::Error("[YamlParser::Parse] error: type: %d %s line: %d pos: %d", parser.error, parser.problem, parser.problem_mark.line, parser.problem_mark.column);
			break;
		}
		
		switch(event.type)
		{
		case YAML_ALIAS_EVENT:
			Logger::FrameworkDebug("[YamlParser::Parse] alias: %s", event.data.alias.anchor);
			break;
		
		case YAML_SCALAR_EVENT:
			{
				String scalarValue = (const char*)event.data.scalar.value;

				if (objectStack.empty())
				{
					YamlNode * node = YamlNode::CreateStringNode();
					node->Set(scalarValue);
					rootObject = node;
				}
				else
				{
					YamlNode * topContainer = objectStack.top();
					DVASSERT(topContainer->GetType() != YamlNode::TYPE_STRING);
					if (topContainer->GetType() == YamlNode::TYPE_MAP)
					{
						if (!isKeyPresent)
						{
							lastMapKey = scalarValue;
						}
						else
						{
							topContainer->Add(lastMapKey, scalarValue);
						}
						isKeyPresent = !isKeyPresent;
					}
					else if (topContainer->GetType() == YamlNode::TYPE_ARRAY)
					{
						topContainer->Add(scalarValue);
					}
				}
			}
			break;
		
		case YAML_DOCUMENT_START_EVENT:
			//Logger::FrameworkDebug("document start:");
			break;
		
		case YAML_DOCUMENT_END_EVENT:
			//Logger::FrameworkDebug("document end:");
			break;

		case YAML_SEQUENCE_START_EVENT:
			{
				YamlNode * node = YamlNode::CreateArrayNode();
				if (objectStack.empty())
				{
					rootObject = node;
				}
				else
				{
					YamlNode * topContainer = objectStack.top();
					DVASSERT(topContainer->GetType() != YamlNode::TYPE_STRING);
					if (topContainer->GetType() == YamlNode::TYPE_MAP)
					{
						DVASSERT(isKeyPresent);
						topContainer->AddNodeToMap(lastMapKey, node);
						isKeyPresent = false;
					}
					else if (topContainer->GetType() == YamlNode::TYPE_ARRAY)
					{
						topContainer->AddNodeToArray(node);
					}
				}
				objectStack.push(node);
			}break;
				
		case YAML_SEQUENCE_END_EVENT:
			{
				objectStack.pop();
			}break;
		
		case YAML_MAPPING_START_EVENT:
			{
				YamlNode * node = YamlNode::CreateMapNode();
				if (objectStack.empty())
				{
					rootObject = node;
				}
				else
				{
					YamlNode * topContainer = objectStack.top();
					if (topContainer->GetType() == YamlNode::TYPE_MAP)
					{
						DVASSERT(isKeyPresent);
						topContainer->AddNodeToMap(lastMapKey, node);
						isKeyPresent = false;
					}
					else if (topContainer->GetType() == YamlNode::TYPE_ARRAY)
					{
						topContainer->AddNodeToArray(node);
					}
				}
				objectStack.push(node);
			}
			break;
				
		case YAML_MAPPING_END_EVENT:
			{
				objectStack.pop();
			}
			break;
        default:
            break;
		};

		/* Are we finished? */
		done = (event.type == YAML_STREAM_END_EVENT);
		
		/* The application is responsible for destroying the event object. */
		yaml_event_delete(&event);
		
	}

	/* Destroy the Parser object. */
	yaml_parser_delete(&parser);

    DVASSERT(objectStack.size() == 0);

    return objectStack.empty();
}

YamlParser::YamlParser()
{
	rootObject = 0;
}

YamlParser::~YamlParser()
{
	SafeRelease(rootObject);
}
	
YamlNode * YamlParser::GetRootNode() const
{
	return rootObject;
}

}