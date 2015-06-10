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


#include <QObject>
#include <QStandardItemModel>
#include <QMap>


#include "FileSystem/Logger.h"


class LogModel
    : public QStandardItemModel
    , public DAVA::LoggerOutput
{
    Q_OBJECT

signals:
    void logged(int ll, const QString& text);

public:
    enum Roles
    {
        LEVEL_ROLE = Qt::UserRole,
    };

public:
    explicit LogModel(QObject* parent = NULL);
    ~LogModel();

    QPixmap GetIcon(int ll) const;

    void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text);
    void Output(DAVA::Logger::eLogLevel ll, const DAVA::char16* text);

public slots:
    void AddMessage(int ll, const QString& text);

private:
    QList<QStandardItem *> CreateItem(int ll, const QString& text) const;
    QString normalize(const QString& text) const;

    mutable QMap<int, QPixmap> icons;
};


#endif // __LOGMODEL_H__
