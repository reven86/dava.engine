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


#ifndef STATEPROPERTYGRIDWIDGET_H
#define STATEPROPERTYGRIDWIDGET_H

#include <QWidget>

#include "basepropertygridwidget.h"
#include "StateComboBoxItemDelegate.h"

namespace Ui {
class StatePropertyGridWidget;
}

class QListWidgetItem;
class StatePropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT
    
public:
    explicit StatePropertyGridWidget(QWidget *parent = 0);
    ~StatePropertyGridWidget();

    // Initialize with the metatada assigned.
    virtual void Initialize(BaseMetadata* metaData);
    virtual void Cleanup();

signals:
    void SelectedStateChanged(UIControl::eControlState newState);
	void SelectMultiplyStates(Vector<UIControl::eControlState> selectedStates);
	void SizeChanged(bool expanded);

protected slots:
	void OnExpandButtonClicked();
	void OnSelectAllStateChanged(int state);
	void OnListItemChanged(QListWidgetItem* item);
    void OnCurrrentIndexChanged(int index);

protected:
    // Fill the combobox with the states.
    void FillStatesList();

    // These methods are called when property change is succeeded/failed.
    virtual void HandleChangePropertySucceeded(const QString& propertyName);
    virtual void HandleChangePropertyFailed(const QString& propertyName);
	virtual void OnPropertiesChangedFromExternalSource() {};

    // Markup the "dirty" states in the States combo.
    void MarkupDirtyStates();

	void SetExpandState(bool expanded);

	bool eventFilter(QObject *target, QEvent *event);

private:
	enum eCheckedState
	{
		STATE_EVERY_ITEM_UNCHECKED = 0,
		STATE_EVERY_ITEM_CHECKED,
		STATE_PARTIALLY_CHECKED,
		STATE_COUNT
	};

	bool expanded;

	void UpdateState();
	void SetAllChecked(Qt::CheckState state);
	void MultiplyStateSelectionChanged();
	eCheckedState GetCheckedState();

    Ui::StatePropertyGridWidget *ui;
    StateComboBoxItemDelegate stateComboboxItemDelegate;
};

#endif // STATEPROPERTYGRIDWIDGET_H
