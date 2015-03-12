#ifndef __PROPERTIESVIEWCONTEXT_H__
#define __PROPERTIESVIEWCONTEXT_H__

#include <QObject>

class Document;
class PropertiesModel;
class ControlNode;

class PropertiesContext : public QObject
{
    Q_OBJECT
public:
    PropertiesContext(Document *doc);
    virtual ~PropertiesContext();

    Document *GetDocument() const;
    PropertiesModel *GetModel() const;
    void SetActiveNode(ControlNode *node);
    
signals:
    void ModelChanged(PropertiesModel *model);
    
private:
    Document *document;
    PropertiesModel *model;
};

#endif // __PROPERTIESVIEWCONTEXT_H__
