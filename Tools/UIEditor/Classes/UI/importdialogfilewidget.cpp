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


#include "importdialogfilewidget.h"
#include "ui_importdialogfilewidget.h"
#include "IconHelper.h"

#define EVEN_COLOR QColor(0xFF, 0xFF, 0xFF)
#define ODD_COLOR QColor(0xEE, 0xEE, 0xEE)
#define ACTIVE_FILE_COLOR QColor(0, 0, 0)
#define IGNORED_FILE_COLOR QColor(0xA0, 0xA0, 0xA0)

ImportDialogFileWidget::ImportDialogFileWidget(uint32 id, QWidget *parent)
:	QWidget(parent)
,	ui(new Ui::ImportDialogFileWidget)
,	id(id)
,	activeAction(ImportDialog::ACTION_IGNORE)
,	neverShowSizeWidget(false)
,	showUpperIcons(false)
{
	ui->setupUi(this);
	ui->resetToParentButton->setIcon(QIcon(IconHelper::GetIgnoreIconPath()));
	ui->upperIgnoreIcon->setPixmap(QIcon(IconHelper::GetIgnoreIconPath()).pixmap(16, 16));
	ui->upperScreenIcon->setPixmap(QIcon(IconHelper::GetScreenIconPath()).pixmap(16, 16));
	ui->upperAggregatorIcon->setPixmap(QIcon(IconHelper::GetAggregatorIconPath()).pixmap(16, 16));

	UpdateState(ImportDialog::ACTION_IGNORE);
	SetFilename("");

	ui->actionButtons->setId(ui->ignoreRadioButton, ImportDialog::ACTION_IGNORE);
	ui->actionButtons->setId(ui->screenRadioButton, ImportDialog::ACTION_SCREEN);
	ui->actionButtons->setId(ui->aggregatorRadioButton, ImportDialog::ACTION_AGGREGATOR);

	connect(ui->actionButtons, SIGNAL(buttonClicked(int)), this, SLOT(UpdateState(int)));
	connect(ui->resetToParentButton, SIGNAL(clicked()), this, SLOT(ResetAggregatorSize()));
	connect(ui->aggregatorWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnSizeChanged()));
	connect(ui->aggregatorHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnSizeChanged()));

	QColor color;
	if (id % 2 == 0)
	{
		color = EVEN_COLOR;
	}
	else
	{
		color = ODD_COLOR;
	}

	QPalette pal(palette());
	pal.setColor(QPalette::Background, color);
	setAutoFillBackground(true);
	setPalette(pal);

	UpdateState(activeAction);
}

ImportDialogFileWidget::~ImportDialogFileWidget()
{
	delete ui;
}

void ImportDialogFileWidget::SetSizeWidgetShowable(bool showable)
{
	neverShowSizeWidget = !showable;
}

ImportDialog::eAction ImportDialogFileWidget::GetSelectedAction() const
{
	return activeAction;
}

void ImportDialogFileWidget::SetAction(ImportDialog::eAction action)
{
	if (activeAction == action)
	{
		return;
	}

	UpdateState(action);
}

void ImportDialogFileWidget::UpdateState(int action)
{
	ui->actionButtons->blockSignals(true);

	QPalette pal = ui->fileNameLabel->palette();

	activeAction = (ImportDialog::eAction)action;
	switch (activeAction)
	{
		case ImportDialog::ACTION_IGNORE:
			ui->ignoreRadioButton->setChecked(true);
			ui->actionLabel->setPixmap(QIcon(IconHelper::GetIgnoreIconPath()).pixmap(16, 16));
			ui->actionLabel->setToolTip(tr("Ignore file"));
			ui->aggregatorSizeWidget->hide();
			pal.setColor(QPalette::Foreground, IGNORED_FILE_COLOR);
			break;

		case ImportDialog::ACTION_SCREEN:
			ui->screenRadioButton->setChecked(true);
			ui->actionLabel->setPixmap(QIcon(IconHelper::GetScreenIconPath()).pixmap(16, 16));
			ui->actionLabel->setToolTip(tr("Load as Screen"));
			ui->aggregatorSizeWidget->hide();
			pal.setColor(QPalette::Foreground, ACTIVE_FILE_COLOR);
			break;

		case ImportDialog::ACTION_AGGREGATOR:
			ui->aggregatorRadioButton->setChecked(true);
			ui->actionLabel->setToolTip(tr("Load as Aggregator"));
			ui->actionLabel->setPixmap(QIcon(IconHelper::GetAggregatorIconPath()).pixmap(16, 16));

			if (!neverShowSizeWidget)
			{
				ui->aggregatorSizeWidget->show();
			}

			pal.setColor(QPalette::Foreground, ACTIVE_FILE_COLOR);
			break;

		default:
			break;
	}

	ui->upperIgnoreIcon->setVisible(showUpperIcons);
	ui->upperScreenIcon->setVisible(showUpperIcons);
	ui->upperAggregatorIcon->setVisible(showUpperIcons);

	if (showUpperIcons)
	{
		ui->actionLabel->setPixmap(QIcon().pixmap(16, 16));
	}

	ui->fileNameLabel->setPalette(pal);
	ui->actionButtons->blockSignals(false);

	emit ActionChanged(id);
}

QString ImportDialogFileWidget::GetFilename() const
{
	return filename;
}

QSize ImportDialogFileWidget::GetSize() const
{
	return QSize(ui->aggregatorWidthSpinBox->value(), ui->aggregatorHeightSpinBox->value());
}

void ImportDialogFileWidget::SetFilename(const QString& filename)
{
	this->filename = filename;
	ui->fileNameLabel->setText(filename);
}

void ImportDialogFileWidget::InitWithFilenameAndAction(const QString& filename, ImportDialog::eAction action)
{
	SetFilename(filename);
	UpdateState(action);
}

void ImportDialogFileWidget::ResetAggregatorSize()
{
	ui->aggregatorWidthSpinBox->setValue(-1);
	ui->aggregatorHeightSpinBox->setValue(-1);
}

void ImportDialogFileWidget::OnSizeChanged()
{
	emit SizeChanged();
}

void ImportDialogFileWidget::SetUpperIconsShowable(bool showable)
{
	showUpperIcons = showable;
	UpdateState(activeAction);
}
