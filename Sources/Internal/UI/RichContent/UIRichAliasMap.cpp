#include "UI/RichContent/UIRichAliasMap.h"
#include "FileSystem/XMLParser.h"
#include "Logger/Logger.h"
#include "Utils/Utils.h"

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
    aliases.push_back(alias);
    asStringDirty = true;
}

void UIRichAliasMap::PutAlias(const String& alias, const String& tag, const Attributes& attributes)
{
    aliases.push_back(Alias{ alias, tag, attributes });
    asStringDirty = true;
}

void UIRichAliasMap::PutAlias(const String& alias, const String& tag)
{
    aliases.push_back(Alias{ alias, tag });
    asStringDirty = true;
}

void UIRichAliasMap::PutAliasFromXml(const String& alias, const String& xmlSrc)
{
    RefPtr<XMLParser> p(new XMLParser());
    AliasXmlDelegate delegate(alias);
    if (p->ParseBytes(reinterpret_cast<const uint8*>(xmlSrc.c_str()), static_cast<int32>(xmlSrc.length()), &delegate))
    {
        const Alias& alias = delegate.GetAlias();
        if (!alias.alias.empty() && !alias.tag.empty())
        {
            PutAlias(alias);
        }
    }
}

bool UIRichAliasMap::HasAlias(const String& alias) const
{
    return std::find_if(aliases.begin(), aliases.end(), [&alias](const Alias& a) { return a.alias == alias; }) != aliases.end();
}

const UIRichAliasMap::Alias& UIRichAliasMap::GetAlias(const String& alias) const
{
    auto it = std::find_if(aliases.begin(), aliases.end(), [&alias](const Alias& a) { return a.alias == alias; });
    if (it != aliases.end())
    {
        return *it;
    }
    static const UIRichAliasMap::Alias EMPTY_ALIAS;
    return EMPTY_ALIAS;
}

uint32 UIRichAliasMap::Count() const
{
    return static_cast<uint32>(aliases.size());
}

void UIRichAliasMap::RemoveAlias(const String& alias)
{
    aliases.erase(std::remove_if(aliases.begin(), aliases.end(), [&alias](const Alias& a) { return a.alias == alias; }), aliases.end());
    asStringDirty = true;
}

void UIRichAliasMap::RemoveAll()
{
    aliases.clear();
    asStringDirty = true;
}

const String& UIRichAliasMap::AsString()
{
    if (asStringDirty)
    {
        asStringTemp.clear();
        for (const Alias& alias : aliases)
        {
            asStringTemp += alias.alias + ",<" + alias.tag;
            for (const auto& pair : alias.attributes)
            {
                asStringTemp += " " + pair.first + "=\"" + pair.second + "\"";
            }
            asStringTemp += " />;";
        }
        asStringDirty = false;
    }
    return asStringTemp;
}

void UIRichAliasMap::FromString(const String& aliases)
{
    RemoveAll();

    Vector<String> tokens;
    Split(aliases, ";", tokens);
    for (const String& token : tokens)
    {
        size_t pos = token.find(",");
        if (pos != String::npos)
        {
            String alias = token.substr(0, pos);
            String xmlSrc = token.substr(pos + 1);
            PutAliasFromXml(alias, xmlSrc);
        }
        else
        {
            Logger::Error("[UIRichAliasMap::FromString] Wrong string token '%s'!", token.c_str());
        }
    }
}

bool UIRichAliasMap::operator==(const UIRichAliasMap& b) const
{
    const auto pred = [](const Alias& a, const Alias& b) { return (a == b); };

    return (aliases.size() == b.aliases.size() &&
            std::equal(aliases.begin(), aliases.end(), b.aliases.begin(), pred));
}

bool UIRichAliasMap::operator!=(const UIRichAliasMap& b) const
{
    return !(operator==(b));
}

bool UIRichAliasMap::Alias::operator==(const Alias& b) const
{
    const auto pred = [](const Attributes::value_type& a, const Attributes::value_type& b) { return (a == b); };

    return (alias == b.alias &&
            tag == b.tag &&
            attributes.size() == b.attributes.size() &&
            std::equal(attributes.begin(), attributes.end(), b.attributes.begin(), pred));
}

bool UIRichAliasMap::Alias::operator!=(const Alias& b) const
{
    return !(operator==(b));
}
}