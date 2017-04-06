#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
/** Container for work with aliases in rich content source text. */
class UIRichAliasMap final
{
public:
    using Attributes = Map<String, String>; //!< Type for tag attributes

    /** Describe alias configuration. */
    struct Alias
    {
        String alias; //!< Alias tag name
        String tag; //!< Original tag name
        Attributes attributes; //!< Original attributes
    };

    /** Add new specified `alias`. */
    void PutAlias(const Alias& alias);
    /** Add new alias with specified alias, tag and attributes. */
    void PutAlias(const String& alias, const String& tag, const Attributes& attributes);
    /** Add new alias with specified alias and tag. */
    void PutAlias(const String& alias, const String& tag);
    /** Add new alias with specified alias and xml source. */
    void PutAliasFromXml(const String& alias, const String& xmlSrc);
    /** Check Alias by specified `alias` name. */
    bool HasAlias(const String& alias) const;
    /** Return Alias by specified `alias` name. */
    const Alias& GetAlias(const String& alias) const;
    /** Return count of aliases. */
    uint32 Count() const;
    /** Remove Alias by specified `alias` name. */
    void RemoveAlias(const String& alias);
    /** Remove all aliases. */
    void RemoveAll();
    /** Return all aliases as serializable string. */
    String AsString() const;
    /** Load aliases from serializable string. */
    void FromString(const String& aliases);

private:
    UnorderedMap<String, Alias> aliases;
};
}