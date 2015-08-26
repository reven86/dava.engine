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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "applicationmanager.h"
#include "buttonswidget.h"
#include "filedownloader.h"
#include <QMainWindow>
#include <QListWidgetItem>
#include <QtGui>
#include <QSet>
#include <QDebug>

class ListModel;
class QSortFilterProxyModel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void OnRefreshClicked();
    void OnListItemClicked(QModelIndex);
    void RefreshApps();
    void OnURLClicked();

    void OnRun(int rowNumber);
    void OnInstall(int rowNumber);
    void OnRemove(int rowNumber);

    void OnCellClicked(const QPoint & pos);
    void OnCellDoubleClicked(QModelIndex index);

    void OnlinkClicked(QUrl url);
    
    void NewsDownloadFinished(QByteArray downloadedData, QList< QPair<QByteArray, QByteArray> > rawHeaderList, int errorCode, QString errorDescr);

private:
    void ShowWebpage();
    void ShowTable(const QString & branchID);
    void ShowUpdateDialog(QQueue<UpdateTask> & tasks);

    void UpdateURLValue();

    void RefreshBranchesList();
    void UpdateButtonsState(int rowNumber, ButtonsWidget::ButtonsState state);

    void GetTableApplicationIDs(int rowNumber, QString & appID, QString & installedVersionID, QString & avalibleVersionID);

    QWidget * CreateAppNameTableItem(const QString & stringID);
    QWidget * CreateAppInstalledTableItem(const QString & stringID);
    QWidget * CreateAppAvalibleTableItem(Application * app);

    Ui::MainWindow *ui;
    ApplicationManager * appManager;

    QNetworkAccessManager * networkManager;
    FileDownloader * newsDownloader;
    
    QPersistentModelIndex selectedListItem;
    QString selectedBranchID;

    QFont tableFont;
    ListModel *listModel;
    QSortFilterProxyModel *filterModel;
};

#endif // MAINWINDOW_H
