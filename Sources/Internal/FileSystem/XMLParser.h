#ifndef __DAVAENGINE_XML_PARSER__
#define __DAVAENGINE_XML_PARSER__


#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

#include "FileSystem/FileSystem.h"

using xmlChar = unsigned char;

namespace DAVA
{
class XMLParserDelegate
{
public:
    virtual ~XMLParserDelegate() = default;

    virtual void OnElementStarted(const String& elementName, const String& namespaceURI
                                  ,
                                  const String& qualifedName, const Map<String, String>& attributes) = 0;
    virtual void OnElementEnded(const String& elementName, const String& namespaceURI
                                ,
                                const String& qualifedName) = 0;

    virtual void OnFoundCharacters(const String& chars) = 0;

    /**
	 \brief Returns attribute value if this value is presents in the attributesMap.
	 \param[in] attributes map you want to search for.
	 \param[in] key you fant to found in the map.
	 \param[out] writes to this string value for the key if attribute is present.
	 \returns true if attribute for key is present.
	 */
    inline bool GetAttribute(const Map<String, String>& attributesMap, const String& key, String& attributeValue);
};

class XMLParser : public BaseObject
{
public:
    XMLParser();

    static bool ParseFile(const FilePath& fileName, XMLParserDelegate* delegate);
    static bool ParseBytes(const unsigned char* bytes, int length, XMLParserDelegate* delegate);

private:
    static void StartDocument(void* user_data);
    static void EndDocument(void* user_data);
    static void Characters(void* user_data, const xmlChar* ch, int len);

    static void StartElement(void* user_data, const xmlChar* name, const xmlChar** attrs);
    static void EndElement(void* user_data, const xmlChar* name);

protected:
    virtual ~XMLParser();
};

bool XMLParserDelegate::GetAttribute(const Map<String, String>& attributesMap, const String& key, String& attributeValue)
{
    Map<String, String>::const_iterator it;
    it = attributesMap.find(key);
    if (it != attributesMap.end())
    {
        attributeValue = it->second;
        return true;
    }

    return false;
}
};

#endif //#ifndef __DAVAENGINE_XML_PARSER__