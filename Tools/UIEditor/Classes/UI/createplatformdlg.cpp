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


#include "Classes/UI/createplatformdlg.h"
#include "ui_createplatformdlg.h"
#include <QMessageBox>
#include "HierarchyTreeController.h"
#include "HierarchyTree.h"

CreatePlatformDlg::CreatePlatformDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreatePlatformDlg)
{
    ui->setupUi(this);
}

CreatePlatformDlg::~CreatePlatformDlg()
{
    delete ui;
}

QString CreatePlatformDlg::GetPlatformName() const
{
	return ui->lineEdit->text();
}

int CreatePlatformDlg::GetWidth() const
{
	return ui->width->value();
}

int CreatePlatformDlg::GetHeight() const
{
	return ui->height->value();
}

void CreatePlatformDlg::accept()
{
	const QString platformName = GetPlatformName();
	if (!platformName.isNull() && !platformName.isEmpty())
	{
		if (GetHeight() > 0 && GetWidth() > 0)
		{
			if(!HierarchyTreeController::Instance()->GetTree().IsPlatformNamePresent(platformName) )
			{
				QDialog::accept();
			}
			else
			{
				QMessageBox msgBox;
				msgBox.setText(tr("Please fill platform name field with unique value."));
				msgBox.exec();
			}
		}
		else
		{
			QMessageBox msgBox;
			msgBox.setText(tr("Platform height and weight should have non-zero value!"));
			msgBox.exec();
		}
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Please fill platform name field with value. It can't be empty."));
		msgBox.exec();
	}	
}