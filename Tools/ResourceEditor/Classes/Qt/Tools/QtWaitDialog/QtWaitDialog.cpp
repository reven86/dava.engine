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


#include <QApplication>
#include "Tools/QtWaitDialog/QtWaitDialog.h"
#include "ui_waitdialog.h"

QtWaitDialog::QtWaitDialog(QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::CustomizeWindowHint)
    , ui(new Ui::QtWaitDialog)
    , wasCanceled(false)
    , isRunnedFromExec(false)
{
    setFixedSize(400, 150);
	setWindowModality(Qt::WindowModal);

    ui->setupUi(this);

	QPalette pal = palette();
	pal.setColor(QPalette::Base, Qt::transparent);
	ui->waitLabel->setPalette(pal);

	connect(ui->waitButton, SIGNAL(pressed()), this, SLOT(CancelPressed()));
	connect(this, SIGNAL(canceled()), this, SLOT(WaitCanceled()));
}

QtWaitDialog::~QtWaitDialog()
{}

void QtWaitDialog::Exec(const QString &title, const QString &message, bool hasWaitbar, bool hasCancel)
{
	Setup(title, message, hasWaitbar, hasCancel);

	setCursor(Qt::BusyCursor);
    isRunnedFromExec = true;
    show();
	loop.exec();
}

void QtWaitDialog::Show(const QString &title, const QString &message, bool hasWaitbar, bool hasCancel)
{
	Setup(title, message, hasWaitbar, hasCancel);
	
	setCursor(Qt::BusyCursor);
    isRunnedFromExec = false;
	show();

    processEvents();
}

void QtWaitDialog::Reset()
{
	wasCanceled = false;
	emit canceled();

    if ( isRunnedFromExec )
    {
        loop.quit();
    }

    close();
	setCursor(Qt::ArrowCursor);
}

void QtWaitDialog::SetMessage(const QString &message)
{
	ui->waitLabel->setPlainText(message);
    processEvents();
}

void QtWaitDialog::SetRange(int min, int max)
{
	ui->waitBar->setRange(min, max);
    processEvents();
}

void QtWaitDialog::SetRangeMin(int min)
{
	ui->waitBar->setMinimum(min);
    processEvents();
}

void QtWaitDialog::SetRangeMax(int max)
{
	ui->waitBar->setMaximum(max);
    processEvents();
}

void QtWaitDialog::SetValue(int value)
{
	ui->waitBar->setVisible(true);
	ui->waitBar->setValue(value);
    processEvents();
}

void QtWaitDialog::CancelPressed()
{
	wasCanceled = true;
	emit canceled();
}

void QtWaitDialog::WaitCanceled()
{
	ui->waitButton->setEnabled(false);
    processEvents();
}

void QtWaitDialog::processEvents()
{
    if ( !isRunnedFromExec )
    {
        QApplication::processEvents();
    }
}

void QtWaitDialog::EnableCancel(bool enable)
{
	if(!wasCanceled)
	{
		ui->waitButton->setEnabled(enable);
	}
}

void QtWaitDialog::Setup(const QString &title, const QString &message, bool hasWaitbar, bool hasCancel)
{
	setWindowTitle(title);
	SetMessage(message);

	ui->waitButton->setEnabled(hasCancel);
	ui->waitBar->setVisible(hasWaitbar);

	wasCanceled = false;
}

bool QtWaitDialog::WasCanceled() const
{
	return wasCanceled;
}