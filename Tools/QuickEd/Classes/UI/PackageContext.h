#ifndef __QUICKED_PACKAGE_CONTEXT_H__
#define __QUICKED_PACKAGE_CONTEXT_H__

#include <QObject>
#include <QPoint>
#include <QString>

class QSortFilterProxyModel;
class QItemSelection;
class PackageModel;
class Document;

class PackageContext : public QObject
{
    Q_OBJECT
    
public:
    PackageContext(Document *document);
    virtual ~PackageContext();
    
    Document *GetDocument() const;
    PackageModel *GetModel() const;
    QSortFilterProxyModel *GetFilterProxyModel() const;
    const QString &GetFilterString() const;
    
    QItemSelection *GetCurrentSelection() const;
    
private:
    Document *document;

    QPoint scrollPosition;
    PackageModel *model;
    QSortFilterProxyModel *proxyModel;
    QItemSelection *currentSelection;
    QString filterString;
};


#endif // __QUICKED_PACKAGE_CONTEXT_H__
