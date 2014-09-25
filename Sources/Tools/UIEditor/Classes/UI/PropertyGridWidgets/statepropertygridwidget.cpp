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


#include "statepropertygridwidget.h"
#include "ui_statepropertygridwidget.h"

#include "UIControlStateHelper.h"
#include "PropertiesGridController.h"
#include <QList>
#include <QEvent>
#include <QKeyEvent>

static const QString STATE_PROPERTY_BLOCK_NAME = "State";

StatePropertyGridWidget::StatePropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
	expanded(false),
    ui(new Ui::StatePropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(STATE_PROPERTY_BLOCK_NAME);
    ui->stateSelectComboBox->setItemDelegate(&stateComboboxItemDelegate);

	UpdateState();

	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);

	ui->stateSelectComboBox->installEventFilter(this);
	ui->stateSelectListWidget->installEventFilter(this);
}

StatePropertyGridWidget::~StatePropertyGridWidget()
{
    delete ui;
}

bool StatePropertyGridWidget::eventFilter(QObject *target, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = (QKeyEvent*)event;

		if (target == ui->stateSelectComboBox && keyEvent->key() == Qt::Key_Right)
		{
			SetExpandState(true);
			return true;
		}

		if (target == ui->stateSelectListWidget && keyEvent->key() == Qt::Key_Left)
		{
			SetExpandState(false);
			return true;
		}
	}

	return BasePropertyGridWidget::eventFilter(target, event);
}

void StatePropertyGridWidget::UpdateState()
{
	ui->stateSelectComboBox->setVisible(!expanded);
	ui->stateSelectListWidget->setVisible(expanded);
	ui->selectAllCheckbox->setVisible(expanded);

	if (expanded)
	{
		ui->expandButton->setText("-");
		setMinimumHeight(ui->stateSelectListWidget->height() + ui->expandButton->height() + 30);
		ui->stateSelectListWidget->setFocus();
	}
	else
	{
		ui->expandButton->setText("+");
		setMinimumHeight(ui->stateSelectComboBox->height() + ui->expandButton->height() + 30);
		ui->stateSelectComboBox->setFocus();
	}
}

void StatePropertyGridWidget::OnSelectAllStateChanged(int state)
{
	ui->stateSelectListWidget->blockSignals(true);
	SetAllChecked((Qt::CheckState)state);
	ui->stateSelectListWidget->blockSignals(false);

	MultiplyStateSelectionChanged();
}

void StatePropertyGridWidget::SetAllChecked(Qt::CheckState state)
{
	for (int32 i = 0; i < ui->stateSelectListWidget->count(); ++i)
	{
		QListWidgetItem* item = ui->stateSelectListWidget->item(i);
		item->setCheckState(state);
	}

	if (GetCheckedState() == STATE_EVERY_ITEM_UNCHECKED)
	{
		ui->stateSelectListWidget->item(0)->setCheckState(Qt::Checked);
	}
}

StatePropertyGridWidget::eCheckedState StatePropertyGridWidget::GetCheckedState()
{
	eCheckedState state = STATE_PARTIALLY_CHECKED;

	bool everyItemUnchecked = true;
	bool everyItemChecked = true;

	for (int32 i = 0; i < ui->stateSelectListWidget->count(); ++i)
	{
		QListWidgetItem* item = ui->stateSelectListWidget->item(i);

		everyItemChecked &= (item->checkState() == Qt::Checked);
		everyItemUnchecked &= (item->checkState() == Qt::Unchecked);
	}

	if (everyItemChecked)
	{
		state = STATE_EVERY_ITEM_CHECKED;
	}
	if (everyItemUnchecked)
	{
		state = STATE_EVERY_ITEM_UNCHECKED;
	}

	return state;
}

void StatePropertyGridWidget::SetExpandState(bool expanded)
{
	this->expanded = expanded;
	UpdateState();
	
	if (expanded)
	{
		MultiplyStateSelectionChanged();
	}
	else
	{
		OnCurrrentIndexChanged(ui->stateSelectComboBox->currentIndex());
	}
}

void StatePropertyGridWidget::OnExpandButtonClicked()
{
	SetExpandState(!expanded);
}

void StatePropertyGridWidget::MultiplyStateSelectionChanged()
{
	Vector<UIControl::eControlState> selectedStates;
	for (int32 i = 0; i < ui->stateSelectListWidget->count(); ++i)
	{
		QListWidgetItem* item = ui->stateSelectListWidget->item(i);

		if (item->checkState() == Qt::Checked)
		{
			UIControl::eControlState selectedState = UIControlStateHelper::GetUIControlStateValue(item->text());
			selectedStates.push_back(selectedState);
		}
	}

	emit SelectMultiplyStates(selectedStates);
}

void StatePropertyGridWidget::OnListItemChanged(QListWidgetItem* /*item*/)
{
	ui->stateSelectListWidget->blockSignals(true);
	ui->selectAllCheckbox->blockSignals(true);

	eCheckedState checkedState = GetCheckedState();

	switch (checkedState)
	{
		case STATE_EVERY_ITEM_CHECKED:
			ui->selectAllCheckbox->setCheckState(Qt::Checked);
			break;

		case STATE_EVERY_ITEM_UNCHECKED:
			ui->selectAllCheckbox->setCheckState(Qt::Unchecked);
			ui->stateSelectListWidget->item(0)->setCheckState(Qt::Checked);
			break;
		case STATE_PARTIALLY_CHECKED:
			ui->selectAllCheckbox->setCheckState(Qt::Unchecked);
			break;

		default:
			break;
	}

	ui->selectAllCheckbox->blockSignals(false);
	ui->stateSelectListWidget->blockSignals(false);

	MultiplyStateSelectionChanged();
}

void StatePropertyGridWidget::FillStatesList()
{
    ui->stateSelectComboBox->clear();
	ui->stateSelectListWidget->clear();
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState controlState = UIControlStateHelper::GetUIControlState(i);
		QString stateName = UIControlStateHelper::GetUIControlStateName(controlState);

        ui->stateSelectComboBox->addItem(stateName, QVariant(controlState));

		QListWidgetItem* item = new QListWidgetItem(stateName);
		item->setData(Qt::UserRole, QVariant(controlState));
		item->setCheckState(Qt::Unchecked);
		if (controlState == UIControlStateHelper::GetDefaultControlState())
		{
			item->setCheckState(Qt::Checked);
		}
		ui->stateSelectListWidget->addItem(item);
    }

    MarkupDirtyStates();
}

// Initialize with the metatada assigned.
void StatePropertyGridWidget::Initialize(BaseMetadata* metaData)
{
    BasePropertyGridWidget::Initialize(metaData);
    FillStatesList();

    connect(this->ui->stateSelectComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(OnCurrrentIndexChanged(int)));
    connect(this, SIGNAL(SelectedStateChanged(UIControl::eControlState)),
            PropertiesGridController::Instance(),
            SLOT(OnSelectedStateChanged(UIControl::eControlState)));
	connect(this, SIGNAL(SelectMultiplyStates(Vector<UIControl::eControlState>)),
			PropertiesGridController::Instance(), SLOT(OnSelectedStatesChanged(Vector<UIControl::eControlState>)));

	connect(this->ui->expandButton, SIGNAL(pressed()),
			this, SLOT(OnExpandButtonClicked()));
	connect(this->ui->selectAllCheckbox, SIGNAL(stateChanged(int)),
			this, SLOT(OnSelectAllStateChanged(int)));
	connect(this->ui->stateSelectListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
			this, SLOT(OnListItemChanged(QListWidgetItem*)));

    // Select the first state, emit the signal to update controller.
    int selectedStateIndex = UIControlStateHelper::GetDefaultControlStateIndex();
    ui->stateSelectComboBox->setCurrentIndex(selectedStateIndex);
	ui->selectAllCheckbox->setCheckState(Qt::Unchecked);
    emit SelectedStateChanged(UIControlStateHelper::GetDefaultControlState());
}

void StatePropertyGridWidget::Cleanup()
{
    disconnect(this->ui->stateSelectComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(OnCurrrentIndexChanged(int)));
    disconnect(this, SIGNAL(SelectedStateChanged(UIControl::eControlState)),
            PropertiesGridController::Instance(),
            SLOT(OnSelectedStateChanged(UIControl::eControlState)));
	disconnect(this, SIGNAL(SelectMultiplyStates(Vector<UIControl::eControlState>)),
			PropertiesGridController::Instance(), SLOT(OnSelectedStatesChanged(Vector<UIControl::eControlState>)));

	disconnect(this->ui->expandButton, SIGNAL(pressed()),
			this, SLOT(OnExpandButtonClicked()));
	disconnect(this->ui->selectAllCheckbox, SIGNAL(stateChanged(int)),
			this, SLOT(OnSelectAllStateChanged(int)));
	disconnect(this->ui->stateSelectListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
			this, SLOT(OnListItemChanged(QListWidgetItem*)));

    BasePropertyGridWidget::Cleanup();
}

void StatePropertyGridWidget::OnCurrrentIndexChanged(int index)
{
    const QString& selectedItemName = this->ui->stateSelectComboBox->itemText(index);
    UIControl::eControlState selectedState = UIControlStateHelper::GetUIControlStateValue(selectedItemName);
    
    emit SelectedStateChanged(selectedState);
}


void StatePropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertySucceeded(propertyName);
    MarkupDirtyStates();
}

void StatePropertyGridWidget::HandleChangePropertyFailed(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertyFailed(propertyName);
    MarkupDirtyStates();
}

void StatePropertyGridWidget::MarkupDirtyStates()
{
    QList<int> markedTextIndexesList;

    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState controlState = UIControlStateHelper::GetUIControlState(i);
        if (this->activeMetadata->IsStateDirty(controlState))
        {
            markedTextIndexesList.append(i);
        }
    }

    stateComboboxItemDelegate.SetBoldTextIndexesList(markedTextIndexesList);
	
	ui->stateSelectListWidget->blockSignals(true);
	for (int32 i = 0; i < ui->stateSelectListWidget->count(); ++i)
	{
		QListWidgetItem* item = ui->stateSelectListWidget->item(i);
		QFont f = item->font();
		f.setBold(markedTextIndexesList.contains(i));
		item->setFont(f);
	}
	ui->stateSelectListWidget->blockSignals(false);
}