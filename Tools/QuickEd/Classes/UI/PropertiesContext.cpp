#include "PropertiesContext.h"

#include "UI/Document.h"

PropertiesContext::PropertiesContext(Document *doc) : QObject(doc), document(doc)
{

}

PropertiesContext::~PropertiesContext()
{

}

Document *PropertiesContext::GetDocument() const
{
    return document;
}
