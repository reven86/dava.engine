#ifndef __LIST_MODEL_H__
#define __LIST_MODEL_H__

#include <QObject>
#include <QAbstractListModel>
#include <QFont>

class ApplicationManager;

class ListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ListItemType
    {
        LIST_ITEM_NEWS,
        LIST_ITEM_FAVORITES,
        LIST_ITEM_BRANCH,
        LIST_ITEM_SEPARATOR
    };
    static const int DAVA_WIDGET_ROLE = Qt::UserRole + 1;
    ListModel(const ApplicationManager *appManager_, QObject *parent = nullptr);
    void clearItems();
    void addItem(const QString &dataText, ListItemType type);
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
private:
    struct Item
    {
        QString text;
        QString dataText;
        ListItemType type;
    };

    QList<Item> items;
    QFont fontFavorites;
    const ApplicationManager *appManager;
};

Q_DECLARE_METATYPE(ListModel::ListItemType);

#endif // __LIST_MODEL_H__
