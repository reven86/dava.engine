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

bool XMLParser::ParseFile(const FilePath& fileName, XMLParserDelegate* delegateptr)
{
    // 		Logger::FrameworkDebug("[XMLParser::ParseFile] fileName = %s", fileName.c_str());
    // 		Logger::FrameworkDebug("[XMLParser::ParseFile] delegateptr = %p", delegateptr);

    bool retValue = false;
    File* xmlFile = File::Create(fileName, File::OPEN | File::READ);
    if (xmlFile)
    {
        uint64 dataSize = xmlFile->GetSize();
        //			Logger::FrameworkDebug("[XMLParser::ParseFile] dataSize = %d", dataSize);

        uint8* data = new uint8[static_cast<size_t>(dataSize)];
        if (data)
        {
            uint32 readBytes = xmlFile->Read(data, static_cast<uint32>(dataSize));
            //				Logger::FrameworkDebug("[XMLParser::ParseFile] readBytes = %d", readBytes);
            if (readBytes == dataSize)
            {
                retValue = XMLParser::ParseBytes(data, static_cast<uint32>(dataSize), delegateptr);
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
        Logger::Error("[XMLParser::ParseFile] can't Open file %s for read", fileName.GetStringValue().c_str());
    }

    //		Logger::FrameworkDebug("[XMLParser::ParseFile] retValue = %d", retValue);
    return retValue;
}

bool XMLParser::ParseBytes(const unsigned char* bytes, int length, XMLParserDelegate* delegateptr)
{
    //		Logger::FrameworkDebug("[XMLParser::ParseBytes] delegateptr = %p", delegateptr);

    bool retValue = false;

    xmlSAXHandler saxHandler = { 0 };
    saxHandler.startDocument = XMLParser::StartDocument;
    saxHandler.endDocument = XMLParser::EndDocument;
    saxHandler.startElement = XMLParser::StartElement;
    saxHandler.endElement = XMLParser::EndElement;
    saxHandler.characters = XMLParser::Characters;

    int32 retCode = xmlSAXUserParseMemory(&saxHandler, reinterpret_cast<void*>(delegateptr), reinterpret_cast<const char*>(bytes), length);
    //		Logger::FrameworkDebug("[XMLParser::ParseBytes] retCode = %d", retCode);
    if (0 <= retCode)
    {
        retValue = true;
    }

    //		Logger::FrameworkDebug("[XMLParser::ParseBytes] retValue = %d", retValue);
    return retValue;
}

void XMLParser::StartDocument(void* user_data)
{
    //		Logger::FrameworkDebug("[XMLParser::StartDocument] user_data = %p", user_data);
    XMLParserDelegate* delegateptr = reinterpret_cast<XMLParserDelegate*>(user_data);
    if (delegateptr)
    {
    }
}
void XMLParser::EndDocument(void* user_data)
{
    //		Logger::FrameworkDebug("[XMLParser::EndDocument] user_data = %p", user_data);
    XMLParserDelegate* delegateptr = reinterpret_cast<XMLParserDelegate*>(user_data);
    if (delegateptr)
    {
    }
}

void XMLParser::Characters(void* user_data, const xmlChar* ch, int len)
{
    //		Logger::FrameworkDebug("[XMLParser::Characters] user_data = %p, len = %d", user_data, len);
    XMLParserDelegate* delegateptr = reinterpret_cast<XMLParserDelegate*>(user_data);
    if (delegateptr)
    {
        //char *content = new char[len + 1];
        String s(reinterpret_cast<const char*>(ch), len);

        // 			delegateptr->OnFoundCharacters(content);
        delegateptr->OnFoundCharacters(s);

        //SafeDeleteArray(content);
    }
}

void XMLParser::StartElement(void* user_data, const xmlChar* name, const xmlChar** attrs)
{
    //		Logger::FrameworkDebug("[XMLParser::StartElement] %s, user_data = %p", name, user_data);
    XMLParserDelegate* delegateptr = reinterpret_cast<XMLParserDelegate*>(user_data);
    if (delegateptr)
    {
        Map<String, String> attributes;

        if (attrs)
        {
            //				Logger::FrameworkDebug("[XMLParser::StartElement] attrs in");

            int32 i = 0;
            while (attrs[i])
            {
                const char* str = (attrs[i + 1]) ? reinterpret_cast<const char*>(attrs[i + 1]) : "";
                attributes[reinterpret_cast<const char*>(attrs[i])] = str;

                //					Logger::FrameworkDebug("[XMLParser::StartElement] %s = %s", attrs[i], str);

                i += 2;
            }

            //				Logger::FrameworkDebug("[XMLParser::StartElement] attrs out");
        }

        delegateptr->OnElementStarted(reinterpret_cast<const char*>(name), "", "", attributes);
    }
}

void XMLParser::EndElement(void* user_data, const xmlChar* name)
{
    //		Logger::FrameworkDebug("[XMLParser::EndElement] %s", name);
    XMLParserDelegate* delegateptr = reinterpret_cast<XMLParserDelegate*>(user_data);
    if (delegateptr)
    {
        delegateptr->OnElementEnded(reinterpret_cast<const char*>(name), "", "");
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
