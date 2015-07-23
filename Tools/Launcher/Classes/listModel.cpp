#include "listModel.h"
#include <QBrush>
#include <QColor>
#include <QApplication>
#include <QTimer>
#include "applicationmanager.h"

ListModel::ListModel(const ApplicationManager *appManager_, QObject *parent)
    : QAbstractListModel(parent)
    , fontFavorites(QApplication::font())
    , pushTimer(new QTimer(this))
    , appManager(appManager_)
{
    pushTimer->setInterval(0);
    connect(pushTimer, &QTimer::timeout, this, &ListModel::updateRows, Qt::QueuedConnection);
    
    fontFavorites.setPointSize(fontFavorites.pointSize() + 1);
    fontFavorites.setBold(true);
}

void ListModel::clearItems()
{
    beginRemoveRows(QModelIndex(), 0, rowCount());
    items.clear();
    endInsertRows();
}

void ListModel::addItem(const QString &dataText, ListItemType type)
{
    items.push_back({ appManager->GetString(dataText), dataText, type });
    pushTimer->start();
}

QVariant ListModel::data(const QModelIndex &index, int role) const
{
    auto type = items.at(index.row()).type;
    switch (type)
    {
    case LIST_ITEM_SEPARATOR:
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
        case Qt::FontRole:
            switch (type)
            {
            case LIST_ITEM_FAVORITES:
                return fontFavorites;
            default:
                return QVariant();
            }
            break;
        case Qt::TextColorRole:
            switch (type)
            {
            case LIST_ITEM_BRANCH:
                return QColor(100, 100, 100);
            default:
                return QVariant();
            }
            break;
        }
    }
    switch(role)
    {
    case Qt::DisplayRole:
        return items.at(index.row()).text;
    case DAVA_WIDGET_ROLE:
        return items.at(index.row()).dataText;
    case Qt::SizeHintRole:
        return QSize(-1, 34);
    default:
        return QVariant();
    }

}

int ListModel::rowCount(const QModelIndex &parent) const
{
    return items.size();
}

void ListModel::updateRows()
{
    static int registredCount;
    const int count = rowCount();
    if (count != registredCount)
    {
        beginInsertRows(QModelIndex(), registredCount, count - 1);
        registredCount = rowCount();
        endInsertRows();
    }
}