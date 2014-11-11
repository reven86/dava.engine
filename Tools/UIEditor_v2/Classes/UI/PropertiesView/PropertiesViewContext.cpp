#include "PropertiesViewContext.h"

PropertiesViewContext::PropertiesViewContext( PackageDocument *doc ) : document(doc)
{

}

PropertiesViewContext::~PropertiesViewContext()
{

}

PackageDocument * PropertiesViewContext::Document() const
{
    return document;
}
