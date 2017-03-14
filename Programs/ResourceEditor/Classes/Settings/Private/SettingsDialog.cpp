#include "Classes/Settings/Private/SettingsDialog.h"
#include "Classes/Settings/SettingsManager.h"

#include "Scene/SceneSignals.h"
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

    QVBoxLayout* dlgLayout = new QVBoxLayout();
    editor = new QtPropertyEditor(this);

    QPushButton* defaultsBtn = new QPushButton("Defaults");
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    buttonBox->addButton(defaultsBtn, QDialogButtonBox::ResetRole);

    QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(defaultsBtn, SIGNAL(pressed()), this, SLOT(OnResetPressed()));
    QObject::connect(SceneSignals::Instance(), &SceneSignals::ThemeChanged, this, &SettingsDialog::InitProperties);

    dlgLayout->setMargin(5);
    dlgLayout->addWidget(editor);
    dlgLayout->addWidget(buttonBox);
    setLayout(dlgLayout);

    InitProperties();

    posSaver.Attach(this, "SettingsDialog");
    DAVA::VariantType v = posSaver.LoadValue("splitPos");
    if (v.GetType() == DAVA::VariantType::TYPE_INT32)
        editor->header()->resizeSection(0, v.AsInt32());
}

SettingsDialog::~SettingsDialog()
{
    DAVA::VariantType v(editor->header()->sectionSize(0));
    posSaver.SaveValue("splitPos", v);
}

void SettingsDialog::InitProperties()
{
    editor->RemovePropertyAll();

    for (size_t i = 0; i < SettingsManager::GetSettingsCount(); ++i)
    {
        DAVA::FastName name = SettingsManager::GetSettingsName(i);
        SettingsNode* node = SettingsManager::GetSettingsNode(name);

        DAVA::String key;
        DAVA::Vector<DAVA::FastName> keys;

        std::stringstream ss(name.c_str());
        while (std::getline(ss, key, Settings::Delimiter))
        {
            keys.emplace_back(key);
        }

        if (keys.empty() == false && keys[0] != Settings::InternalGroup) // skip internal settings
        {
            // go deep into tree to find penultimate propertyData
            QtPropertyData* parent = editor->GetRootProperty();
            for (size_t i = 0; i < keys.size() - 1; ++i)
            {
                const DAVA::FastName& currentKey = keys[i];
                QtPropertyData* prop = parent->ChildGet(currentKey);
                if (NULL == prop)
                {
                    prop = new QtPropertyData(currentKey);
                    QFont boldFont = prop->GetFont();
                    boldFont.setBold(true);
                    prop->SetFont(boldFont);
                    prop->SetBackground(palette().alternateBase());
                    prop->SetEnabled(false);

                    parent->ChildAdd(std::unique_ptr<QtPropertyData>(prop));
                }

                parent = prop;
            }

            if (NULL != parent)
            {
                QtPropertyDataSettingsNode* settingProp = new QtPropertyDataSettingsNode(name, keys.back());
                settingProp->SetInspDescription(node->desc);

                parent->ChildAdd(std::unique_ptr<QtPropertyData>(settingProp));
            }
        }
    }

    editor->GetRootProperty()->FinishTreeCreation();
    editor->expandToDepth(0);
}

void SettingsDialog::OnResetPressed()
{
    if (QMessageBox::Yes == QMessageBox::question(this, "Reseting settings", "Are you sure you want to reset settings to their default values?", (QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes))
    {
        SettingsManager::ResetToDefault();
        InitProperties();
    }
}

QtPropertyDataSettingsNode::QtPropertyDataSettingsNode(const DAVA::FastName& path, const DAVA::FastName& name)
    : QtPropertyDataDavaVariant(name, DAVA::VariantType())
    , settingPath(path)
{
    SetVariantValue(SettingsManager::GetValue(settingPath));
}

QtPropertyDataSettingsNode::~QtPropertyDataSettingsNode()
{
}

void QtPropertyDataSettingsNode::SetValueInternal(const QVariant& value)
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
    if (v != GetVariantValue())
    {
        QtPropertyDataDavaVariant::SetVariantValue(v);
        ret = true;
    }

    return ret;
}

bool QtPropertyDataSettingsNode::EditorDoneInternal(QWidget* editor)
{
    bool ret = QtPropertyDataDavaVariant::EditorDoneInternal(editor);

    // if there was some changes in current value, done by editor
    // we should save them into meta-object
    if (ret)
    {
        SettingsManager::SetValue(settingPath, QtPropertyDataDavaVariant::GetVariantValue());
    }

    return ret;
}
