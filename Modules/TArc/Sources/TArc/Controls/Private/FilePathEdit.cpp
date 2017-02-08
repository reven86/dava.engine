#include "TArc/Controls/FilePathEdit.h"
#include "TArc/Controls/Private/TextValidator.h"

#include <Base/FastName.h>
#include <Utils/StringFormat.h>
#include <Reflection/ReflectedMeta.h>

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QToolTip>

namespace DAVA
{
namespace TArc
{
FilePathEdit::FilePathEdit(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxy<QWidget>(ControlDescriptor(params.fields), wrappersProcessor, model, parent)
    , ui(params.ui)
    , wndKey(params.wndKey)
{
    SetupControl();
}

FilePathEdit::FilePathEdit(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxy<QWidget>(ControlDescriptor(params.fields), accessor, model, parent)
    , ui(params.ui)
    , wndKey(params.wndKey)
{
    SetupControl();
}

void FilePathEdit::SetupControl()
{
    edit = new QLineEdit(this);
    edit->setObjectName("filePathEdit");

    button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setIcon(QIcon(":/TArc/Resources/openfile.png"));
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setObjectName("openFileDialogButton");

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(edit);
    layout->addWidget(button);

    connections.AddConnection(edit, &QLineEdit::editingFinished, MakeFunction(this, &FilePathEdit::EditingFinished));
    connections.AddConnection(button, &QToolButton::clicked, MakeFunction(this, &FilePathEdit::ButtonClicked));

    TextValidator* validator = new TextValidator(this, this);
    edit->setValidator(validator);
}

void FilePathEdit::EditingFinished()
{
    if (!edit->isReadOnly())
    {
        FilePath path(edit->text().toStdString());
        M::ValidationResult validation = FixUp(path);
        if (validation.state == M::ValidationResult::eState::Valid)
        {
            wrapper.SetFieldValue(GetFieldName(Fields::Value), path);
            if (path.GetStringValue() != path.GetAbsolutePathname())
            {
                edit->setText(QString::fromStdString(path.GetAbsolutePathname()));
            }
        }
        else
        {
            Reflection r = model.GetField(GetFieldName(Fields::Value));
            DVASSERT(r.IsValid());
            edit->setText(QString::fromStdString(r.GetValue().Cast<FilePath>().GetAbsolutePathname()));
            if (validation.message.empty() == false)
            {
                ShowHint(QString::fromStdString(validation.message));
            }
        }
    }
}

void FilePathEdit::ButtonClicked()
{
    if (!edit->isReadOnly())
    {
        bool isFile;
        bool shouldExists;
        QString filters;
        ExtractMetaInfo(isFile, shouldExists, filters);

        QString path;
        if (isFile == true)
        {
            FileDialogParams params;
            params.dir = edit->text();
            params.filters = filters;
            params.title = QString::fromStdString(GetFieldValue<String>(Fields::DialogTitle, "Open File"));

            path = ui->GetOpenFileName(wndKey, params);
        }
        else
        {
            DirectoryDialogParams params;
            params.dir = edit->text();
            params.title = QString::fromStdString(GetFieldValue<String>(Fields::DialogTitle, "Open directory"));
            path = ui->GetExistingDirectory(wndKey, params);
        }

        if (path.isEmpty() == false)
        {
            edit->setText(path);
            EditingFinished();
        }
    }
}

void FilePathEdit::UpdateControl(const ControlDescriptor& descriptor)
{
    bool readOnlyChanged = descriptor.IsChanged(Fields::IsReadOnly);
    bool textChanged = descriptor.IsChanged(Fields::Value);
    if (readOnlyChanged || textChanged)
    {
        edit->setReadOnly(IsValueReadOnly(descriptor, Fields::Value, Fields::IsReadOnly));

        if (textChanged)
        {
            DAVA::Reflection fieldValue = model.GetField(descriptor.GetName(Fields::Value));
            DVASSERT(fieldValue.IsValid());
            edit->setText(QString::fromStdString(fieldValue.GetValue().Cast<FilePath>().GetAbsolutePathname()));
        }
    }

    if (descriptor.IsChanged(Fields::IsEnabled))
    {
        edit->setEnabled(GetFieldValue<bool>(Fields::IsEnabled, true));
    }

    if (descriptor.IsChanged(Fields::PlaceHolder))
    {
        edit->setPlaceholderText(QString::fromStdString(GetFieldValue<String>(Fields::PlaceHolder, "")));
    }

    button->setEnabled(edit->isReadOnly() == false && edit->isEnabled() == true);
}

M::ValidationResult FilePathEdit::FixUp(const Any& value) const
{
    M::ValidationResult result;
    result.state = M::ValidationResult::eState::Valid;

    bool isFile;
    bool shouldExists;
    QString filters;
    ExtractMetaInfo(isFile, shouldExists, filters);

    FilePath path(value.Cast<FilePath>());
    bool isDir = path.IsDirectoryPathname();
    if (isFile == true && isDir == true)
    {
        result.state = M::ValidationResult::eState::Invalid;
        result.message = "Should be a file";
    }
    else if (isFile == false && isDir == false)
    {
        result.state = M::ValidationResult::eState::Invalid;
        result.message = "Should be a directory";
    }
    else if (shouldExists == true && path.Exists() == false)
    {
        result.state = M::ValidationResult::eState::Invalid;
        result.message = "Path should exist";
    }

    FilePath rootDir = GetFieldValue<FilePath>(Fields::RootDirectory, FilePath());
    if (!rootDir.IsEmpty() && !FilePath::ContainPath(path, rootDir))
    {
        result.state = M::ValidationResult::eState::Invalid;
        result.message = Format("Base folder is : %s.\nYour path should be inside it or inside it's subdirectoryes", rootDir.GetAbsolutePathname().c_str());
    }

    return result;
}

M::ValidationResult FilePathEdit::Validate(const Any& value) const
{
    Reflection field = model.GetField(GetFieldName(Fields::Value));
    DVASSERT(field.IsValid());

    const M::Validator* validator = field.GetMeta<M::Validator>();
    if (validator != nullptr)
    {
        return validator->Validate(value, field.GetValue());
    }

    M::ValidationResult r;
    r.state = M::ValidationResult::eState::Valid;
    return r;
}

void FilePathEdit::ExtractMetaInfo(bool& isFile, bool& shouldExists, QString& filters) const
{
    Reflection r = model.GetField(GetFieldName(Fields::Value));
    DVASSERT(r.IsValid());

    isFile = true;
    shouldExists = true;
    const M::File* fileMeta = r.GetMeta<M::File>();
    const M::Directory* dirMeta = r.GetMeta<M::Directory>();
    if (fileMeta != nullptr)
    {
        shouldExists = fileMeta->shouldExists;
        filters = QString::fromStdString(fileMeta->filters);
    }
    else if (dirMeta != nullptr)
    {
        isFile = false;
        shouldExists = dirMeta->shouldExists;
    }

    const FastName& filtersName = GetFieldName(Fields::Filters);
    if (filtersName.IsValid() && filters.isEmpty())
    {
        Reflection filtersField = model.GetField(filtersName);
        DVASSERT(filtersField.IsValid());
        filters = QString::fromStdString(filtersField.GetValue().Cast<String>());
    }
}

void FilePathEdit::ShowHint(const QString& message)
{
    QPoint pos = mapToGlobal(QPoint(0, 0));
    QToolTip::showText(pos, message, this);
}

} // namespace TArc
} // namespace DAVA
