/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "treemodel.h"

#include <QApplication>
#include <QFile>
#include <QTreeView>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopWidget>
#include <QSortFilterProxyModel>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(simpletreemodel);

    QApplication app(argc, argv);

//    QFile file(":/default.txt");
//    file.open(QIODevice::ReadOnly);
//    file.close();

    QString val;
    QFile file;
    file.setFileName("/Users/binaryzebra/Sources/dava.framework/Tools/LeakLogViewer/leaks.log");
    file.open(QIODevice::ReadOnly);
    val = file.readAll();
    file.close();
//    qWarning() << val;
    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
    if (d.isNull())
    {
        qWarning() << "JSON Document Parsing Failed";
    }


    TreeModel model(d.object());

    QSortFilterProxyModel sortProxyModel;
    sortProxyModel.setSourceModel(&model);

    QTreeView view;
    view.resize(QDesktopWidget().availableGeometry(0).size() * 0.9);
    view.setModel(&sortProxyModel);

    view.sortByColumn(1, Qt::DescendingOrder);
    view.setWindowTitle(QObject::tr("Simple Tree Model"));
    view.setColumnWidth(0, QDesktopWidget().availableGeometry(0).size().width() * 0.7);
    view.show();

    return app.exec();
}
