/*==================================================================================
 Copyright (c) 2008, binaryzebra
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the binaryzebra nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/


#include "LogModel.h"

#include <QPainter>
#include <QThread>
#include <QApplication>
#include <QMetaObject>
#include <QTimer>
#include <QMutex>

#include "Base/GlobalEnum.h"
#include "Debug/DVAssert.h"


LogModel::LogModel(QObject* parent)
    : QAbstractListModel(parent)
    , mutex(new QMutex)
    , syncTimer(new QTimer(this))
{
    DVASSERT_MSG(thread() == qApp->thread(), "don't create this model in the separate thread!");
    CreateIcons();
    func = [](const DAVA::String &str)
    {
        return str;
    };
    syncTimer->setSingleShot(true);
    syncTimer->setInterval(50);
    connect(syncTimer, &QTimer::timeout, this, &LogModel::Sync);

    QFontMetrics fm(QApplication::font());
    const int margin = 5;
    rowSize.setHeight(fm.height() + margin);
}

void LogModel::SetConvertFunction(ConvertFunc func_)
{
    func = func_;
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    const auto &item = items.at(index.row());
    switch (role)
    {
    case Qt::ToolTipRole:
    case Qt::DisplayRole:
        return item.text;
    case Qt::SizeHintRole:
        return rowSize;

    case Qt::DecorationRole:
        return GetIcon(item.ll);

    case LEVEL_ROLE:
        return item.ll;
    case INTERNAL_DATA_ROLE:
        return item.data;
    default:
        return QVariant();
    }
}

int LogModel::rowCount(const QModelIndex &parent) const
{
    return items.size();
}

void LogModel::AddMessage(DAVA::Logger::eLogLevel ll, const QByteArray& text)
{
    DVASSERT(QThread::currentThread() == qApp->thread());
    int count = rowCount();
    beginInsertRows(QModelIndex(), count, count);
    items.append(LogItem(ll,
        QString::fromStdString(func(text.toStdString())),
        text));
    RecalculateRowWidth(text);
    endInsertRows();
}

void LogModel::AddMessageAsync(DAVA::Logger::eLogLevel ll, const QByteArray& text)
{
    DVASSERT(QThread::currentThread() != qApp->thread());
    {
        QMutexLocker lock(mutex.get());
        itemsToAdd.append(LogItem(ll,
            QString::fromStdString(func(text.toStdString())),
            text));
    }
    QMetaObject::invokeMethod(syncTimer, "start", Qt::QueuedConnection);
}

void LogModel::Sync()
{
    DVASSERT(QThread::currentThread() == qApp->thread());
    QVector<LogItem> itemsToAddCopy;
    {
        QMutexLocker lock(mutex.get());
        itemsToAddCopy.swap(itemsToAdd);
    }

    if (itemsToAddCopy.empty())
    {
        return;
    }

    int count = rowCount();
    beginInsertRows(QModelIndex(), count, count + itemsToAddCopy.size() - 1);
    for (auto& item : itemsToAddCopy)
    {
        RecalculateRowWidth(item.text);
    }
    items += itemsToAddCopy;
    endInsertRows();
}

void LogModel::Clear()
{
    int count = rowCount();
    if (count <= 0)
    {
        return;
    }
    beginRemoveRows(QModelIndex(), 0, count - 1);
    items.clear();
    endRemoveRows();
}

void LogModel::CreateIcons()
{
    const auto &logMap = GlobalEnumMap<DAVA::Logger::eLogLevel>::Instance();
    for (size_t i = 0; i < logMap->GetCount(); ++i)
    {
        int value;
        bool ok = logMap->GetValue(i, value);
        if (!ok)
        {
            DVASSERT_MSG(ok, "wrong enum used to create eLogLevel list");
            break;
        }
        QPixmap pix(16, 16);
        pix.fill(Qt::transparent);
        QPainter p(&pix);

        QColor bg = Qt::transparent;
        QColor fg1 = Qt::gray;

        switch (value)
        {
        case DAVA::Logger::LEVEL_FRAMEWORK:
            bg = Qt::lightGray;
            break;
        case DAVA::Logger::LEVEL_DEBUG:
            bg = Qt::blue;
            break;
        case DAVA::Logger::LEVEL_INFO:
            bg = Qt::green;
            break;
        case DAVA::Logger::LEVEL_WARNING:
            bg = Qt::yellow;
            break;
        case DAVA::Logger::LEVEL_ERROR:
            bg = Qt::red;
            break;
        default:
            break;
        }

        const int ofs = 3;

        p.setBrush(bg);
        QRect rc = QRect(QPoint(0, 0), pix.size()).adjusted(ofs, ofs, -ofs, -ofs);
        p.setPen(fg1);
        p.drawEllipse(rc);

        icons.append(pix);
    }
}

void LogModel::RecalculateRowWidth(const QString& text)
{
    QFontMetrics fm(QApplication::font());
    const int margin = 10;
    rowSize.setWidth(qMax(rowSize.width(), fm.width(text) + margin));
}
const QPixmap &LogModel::GetIcon(int ll) const
{
    return icons.at(ll);
}

LogModel::LogItem::LogItem(DAVA::Logger::eLogLevel ll_, const QString& text_, const QString &data_)
    : ll(ll_), text(text_), data(data_)
{
    text = text.split('\n', QString::SkipEmptyParts).join("\n");
}