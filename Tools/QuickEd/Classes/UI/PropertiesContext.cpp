#include "PropertiesContext.h"

#include "Document.h"
#include "Properties/PropertiesModel.h"
#include "Model/PackageHierarchy/ControlNode.h"

PropertiesContext::PropertiesContext(Document *doc)
    : QObject(doc)
    , document(doc)
    , model(nullptr)
{

}

PropertiesContext::~PropertiesContext()
{
    if (model)
    {
        delete model;
        model = nullptr;
    }
}

Document *PropertiesContext::GetDocument() const
{
    return document;
}

PropertiesModel *PropertiesContext::GetModel() const
{
    return model;
}

void PropertiesContext::SetActiveNode(ControlNode *node)
{
    PropertiesModel *prevModel = model;
    model = node ? new PropertiesModel(node, document->GetCommandExecutor(), this) : nullptr;
    
    emit ModelChanged(model);
    
    if (prevModel)
        delete prevModel;
}
