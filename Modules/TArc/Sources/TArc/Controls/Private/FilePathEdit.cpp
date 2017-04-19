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
    : ControlProxyImpl<QWidget>(ControlDescriptor(params.fields), wrappersProcessor, model, parent)
    , ui(params.ui)
    , wndKey(params.wndKey)
{
    SetupControl();
}

FilePathEdit::FilePathEdit(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QWidget>(ControlDescriptor(params.fields), accessor, model, parent)
    , ui(params.ui)
    , wndKey(params.wndKey)
{
    SetupControl();
}

void FilePathEdit::SetupControl()
{
    edit = new QLineEdit(this);
    edit->setObjectName("filePathEdit");
    setFocusProxy(edit);
    setFocusPolicy(edit->focusPolicy());

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
}

void FilePathEdit::EditingFinished()
{
    RETURN_IF_MODEL_LOST(void());
    if (!edit->isReadOnly())
    {
        FilePath path(edit->text().toStdString());
        M::ValidationResult validation = Validate(path);
        ProcessValidationResult(validation, path);
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
        }
    }
}

void FilePathEdit::ButtonClicked()
{
    RETURN_IF_MODEL_LOST(void());
    if (!edit->isReadOnly())
    {
        QString path;
        if (IsFile() == true)
        {
            FileDialogParams params = GetFileDialogParams();
            path = ui->GetOpenFileName(wndKey, params);
        }
        else
        {
            DirectoryDialogParams params;
            params.dir = edit->text();
            params.title = QString::fromStdString("Open directory");
            path = ui->GetExistingDirectory(wndKey, params);
        }

        if (path.isEmpty() == false)
        {
            FilePath filePath(path.toStdString());
            M::ValidationResult result = Validate(filePath);
            ProcessValidationResult(result, filePath);

            if (result.state == M::ValidationResult::eState::Valid)
            {
                edit->setText(QString::fromStdString(filePath.GetAbsolutePathname()));
                EditingFinished();
            }
        }
    }
}

void FilePathEdit::UpdateControl(const ControlDescriptor& descriptor)
{
    RETURN_IF_MODEL_LOST(void());
    bool readOnlyChanged = descriptor.IsChanged(Fields::IsReadOnly);
    bool textChanged = descriptor.IsChanged(Fields::Value);
    if (readOnlyChanged || textChanged)
    {
        edit->setReadOnly(IsValueReadOnly(descriptor, Fields::Value, Fields::IsReadOnly));

        if (textChanged)
        {
            DAVA::Reflection fieldValue = model.GetField(descriptor.GetName(Fields::Value));
            DVASSERT(fieldValue.IsValid());
            Any value = fieldValue.GetValue();
            if (value.CanGet<FilePath>())
            {
                edit->setText(QString::fromStdString(value.Get<FilePath>().GetAbsolutePathname()));
            }
            else if (value.CanCast<String>())
            {
                edit->setText(QString::fromStdString(value.Cast<String>()));
            }
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

M::ValidationResult FilePathEdit::Validate(const Any& value) const
{
    RETURN_IF_MODEL_LOST(M::ValidationResult());
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

bool FilePathEdit::IsFile() const
{
    Reflection r = model.GetField(GetFieldName(Fields::Value));
    DVASSERT(r.IsValid());
    return r.HasMeta<M::Directory>() == false;
}

FileDialogParams FilePathEdit::GetFileDialogParams() const
{
    FileDialogParams params;
    Reflection r = model.GetField(GetFieldName(Fields::Value));
    DVASSERT(r.IsValid());
    const M::File* file = r.GetMeta<M::File>();
    if (file != nullptr)
    {
        params.dir = QString::fromStdString(file->GetDefaultPath());
        if (!edit->text().isEmpty())
        {
            params.dir = edit->text();
        }
        params.filters = QString::fromStdString(file->filters);
        params.title = QString::fromStdString(file->dlgTitle);
    }
    else
    {
        params.dir = edit->text();
        params.filters = "All(*.*)";
        params.title = "Open File";
    }

    return params;
}

void FilePathEdit::ProcessValidationResult(M::ValidationResult& validationResult, FilePath& path)
{
    if (validationResult.state == M::ValidationResult::eState::Valid)
    {
        return;
    }

    if (validationResult.message.empty() == false)
    {
        ShowHint(QString::fromStdString(validationResult.message));
    }

    if (validationResult.fixedValue.IsEmpty() == false)
    {
        path = validationResult.fixedValue.Cast<FilePath>();
        validationResult.state = M::ValidationResult::eState::Valid;
    }
}

void FilePathEdit::ShowHint(const QString& message)
{
    QPoint pos = edit->mapToGlobal(QPoint(0, 0));
    QToolTip::showText(pos, message, this);
}

} // namespace TArc
} // namespace DAVA
