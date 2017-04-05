#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class UIRichAliasMap final
{
public:
    using Attributes = Map<String, String>;

    struct Alias
    {
        String alias;
        String tag;
        Attributes attributes;
    };

    void PutAlias(const Alias& alias);
    void PutAlias(const String& alias, const String& tag, const Attributes& attributes);
    void PutAlias(const String& alias, const String& tag);
    void PutAliasFromXml(const String& alias, const String& xmlSrc);
    bool HasAlias(const String& alias) const;
    const Alias& GetAlias(const String& alias) const;
    void RemoveAlias(const String& alias);
    void RemoveAll();

    String AsString() const;
    void FromString(const String& aliases);

private:
    UnorderedMap<String, Alias> aliases;
};
}