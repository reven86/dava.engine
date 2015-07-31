#include "listModel.h"
#include <QBrush>
#include <QColor>
#include <QApplication>
#include <QTimer>
#include "applicationmanager.h"

ListModel::ListModel(const ApplicationManager *appManager_, QObject *parent)
    : QAbstractListModel(parent)
    , fontFavorites(QApplication::font())
    , appManager(appManager_)
{   
    fontFavorites.setPointSize(fontFavorites.pointSize() + 1);
    fontFavorites.setBold(true);
}

void ListModel::clearItems()
{
    if (!items.isEmpty())
    {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        items.clear();
        endRemoveRows();
    }
}

void ListModel::addItem(const QString &dataText, ListItemType type)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    items.push_back({ appManager->GetString(dataText), dataText, type });
    endInsertRows();
}

QVariant ListModel::data(const QModelIndex &index, int role) const
{
    ListItemType type = items.at(index.row()).type;
    switch (type)
    {
    case LIST_ITEM_SEPARATOR: // for separator we display only custom data
        switch (role)
        {
        case Qt::BackgroundColorRole:
            return QBrush(QColor(180, 180, 180), Qt::HorPattern);
        case Qt::SizeHintRole:
            return QSize(0, 7);
        default:
            return QVariant();
        }
        break;
    default:
        switch (role)
        {
        case Qt::DisplayRole:
            return items.at(index.row()).text;
        case DAVA_WIDGET_ROLE:
            return items.at(index.row()).dataText;
        case Qt::SizeHintRole:
            return QSize(-1, 34);
        case Qt::FontRole:
            if (type == LIST_ITEM_FAVORITES)
            {
                return fontFavorites;
            }
            return QVariant();
        case Qt::TextColorRole:
            if (type == LIST_ITEM_BRANCH)
            {
                return QColor(100, 100, 100);
            }
            return QVariant();
        default:
            return QVariant();
        }
    }
    return QVariant();
}

int ListModel::rowCount(const QModelIndex &parent) const
{
    return items.size();
}
