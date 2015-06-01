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


#include "FileSystem/XMLParser.h"


#include "libxml/parser.h"
#include "libxml/xmlstring.h"

namespace DAVA 
{
	XMLParser::XMLParser()
	{

	}

	XMLParser::~XMLParser()
	{

	}

	bool XMLParser::ParseFile(const FilePath &fileName, XMLParserDelegate *delegateptr)
	{
// 		Logger::FrameworkDebug("[XMLParser::ParseFile] fileName = %s", fileName.c_str());
// 		Logger::FrameworkDebug("[XMLParser::ParseFile] delegateptr = %p", delegateptr);

		bool retValue = false;
		File *xmlFile = File::Create(fileName, File::OPEN | File::READ);
		if(xmlFile)
		{
			int32 dataSize = xmlFile->GetSize();
//			Logger::FrameworkDebug("[XMLParser::ParseFile] dataSize = %d", dataSize);

			uint8 *data = new uint8[dataSize];
			if(data)
			{
				int32 readBytes = xmlFile->Read(data, dataSize);
//				Logger::FrameworkDebug("[XMLParser::ParseFile] readBytes = %d", readBytes);
				if(readBytes == dataSize)
				{
					retValue = XMLParser::ParseBytes(data, dataSize, delegateptr);
				}
				else
				{
					Logger::Error("[XMLParser::ParseFile] readBytes != dataSize");
				}

				//TODO: VK: need to delete?
				SafeDeleteArray(data);
			}
			else
			{
				Logger::Error("[XMLParser::ParseFile] can't allocate data");
			}

			//TODO: VK: need to delete?
			SafeRelease(xmlFile);
		}
		else
		{
            Logger::Error("[XMLParser::ParseFile] can't create file %s", fileName.GetStringValue().c_str());
		}

//		Logger::FrameworkDebug("[XMLParser::ParseFile] retValue = %d", retValue);
		return retValue;
	}

	bool XMLParser::ParseBytes(const unsigned char *bytes, int length, XMLParserDelegate *delegateptr)
	{
//		Logger::FrameworkDebug("[XMLParser::ParseBytes] delegateptr = %p", delegateptr);

		bool retValue = false;
		
		xmlSAXHandler saxHandler = {0};
		saxHandler.startDocument = XMLParser::StartDocument;
		saxHandler.endDocument = XMLParser::EndDocument;
		saxHandler.startElement = XMLParser::StartElement;
		saxHandler.endElement = XMLParser::EndElement;
		saxHandler.characters = XMLParser::Characters;

		int32 retCode = xmlSAXUserParseMemory( &saxHandler, (void*)delegateptr, (char *)bytes, length);
//		Logger::FrameworkDebug("[XMLParser::ParseBytes] retCode = %d", retCode);
		if(0 <= retCode)
		{
			retValue = true;
		}

//		Logger::FrameworkDebug("[XMLParser::ParseBytes] retValue = %d", retValue);
		return retValue;
	}


	void XMLParser::StartDocument(void *user_data)
	{
//		Logger::FrameworkDebug("[XMLParser::StartDocument] user_data = %p", user_data);
		XMLParserDelegate *delegateptr = (XMLParserDelegate *)user_data;
		if(delegateptr)
		{

		}
	}
	void XMLParser::EndDocument(void *user_data)
	{
//		Logger::FrameworkDebug("[XMLParser::EndDocument] user_data = %p", user_data);
		XMLParserDelegate *delegateptr = (XMLParserDelegate *)user_data;
		if(delegateptr)
		{

		}
	}

	void XMLParser::Characters(void *user_data, const xmlChar *ch, int len)
	{
//		Logger::FrameworkDebug("[XMLParser::Characters] user_data = %p, len = %d", user_data, len);
		XMLParserDelegate *delegateptr = (XMLParserDelegate *)user_data;
		if(delegateptr)
		{
			//char *content = new char[len + 1];
			String s((char *)ch, len);

// 			delegateptr->OnFoundCharacters(content);
			delegateptr->OnFoundCharacters(s);

			//SafeDeleteArray(content);
		}
	}

	void XMLParser::StartElement(void *user_data, const xmlChar *name, const xmlChar **attrs)
	{
//		Logger::FrameworkDebug("[XMLParser::StartElement] %s, user_data = %p", name, user_data);
		XMLParserDelegate *delegateptr = (XMLParserDelegate *)user_data;
		if(delegateptr)
		{
			Map<String, String> attributes;

			if(attrs)
			{
//				Logger::FrameworkDebug("[XMLParser::StartElement] attrs in");

				int32 i = 0;
				while(attrs[i])
				{
					char *str = (attrs[i+1]) ? (char *)attrs[i+1] : (char *)"";
					attributes[(char *)attrs[i]] = str;

//					Logger::FrameworkDebug("[XMLParser::StartElement] %s = %s", attrs[i], str);

					i += 2;
				}

//				Logger::FrameworkDebug("[XMLParser::StartElement] attrs out");
			}

			delegateptr->OnElementStarted((char *)name, "", "", attributes);
		}
	}

	void XMLParser::EndElement(void *user_data, const xmlChar *name)
	{
//		Logger::FrameworkDebug("[XMLParser::EndElement] %s", name);
		XMLParserDelegate *delegateptr = (XMLParserDelegate *)user_data;
		if(delegateptr)
		{
			delegateptr->OnElementEnded((char *)name, "", "");
		}
	}

// 	xmlEntityPtr XMLParser::GetEntity(void *user_data, const xmlChar *name)
// 	{
// 		Logger::FrameworkDebug("[XMLParser::GetEntity] %s", name);
// 		XMLParserDelegate *delegate = (XMLParserDelegate *)user_data;
// 		if(delegate)
// 		{
// 
// 		}
// 	}

};
