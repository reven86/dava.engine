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


#ifndef __UIEDITOR__IMPORTDIALOGFILEWIDGET__
#define __UIEDITOR__IMPORTDIALOGFILEWIDGET__

#include <QWidget>
#include "importdialog.h"

namespace Ui {
	class ImportDialogFileWidget;
}

class ImportDialogFileWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ImportDialogFileWidget(uint32 id, QWidget *parent = 0);
	~ImportDialogFileWidget();

	ImportDialog::eAction GetSelectedAction() const;
	void SetAction(ImportDialog::eAction action);

	QSize GetSize() const;

	QString GetFilename() const;
	void SetFilename(const QString& filename);

	void InitWithFilenameAndAction(const QString& filename, ImportDialog::eAction action);
	void SetSizeWidgetShowable(bool showable);
	void SetUpperIconsShowable(bool showable);

signals:
	void ActionChanged(uint32 id);
	void SizeChanged();

public slots:
	void ResetAggregatorSize();

private slots:
	void UpdateState(int action);
	void OnSizeChanged();

private:
	Ui::ImportDialogFileWidget *ui;

	uint32 id;
	ImportDialog::eAction activeAction;
	QString filename;
	bool neverShowSizeWidget;
	bool showUpperIcons;
};

#endif /* defined(__UIEDITOR__IMPORTDIALOGFILEWIDGET__) */
