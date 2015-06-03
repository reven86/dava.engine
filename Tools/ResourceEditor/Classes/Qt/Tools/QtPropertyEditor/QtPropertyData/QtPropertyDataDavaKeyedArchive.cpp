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


#include "DAVAEngine.h"
#include "Debug/DVAssert.h"
#include "Main/QtUtils.h"
#include "QtPropertyDataDavaKeyedArchive.h"
#include "QtPropertyDataKeyedArchiveMember.h"
#include "Deprecated/EditorConfig.h"

#include <QSet>
#include <QMenu>
#include <QGridLayout>
#include <QAction>
#include <QLabel>
#include <QMessageBox>
#include <QKeyEvent>

QtPropertyDataDavaKeyedArcive::QtPropertyDataDavaKeyedArcive(DAVA::KeyedArchive *_archive)
	: archive(_archive)
	, lastCommand(NULL)
	, lastAddedType(DAVA::VariantType::TYPE_STRING)
{
	if(NULL != archive)
	{
		archive->Retain();
	}

	SetEnabled(false);

	// add optional widget (button) to add new key
	QToolButton *addButton = AddButton();
	addButton->setIcon(QIcon(":/QtIcons/keyplus.png"));
    addButton->setToolTip("Add keyed archive member");
	addButton->setIconSize(QSize(12, 12));
	//addButton->setAutoRaise(true);
	QObject::connect(addButton, SIGNAL(released()), this, SLOT(AddKeyedArchiveField()));

	UpdateValue();
}

QtPropertyDataDavaKeyedArcive::~QtPropertyDataDavaKeyedArcive()
{
	if(NULL != archive)
	{
		archive->Release();
	}

	if(NULL != lastCommand)
	{
		delete lastCommand;
	}
}

const DAVA::MetaInfo * QtPropertyDataDavaKeyedArcive::MetaInfo() const
{
	return DAVA::MetaInfo::Instance<DAVA::KeyedArchive *>();
}

QVariant QtPropertyDataDavaKeyedArcive::GetValueInternal() const
{
	QVariant v;

	if(NULL != archive)
	{
		v = QString("KeyedArchive");
	}
	else
	{
		v = QString("KeyedArchive [NULL]");
	}

	return v;
}

bool QtPropertyDataDavaKeyedArcive::UpdateValueInternal()
{
	// update children
	{
		QSet<QtPropertyData *> dataToRemove;

		// at first step of sync we mark (placing to vector) items to remove
		for(int i = 0; i < ChildCount(); ++i)
		{
			QtPropertyData *child = ChildGet(i);
			if(NULL != child)
			{
				dataToRemove.insert(child);
			}
		}

		// as second step we go through keyed archive and add new data items,
		// and remove deleting mark from items that are still in archive
		if(NULL != archive)
		{
			DAVA::Map<DAVA::String, DAVA::VariantType*> data = archive->GetArchieveData();
			DAVA::Map<DAVA::String, DAVA::VariantType*>::iterator i = data.begin();

			for(; i != data.end(); ++i)
			{
				QtPropertyData *childData = ChildGet(i->first.c_str());

				// this key already in items list
				if(NULL != childData)
				{
					// remove deleting mark
					dataToRemove.remove(childData);
				}
				// create new child data
				else
				{
					ChildCreate(i->first.c_str(), i->second);
				}
			}
		}

		// delete all marked items
		QSetIterator<QtPropertyData *> it(dataToRemove);
		while(it.hasNext())
		{
			ChildRemove(it.next());
		}
	}

	return false;
}

void QtPropertyDataDavaKeyedArcive::ChildCreate(const QString &key, DAVA::VariantType *value)
{
	QtPropertyData *childData = NULL;

	if(value->type == DAVA::VariantType::TYPE_KEYED_ARCHIVE)
	{
		childData = new QtPropertyDataDavaKeyedArcive(value->AsKeyedArchive());
	}
	else
	{
		childData = new QtPropertyKeyedArchiveMember(archive, key.toStdString());
	}

	ChildAdd(key, childData);

	// add optional widget (button) to remove this key
	QToolButton *remButton = childData->AddButton();
	remButton->setIcon(QIcon(":/QtIcons/keyminus.png"));
    remButton->setToolTip("Remove keyed archive member");
	remButton->setIconSize(QSize(12, 12));

	QObject::connect(remButton, SIGNAL(clicked()), this, SLOT(RemKeyedArchiveField()));
}

void QtPropertyDataDavaKeyedArcive::AddKeyedArchiveField()
{
	QToolButton* btn = dynamic_cast<QToolButton*>(QObject::sender());
	if(NULL != archive && NULL != btn)
	{
		KeyedArchiveItemWidget *w = new KeyedArchiveItemWidget(archive, lastAddedType, GetOWViewport());
		QObject::connect(w, SIGNAL(ValueReady(const DAVA::String&, const DAVA::VariantType&)), this, SLOT(NewKeyedArchiveFieldReady(const DAVA::String&, const DAVA::VariantType&)));

		w->show();

		QRect bRect = btn->geometry();
		QPoint bPos = btn->mapToGlobal(btn->mapFromParent(bRect.topLeft()));

		QRect wRect = w->geometry();
		QPoint wPos = QPoint(bPos.x() - wRect.width() + bRect.width(), bPos.y() + bRect.height());

		w->move(wPos);
	}
}

void QtPropertyDataDavaKeyedArcive::RemKeyedArchiveField()
{
	QToolButton* btn = dynamic_cast<QToolButton*>(QObject::sender());
	if(NULL != btn && NULL != archive)
	{
		// search for child data with such button
		for(int i = 0; i < ChildCount(); ++i)
		{
			QtPropertyData *childData = ChildGet(i);
			if(NULL != childData)
			{
				// search btn thought this child optional widgets
				for (int j = 0; j < childData->GetButtonsCount(); j++)
				{
					if(btn == childData->GetButton(j))
					{
						if(NULL != lastCommand)
						{
							delete lastCommand;
						}

						lastCommand = new KeyeadArchiveRemValueCommand(archive, childData->GetName().toStdString());
						archive->DeleteKey(childData->GetName().toStdString());

						ChildRemove(childData);
						EmitDataChanged(QtPropertyData::VALUE_EDITED);
						break;
					}
				}
				
			}
		}
	}
}

void QtPropertyDataDavaKeyedArcive::NewKeyedArchiveFieldReady(const DAVA::String &key, const DAVA::VariantType &value)
{
	DVASSERT(value.type != DAVA::VariantType::TYPE_NONE && value.type < DAVA::VariantType::TYPES_COUNT);
	if(NULL != archive)
	{
		archive->SetVariant(key, value);
		lastAddedType = value.type;

		if(NULL != lastCommand)
		{
			delete lastCommand;
		}

		lastCommand = new KeyedArchiveAddValueCommand(archive, key, value);
		ChildCreate(key.c_str(), archive->GetVariant(key));
		EmitDataChanged(QtPropertyData::VALUE_EDITED);
	}
}

void* QtPropertyDataDavaKeyedArcive::CreateLastCommand() const
{
	Command2 *command = NULL;

	if(NULL != lastCommand)
	{
		if(CMDID_KEYEDARCHIVE_REM_KEY == lastCommand->GetId())
		{
			command = new KeyeadArchiveRemValueCommand(*((KeyeadArchiveRemValueCommand *) lastCommand));
		}
		else if(CMDID_KEYEDARCHIVE_ADD_KEY == lastCommand->GetId())
		{
			command = new KeyedArchiveAddValueCommand(*((KeyedArchiveAddValueCommand *) lastCommand));
		}
	}

	return command;
}

KeyedArchiveItemWidget::KeyedArchiveItemWidget(DAVA::KeyedArchive *_arch, int defaultType, QWidget *parent /* = NULL */) 
	: QWidget(parent)
	, arch(_arch)
	, presetWidget(NULL)
{
	QGridLayout *grLayout = new QGridLayout();
	int delautTypeIndex = 0;

	if(NULL != arch)
	{
		arch->Retain();
	}

	defaultBtn = new QPushButton("Ok", this);
	keyWidget = new QLineEdit(this);
	valueWidget = new QComboBox(this);

	int j = 0;
	for (int type = (DAVA::VariantType::TYPE_NONE + 1); type < DAVA::VariantType::TYPES_COUNT; type++)
	{
		// don't allow byte array
		if(type != DAVA::VariantType::TYPE_BYTE_ARRAY)
		{
			valueWidget->addItem(DAVA::VariantType::variantNamesMap[type].variantName.c_str(), type);

			if(type == defaultType)
			{
				delautTypeIndex = j;
			}

			j++;
		}
	}
	valueWidget->setCurrentIndex(delautTypeIndex);

	int row = 0;
	grLayout->addWidget(new QLabel("Key:", this), row, 0, 1, 1);
	grLayout->addWidget(keyWidget, row, 1, 1, 2);
	grLayout->addWidget(new QLabel("Value type:", this), ++row, 0, 1, 1);
	grLayout->addWidget(valueWidget, row, 1, 1, 2);

	const Vector<String> &presetValues = EditorConfig::Instance()->GetProjectPropertyNames();
	if(presetValues.size() > 0)
	{
		presetWidget = new QComboBox(this);
		
		presetWidget->addItem("None", DAVA::VariantType::TYPE_NONE);
		for(size_t i = 0; i < presetValues.size(); ++i)
		{
			presetWidget->addItem(presetValues[i].c_str(), EditorConfig::Instance()->GetPropertyValueType(presetValues[i]));
		}

		grLayout->addWidget(new QLabel("Preset:", this), ++row, 0, 1, 1);
		grLayout->addWidget(presetWidget, row, 1, 1, 2);

		QObject::connect(presetWidget, SIGNAL(activated(int)), this, SLOT(PreSetSelected(int)));
	}
    presetWidget->setMaxVisibleItems(presetWidget->count());

	grLayout->addWidget(defaultBtn, ++row, 2, 1, 1);

	grLayout->setMargin(5);
	grLayout->setSpacing(3);
	setLayout(grLayout);

	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
	setWindowOpacity(0.95);

	QObject::connect(defaultBtn, SIGNAL(pressed()), this, SLOT(OkKeyPressed()));
}

KeyedArchiveItemWidget::~KeyedArchiveItemWidget()
{
	if(NULL != arch)
	{
		arch->Release();
	} 
}

void KeyedArchiveItemWidget::showEvent(QShowEvent * event)
{
	QWidget::showEvent(event);
	keyWidget->setFocus();
}

void KeyedArchiveItemWidget::keyPressEvent(QKeyEvent *e)
{
	if (!e->modifiers() || (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter)) 
	{
		switch(e->key())
		{
		case Qt::Key_Enter:
		case Qt::Key_Return:
			defaultBtn->click();
			break;
		case Qt::Key_Escape:
			this->deleteLater();
			break;
		default:
			e->ignore();
			return;
		}
	} 
	else 
	{
		e->ignore();
	}
}

void KeyedArchiveItemWidget::OkKeyPressed()
{
	if(NULL != arch)
	{
		DAVA::String key = keyWidget->text().toStdString();

		if(key.empty())
		{
			// TODO:
			// other way to report error without losing focus
			// ...
			// 

			QMessageBox::warning(NULL, "Wrong key value", "Key value can't be empty");
		}
		else if(arch->IsKeyExists(key))
		{
			// TODO:
			// other way to report error without losing focus
			// ...
			// 

			QMessageBox::warning(NULL, "Wrong key value", "That key already exists");
		}
		else
		{
			// preset?
			int presetType = DAVA::VariantType::TYPE_NONE;
			if(NULL != presetWidget)
			{
				presetType = presetWidget->itemData(presetWidget->currentIndex()).toInt();
			}
			
			if(DAVA::VariantType::TYPE_NONE != presetType)
			{
				DAVA::VariantType presetValue = *(EditorConfig::Instance()->GetPropertyDefaultValue(key));
				emit ValueReady(key, presetValue);
			}
			else
			{
				emit ValueReady(key, DAVA::VariantType::FromType(valueWidget->itemData(valueWidget->currentIndex()).toInt()));
			}

			this->deleteLater();
		}
	}
	else
	{
		this->deleteLater();
	}
}

void KeyedArchiveItemWidget::PreSetSelected(int index)
{
	if(presetWidget->itemData(index).toInt() != DAVA::VariantType::TYPE_NONE)
	{
		keyWidget->setText(presetWidget->itemText(index));
		keyWidget->setEnabled(false);
		valueWidget->setEnabled(false);
	}
	else
	{
		keyWidget->setText("");
		keyWidget->setEnabled(true);
		valueWidget->setEnabled(true);
	}
}