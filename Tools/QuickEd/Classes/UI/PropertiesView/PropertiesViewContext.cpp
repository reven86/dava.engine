#include "PropertiesViewContext.h"

#include "UI/Document.h"

PropertiesViewContext::PropertiesViewContext(Document *doc) : document(doc)
{

}

PropertiesViewContext::~PropertiesViewContext()
{

}

Document * PropertiesViewContext::GetDocument() const
{
    return document;
}
