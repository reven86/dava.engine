/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "SettingsStateDialog.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <Qstring>


SettingsStateDialog::SettingsStateDialog(DAVA::Map<DAVA::String,std::pair<DAVA::uint32, bool> >* _flags, QWidget* parent)
		:QDialog(parent)
{
	flags = _flags;
	checkBoxForNothing = NULL;
	checkBoxForAll = NULL;
	mainLayout = new QVBoxLayout;
	for(DAVA::Map<DAVA::String,std::pair<DAVA::uint32, bool> >::iterator it = flags->begin(); it != flags->end(); ++it)
	{
		DAVA::String description = it->first;
		bool isChecked = it->second.second;
		
		QCheckBox* checkBox = new QCheckBox(this);

		if( it->second.first == 0)
		{
			checkBoxForNothing = checkBox;
		}
		if( it->second.first == std::numeric_limits<DAVA::uint32>::max())
		{
			checkBoxForAll = checkBox;
		}
		checkList.push_back(checkBox);
		initialValues[description] = isChecked;
		checkBox->setText(QString(description.c_str()));
		checkBox->setChecked(isChecked);
		connect(checkBox, SIGNAL(stateChanged (int)), this,SLOT(CheckBoxChecked(int)));
		mainLayout->addWidget(checkBox);
	}
	DVASSERT(checkBoxForNothing);
	DVASSERT(checkBoxForAll);
	btnBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);;
	connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));

	mainLayout->addWidget(btnBox);
	setLayout(mainLayout);
	
	setWindowFlags(Qt::Popup);
}

SettingsStateDialog::~SettingsStateDialog()
{
	Q_FOREACH(QWidget* item,checkList)
	{
		delete item;
	}
	delete btnBox;
	delete mainLayout;
}

void SettingsStateDialog::CheckBoxChecked(int value)
{
	QCheckBox* sender = dynamic_cast<QCheckBox*>(QObject::sender());
	DVASSERT(sender);
	DAVA::String key = sender->text().toStdString();
	bool isChecked = value != Qt::Unchecked;
	(*flags)[key].second =  isChecked;
	
	if(!isChecked)
	{
		Q_FOREACH(QCheckBox* item, checkList )
		{
			if(item->isChecked())
			{
				return;
			}
		}
		checkBoxForNothing->setChecked(true);
		checkBoxForNothing->setEnabled(false);
		return;
	}
	if( sender == checkBoxForNothing ||	sender == checkBoxForAll)
	{
		Q_FOREACH(QCheckBox* item, checkList )
		{
			if(item!=sender)
			{
				item->setChecked(false);
			}
		}
	}
	else
	{
		checkBoxForNothing->setChecked(false);
		checkBoxForNothing->setEnabled(true);
		checkBoxForAll->setChecked(false);
	}
}

void SettingsStateDialog::reject()
{
	for(DAVA::Map<DAVA::String, bool>::iterator it = initialValues.begin(); it != initialValues.end(); ++it)
	{
		(*flags)[it->first].second = it->second;
	}
	QDialog::reject();
}