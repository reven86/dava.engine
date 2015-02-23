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


#ifndef FONTMANAGERDIALOG_H
#define FONTMANAGERDIALOG_H

#include <QDialog>
#include "DAVAEngine.h"

class QStandardItemModel;
class QStringList;
class QItemSelectionModel;
class QStandardItem;

namespace DAVA {
    class Font;
}

namespace Ui {
class FontManagerDialog;
}

class FontManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FontManagerDialog(bool okButtonEnable = false, const QString& graphicsFontPath = QString(), QWidget *parent = 0);
    ~FontManagerDialog();
    //Return created font
    DAVA::Font * ResultFont();
private:
    Ui::FontManagerDialog *ui;
    QStandardItemModel *tableModel;
    DAVA::Font *dialogResultFont;
	QString currentFontPath;
    
    void ConnectToSignals();
    void InitializeTableView();
    void UpdateTableViewContents();
	void UpdateDialogInformation();
    DAVA::Font* GetSelectedFont(QItemSelectionModel *selectionModel);
	//void SetDefaultItemFont(QStandardItem *item, QString defaultFontName, QString fontName);
	QStandardItem* CreateFontItem(QString itemText, QString fontName, QString defaultFontName);

    void ValidateFont(const DAVA::Font* font) const;

private slots:
    void OkButtonClicked();
    void SetDefaultButtonClicked();

};

#endif // FONTMANAGERDIALOG_H
