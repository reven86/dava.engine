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


#include "SettingsDialog.h"
#include "SettingsManager.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataKeyedArchiveMember.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialogButtonBox>

SettingsDialog::SettingsDialog(QWidget* parent) 
: QDialog(parent)
{
    setWindowFlags(Qt::Tool);
    setWindowTitle("ResourceEditor Settings");
    
    QVBoxLayout *dlgLayout = new QVBoxLayout();
    editor = new QtPropertyEditor(this);

    QPushButton *defaultsBtn = new QPushButton("Defaults");
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    buttonBox->addButton(defaultsBtn, QDialogButtonBox::ResetRole);

    QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(defaultsBtn, SIGNAL(pressed()), this, SLOT(OnResetPressed()));

    dlgLayout->setMargin(5);
    dlgLayout->addWidget(editor);
    dlgLayout->addWidget(buttonBox);
    setLayout(dlgLayout);

    InitProperties();

    posSaver.Attach(this, "SettingsDialog");
	DAVA::VariantType v = posSaver.LoadValue("splitPos");
	if(v.GetType() == DAVA::VariantType::TYPE_INT32) editor->header()->resizeSection(0, v.AsInt32());
}

SettingsDialog::~SettingsDialog()
{
    DAVA::VariantType v(editor->header()->sectionSize(0));
	posSaver.SaveValue("splitPos", v);
}

void SettingsDialog::InitProperties()
{
    editor->RemovePropertyAll();

    for(size_t i = 0; i < SettingsManager::GetSettingsCount(); ++i)
    {
        DAVA::FastName name = SettingsManager::GetSettingsName(i);
        SettingsNode *node = SettingsManager::GetSettingsNode(name);

        DAVA::String key;
        QVector<QString> keys;

        std::stringstream ss(name.c_str());
        while(std::getline(ss, key, Settings::Delimiter))
        {
            keys.push_back(key.c_str());
        }

        if( keys.size() > 0 && 
            keys[0] != QString(Settings::InternalGroup.c_str())) // skip internal settings
        {
            // go deep into tree to find penultimate propertyData
            QtPropertyData *parent = editor->GetRootProperty();
            for(int i = 0; i < keys.size() - 1; ++i)
            {
                QtPropertyData *prop = parent->ChildGet(keys[i]);
                if(NULL == prop)
                {
                    prop = new QtPropertyData();
                    QFont boldFont = prop->GetFont();
				    boldFont.setBold(true);
                    prop->SetFont(boldFont);
				    prop->SetBackground(QBrush(QColor(Qt::lightGray)));
                    prop->SetEnabled(false);

                    parent->ChildAdd(keys[i], prop);
                }

                parent = prop;
            }

            if(NULL != parent)
            {
                QtPropertyDataSettingsNode *settingProp = new QtPropertyDataSettingsNode(name);
                settingProp->SetInspDescription(node->desc);

                parent->ChildAdd(keys.last(), settingProp);
            }
        }
    }

    editor->expandToDepth(0);
}

void SettingsDialog::OnResetPressed()
{
    if(QMessageBox::Yes == QMessageBox::question(this, "Reseting settings", "Are you sure you want to reset settings to their default values?", (QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes))
    {
        SettingsManager::ResetToDefault();
        InitProperties();
    }
}

QtPropertyDataSettingsNode::QtPropertyDataSettingsNode(DAVA::FastName path)
: QtPropertyDataDavaVariant(DAVA::VariantType())
, settingPath(path)
{ 
    SetVariantValue(SettingsManager::GetValue(path));
}

QtPropertyDataSettingsNode::~QtPropertyDataSettingsNode()
{ }

void QtPropertyDataSettingsNode::SetValueInternal(const QVariant &value)
{
    QtPropertyDataDavaVariant::SetValueInternal(value);

	// also save value to settings
    SettingsManager::SetValue(settingPath, QtPropertyDataDavaVariant::GetVariantValue());
}

bool QtPropertyDataSettingsNode::UpdateValueInternal()
{
	bool ret = false;

	// load current value from meta-object
	// we should do this because meta-object may change at any time 
	DAVA::VariantType v = SettingsManager::GetValue(settingPath);

	// if current variant value not equal to the real meta-object value
	// we should update current variant value
	if(v != GetVariantValue())
	{
		QtPropertyDataDavaVariant::SetVariantValue(v);
		ret = true;
	}

	return ret;
}

bool QtPropertyDataSettingsNode::EditorDoneInternal(QWidget *editor)
{
	bool ret = QtPropertyDataDavaVariant::EditorDoneInternal(editor);

	// if there was some changes in current value, done by editor
	// we should save them into meta-object
	if(ret)
	{
        SettingsManager::SetValue(settingPath, QtPropertyDataDavaVariant::GetVariantValue());
	}

	return ret;
}


