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


#ifndef __QUICKED_SHARED_DATA_H__
#define __QUICKED_SHARED_DATA_H__

#include <QtCore>
#include <QWidget>
#include <QSharedPointer>
#include <DAVAEngine.h>
#include "Model/PackageHierarchy/ControlNode.h"
#include "Document.h"

struct WidgetContext
{
};

class SharedData : public QObject
{
    Q_OBJECT
public:
    SharedData(QObject *parent = nullptr);
    QVariant& GetData(const QByteArray &role);
    Document *GetDocument() const; //TODO - this is deprecated
    void SetData(const QByteArray &role, const QVariant &value);
    WidgetContext* GetContext(QWidget* requester) const;
    void SetContext(QWidget* requester, WidgetContext* widgetContext);
signals:
    void DataChanged(const QByteArray &role);
private:
    QMap < QByteArray, QVariant > values;
    std::map < QWidget*, std::unique_ptr<WidgetContext> > contexts;
};

inline Document* SharedData::GetDocument() const
{
    return qobject_cast<Document*>(parent());
}


Q_DECLARE_METATYPE(SharedData*);
Q_DECLARE_METATYPE(QAbstractItemModel*)
Q_DECLARE_METATYPE(QPointer<QAbstractItemModel>)
Q_DECLARE_METATYPE(QItemSelection);
Q_DECLARE_METATYPE(ControlNode*);
Q_DECLARE_METATYPE(QList<ControlNode*>);
Q_DECLARE_METATYPE(DAVA::UIControl*);
Q_DECLARE_METATYPE(QPersistentModelIndex);
Q_DECLARE_METATYPE(QList<QPersistentModelIndex>);

#endif // __QUICKED_SHARED_DATA_H__
