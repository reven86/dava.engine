#include "UI/RichContent/UIRichAliasMap.h"
#include "FileSystem/XMLParser.h"

namespace DAVA
{
class AliasXmlDelegate final : public XMLParserDelegate
{
public:
    AliasXmlDelegate(const String& _alias)
        : alias(UIRichAliasMap::Alias{ _alias })
    {
    }

    AliasXmlDelegate(const AliasXmlDelegate& copy)
        : alias(copy.alias)
    {
    }

    void OnElementStarted(const String& elementName, const String& namespaceURI, const String& qualifedName, const Map<String, String>& attributes) override
    {
        if (alias.tag.empty())
        {
            alias.tag = elementName;
            alias.attributes = attributes;
        }
    }

    void OnElementEnded(const String& elementName, const String& namespaceURI, const String& qualifedName) override
    {
    }

    void OnFoundCharacters(const String& chars) override
    {
    }

    const UIRichAliasMap::Alias& GetAlias() const
    {
        return alias;
    }

private:
    UIRichAliasMap::Alias alias;
};

void UIRichAliasMap::PutAlias(const Alias& alias)
{
    aliases[alias.alias] = alias;
}

void UIRichAliasMap::PutAlias(const String& alias, const String& tag, const Attributes& attributes)
{
    aliases[alias] = Alias{ alias, tag, attributes };
}

void UIRichAliasMap::PutAlias(const String& alias, const String& tag)
{
    aliases[alias] = Alias{ alias, tag };
}

void UIRichAliasMap::PutAliasFromXml(const String& alias, const String& xmlSrc)
{
    RefPtr<XMLParser> p(new XMLParser());
    AliasXmlDelegate delegate(alias);
    p->ParseBytes(reinterpret_cast<const uint8*>(xmlSrc.c_str()), static_cast<int32>(xmlSrc.length()), &delegate);
    PutAlias(delegate.GetAlias());
}

bool UIRichAliasMap::HasAlias(const String& alias) const
{
    return aliases.find(alias) != aliases.end();
}

const UIRichAliasMap::Alias& UIRichAliasMap::GetAlias(const String& alias) const
{
    return aliases.at(alias);
}

void UIRichAliasMap::RemoveAlias(const String& alias)
{
    aliases.erase(alias);
}

void UIRichAliasMap::RemoveAll()
{
    aliases.clear();
}

String UIRichAliasMap::AsString() const
{
    String out;
    for (const auto& pair : aliases)
    {
        const Alias& alias = pair.second;

        out += alias.alias + ",<" + alias.tag;
        for (const auto& pair : alias.attributes)
        {
            out += " " + pair.first + "=\"" + pair.second + "\"";
        }
        out += " />;";
    }
    return out;
}

void UIRichAliasMap::FromString(const String& aliases)
{
    RemoveAll();

    Vector<String> tokens;
    Split(aliases, ";", tokens);
    for (const String& token : tokens)
    {
        size_t pos = token.find(",");
        String alias = token.substr(0, pos);
        String xmlSrc = token.substr(pos + 1);
        PutAliasFromXml(alias, xmlSrc);
    }
}
}