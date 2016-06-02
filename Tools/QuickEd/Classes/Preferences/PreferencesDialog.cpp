#include "Preferences/PreferencesDialog.h"
#include "Preferences/PreferencesModel.h"
#include "Preferences/PreferencesFilterModel.h"
#include "UI/Properties/PropertiesTreeView.h"
#include "UI/Properties/PropertiesTreeItemDelegate.h"
#include "Preferences/PreferencesStorage.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QDialogButtonBox>

REGISTER_PREFERENCES_ON_START(PreferencesDialog,
                              PREF_ARG("currentGeometry", DAVA::String()),
                              PREF_ARG("headerState", DAVA::String())
                              )

PreferencesDialog::PreferencesDialog(QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , treeView(new PropertiesTreeView(this))
{
    setWindowTitle(tr("Preferences"));
    resize(800, 600);
    QVBoxLayout* verticalLayout = new QVBoxLayout(this);
    preferencesModel = new PreferencesModel(this);
    PreferencesFilterModel* preferencesFilterModel = new PreferencesFilterModel(this);
    preferencesFilterModel->setSourceModel(preferencesModel);
    treeView->setModel(preferencesFilterModel);
    treeView->setItemDelegate(new PropertiesTreeItemDelegate());
    treeView->expandToDepth(0);
    verticalLayout->addWidget(treeView);

    QHBoxLayout* horizontalLayout = new QHBoxLayout();
    verticalLayout->addItem(horizontalLayout);
    horizontalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Maximum));
    QDialogButtonBox* dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(dialogButtonBox, &QDialogButtonBox::accepted, this, &PreferencesDialog::OnButtonBoxAccepted);
    connect(dialogButtonBox, &QDialogButtonBox::rejected, this, &PreferencesDialog::OnButtonBoxRejected);
    verticalLayout->addWidget(dialogButtonBox);

    PreferencesStorage::Instance()->RegisterPreferences(this);
}

PreferencesDialog::~PreferencesDialog()
{
    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

void PreferencesDialog::OnButtonBoxAccepted()
{
    //save all changed properties
    preferencesModel->ApplyAllChangedProperties();
    accept();
}

void PreferencesDialog::OnButtonBoxRejected()
{
    //do nothing
    reject();
}

DAVA::String PreferencesDialog::GetGeometry() const
{
    QByteArray geometry = saveGeometry();
    return geometry.toBase64().toStdString();
}

void PreferencesDialog::SetGeometry(const DAVA::String& str)
{
    QByteArray geometry = QByteArray::fromStdString(str);
    restoreGeometry(QByteArray::fromBase64(geometry));
}

DAVA::String PreferencesDialog::GetHeaderState() const
{
    QByteArray geometry = treeView->header()->saveState();
    return geometry.toBase64().toStdString();
}

void PreferencesDialog::SetHeaderState(const DAVA::String& str)
{
    QByteArray geometry = QByteArray::fromStdString(str);
    treeView->header()->restoreState(QByteArray::fromBase64(geometry));
}
