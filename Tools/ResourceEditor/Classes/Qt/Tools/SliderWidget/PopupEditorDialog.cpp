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


#include "PopupEditorDialog.h"

#include <QLineEdit>
#include <QLayout>
#include <QIntValidator>

PopupEditorDialog::PopupEditorDialog(int initialValue,
									 int rangeMin, int rangeMax,
									 const QWidget* widget /* = 0 */,
									 QWidget* parent /* = 0 */)
:	QDialog(parent, Qt::Popup)
,	widget(widget)
,	value(initialValue)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setMaximumWidth(50);

	editValue = new QLineEdit(this);
	editValue->setText(QString::number(initialValue));

	QIntValidator* validator = new QIntValidator(rangeMin, rangeMax, editValue);
	editValue->setValidator(validator);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(editValue);

	setLayout(layout);

	connect(editValue, SIGNAL(returnPressed()), this, SLOT(OnReturnPressed()));
	connect(editValue, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
}

PopupEditorDialog::~PopupEditorDialog()
{
	emit ValueReady(widget, value);
}

void PopupEditorDialog::showEvent(QShowEvent* event)
{
	editValue->setFocus();
	editValue->selectAll();
	QDialog::showEvent(event);
}

void PopupEditorDialog::OnReturnPressed()
{
	value = editValue->text().toInt();
}

void PopupEditorDialog::OnEditingFinished()
{
	close();
}
