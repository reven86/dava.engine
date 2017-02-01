#include "FMODEventPropertyDelegate.h"
#include <DAVAEngine.h>
#include <QAction>
#include <QLineEdit>
#include <QApplication>
#include <QMap>

#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include "QtTools/FMODSoundBrowser/FMODSoundBrowser.h"
#include "Project/Project.h"

#include "Engine/Engine.h"

using namespace DAVA;

FMODEventPropertyDelegate::FMODEventPropertyDelegate(PropertiesTreeItemDelegate* delegate)
    : BasePropertyDelegate(delegate)

{
}

FMODEventPropertyDelegate::~FMODEventPropertyDelegate()
{
}

QWidget* FMODEventPropertyDelegate::createEditor(QWidget* parent, const PropertiesContext& context, const QStyleOptionViewItem&, const QModelIndex&)
{
    SoundSystem::Instance()->GetAllEventsNames(eventNames);

    lineEdit = new QLineEdit(parent);
    lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
    connect(lineEdit, &QLineEdit::editingFinished, this, &FMODEventPropertyDelegate::OnEditingFinished);
    connect(lineEdit, &QLineEdit::textChanged, this, &FMODEventPropertyDelegate::OnTextChanged);
    return lineEdit;
}

void FMODEventPropertyDelegate::setEditorData(QWidget*, const QModelIndex& index) const
{
    VariantType variant = index.data(Qt::EditRole).value<VariantType>();
    const FastName& fn = variant.AsFastName();
    const QString& stringValue = StringToQString(fn.IsValid() ? fn.c_str() : "");
    DVASSERT(!lineEdit.isNull());
    lineEdit->setText(stringValue);
}

bool FMODEventPropertyDelegate::setModelData(QWidget* rawEditor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    VariantType variantType = index.data(Qt::EditRole).value<VariantType>();
    DVASSERT(!lineEdit.isNull());
    if (!lineEdit->text().isEmpty())
    {
        variantType.SetFastName(FastName(lineEdit->text().toStdString()));
    }
    else
    {
        variantType.SetFastName(FastName());
    }
    QVariant variant;
    variant.setValue<VariantType>(variantType);

    return model->setData(index, variant, Qt::EditRole);
}

void FMODEventPropertyDelegate::enumEditorActions(QWidget* parent, const QModelIndex& index, QList<QAction*>& actions)
{
    QAction* selectEventAction = new QAction(tr("..."), parent);
    selectEventAction->setToolTip(tr("Select sound event"));
    actions.push_back(selectEventAction);
    connect(selectEventAction, SIGNAL(triggered(bool)), this, SLOT(selectEventClicked()));

    QAction* clearEventAction = new QAction(QIcon(":/Icons/editclear.png"), tr("clear"), parent);
    clearEventAction->setToolTip(tr("Clear sound event"));
    actions.push_back(clearEventAction);
    connect(clearEventAction, SIGNAL(triggered(bool)), this, SLOT(clearEventClicked()));

    BasePropertyDelegate::enumEditorActions(parent, index, actions);
}

void FMODEventPropertyDelegate::selectEventClicked()
{
    DVASSERT(!lineEdit.isNull());
    QWidget* editor = lineEdit->parentWidget();
    DVASSERT(editor != nullptr);

    FMODSoundBrowser* soundBrowser = new FMODSoundBrowser(editor->parentWidget());
    soundBrowser->deleteLater();

    QString pathText = lineEdit->text();
    if (!pathText.isEmpty())
    {
        soundBrowser->SetCurrentEvent(pathText.toStdString());
    }

    if (soundBrowser->exec() == QDialog::Accepted)
    {
        const String& selectedEventName = soundBrowser->GetSelectSoundEvent();

        lineEdit->setText(StringToQString(selectedEventName));

        BasePropertyDelegate::SetValueModified(editor, true);
        itemDelegate->emitCommitData(editor);
    }
}

void FMODEventPropertyDelegate::clearEventClicked()
{
    DVASSERT(!lineEdit.isNull());
    QWidget* editor = lineEdit->parentWidget();
    DVASSERT(editor != nullptr);
    lineEdit->setText(QString(""));

    BasePropertyDelegate::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}

void FMODEventPropertyDelegate::OnEditingFinished()
{
    DVASSERT(!lineEdit.isNull());
    if (!lineEdit->isModified())
    {
        return;
    }
    QWidget* editor = lineEdit->parentWidget();
    DVASSERT(editor != nullptr);
    const QString& text = lineEdit->text();
    if (!text.isEmpty() && !IsPathValid(text))
    {
        return;
    }
    BasePropertyDelegate::SetValueModified(editor, lineEdit->isModified());
    itemDelegate->emitCommitData(editor);
    lineEdit->setModified(false);
}

void FMODEventPropertyDelegate::OnTextChanged(const QString& text)
{
    QPalette palette(lineEdit->palette());
    QString textCopy(text);

    QColor globalTextColor = qApp->palette().color(QPalette::Text);
    QColor nextColor = IsPathValid(text) ? globalTextColor : Qt::red;
    palette.setColor(QPalette::Text, nextColor);
    lineEdit->setPalette(palette);
}

bool FMODEventPropertyDelegate::IsPathValid(const QString& path)
{
    return std::find(eventNames.begin(), eventNames.end(), path.toStdString()) != eventNames.end();
}
