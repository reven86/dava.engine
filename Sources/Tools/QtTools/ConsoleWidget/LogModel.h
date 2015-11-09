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


#ifndef __LOGMODEL_H__
#define __LOGMODEL_H__

#include "FileSystem/Logger.h"
#include <functional>

#include <QObject>
#include <QAbstractListModel>
#include <QSize>
#include <QPixmap>

class QMutex;
class QTimer;

class LogModel
    : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        LEVEL_ROLE = Qt::UserRole,
        INTERNAL_DATA_ROLE
    };
    using ConvertFunc = std::function < DAVA::String(const DAVA::String &) >;

    explicit LogModel(QObject* parent = nullptr);
    ~LogModel() = default;
    void SetConvertFunction(ConvertFunc func); //provide mechanism to convert data string to string to be displayed

    const QPixmap &GetIcon(int ll) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void AddMessage(DAVA::Logger::eLogLevel ll, const QByteArray &text);
    void AddMessageAsync(DAVA::Logger::eLogLevel ll, const QByteArray &msg);

public slots:
    void Clear();

private slots:
    void Sync();

private:
    void CreateIcons();
    void RecalculateRowWidth(const QString& text);
    struct LogItem
    {
        LogItem(DAVA::Logger::eLogLevel ll_ = DAVA::Logger::LEVEL_FRAMEWORK, const QString &text_ = QString(), const QString &data_ = QString());
        DAVA::Logger::eLogLevel ll;
        QString text;
        QString data;
    };
    QVector<LogItem> items;

    QVector<QPixmap> icons;
    ConvertFunc func;

    QVector<LogItem> itemsToAdd;
    std::unique_ptr<QMutex> mutex = nullptr;
    QTimer *syncTimer = nullptr;
    QSize rowSize;
};

#endif // __LOGMODEL_H__
