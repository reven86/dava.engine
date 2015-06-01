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


#include "errorslistdialog.h"
#include "ui_errorslistdialog.h"
#include "ScreenWrapper.h"

ErrorsListDialog::ErrorsListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ErrorsListDialog)
{
    ui->setupUi(this);
	
    listModel = new QStringListModel(this);

    ui->errorsListView->setModel(listModel);
    ui->errorsListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

ErrorsListDialog::~ErrorsListDialog()
{
    delete ui;
}

void ErrorsListDialog::InitializeErrorsList(const Set<String>& errorsSet)
{
	QStringList list;
	for (Set<String>::const_iterator p = errorsSet.begin( );p != errorsSet.end( ); ++p)
	{
		list.append(QString::fromStdString(*p));
	}
	
	// Clean up errors list before adding new rows
    if (listModel->rowCount() > 0)
    {
        listModel->removeRows(0, listModel->rowCount());
    }
    listModel->setStringList(list);
	
	ScreenWrapper::Instance()->RestoreApplicationCursor();
}