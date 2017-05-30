#include "TArc/Controls/PlainTextEdit.h"

#include <Reflection/ReflectedMeta.h>

#include <QMimeData>

namespace DAVA
{
namespace TArc
{
PlainTextEdit::PlainTextEdit(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QPlainTextEdit>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
}

PlainTextEdit::PlainTextEdit(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QPlainTextEdit>(params, ControlDescriptor(params.fields), accessor, model, parent)
{
}

void PlainTextEdit::UpdateControl(const ControlDescriptor& descriptor)
{
    bool readOnlyChanged = descriptor.IsChanged(Fields::IsReadOnly);
    bool textChanged = descriptor.IsChanged(Fields::Text);
    if (readOnlyChanged || textChanged)
    {
        DAVA::Reflection fieldValue = model.GetField(descriptor.GetName(Fields::Text));
        DVASSERT(fieldValue.IsValid());

        const M::MaxLength* lengthMeta = fieldValue.GetMeta<M::MaxLength>();
        if (lengthMeta != nullptr)
        {
            maxLength = lengthMeta->length;
        }
        else
        {
            maxLength = 0;
        }

        setReadOnly(IsValueReadOnly(descriptor, Fields::Text, Fields::IsReadOnly));

        if (textChanged)
        {
            if (maxLength > 0)
            {
                setPlainText(QString::fromStdString(fieldValue.GetValue().Cast<String>().substr(0, maxLength)));
            }
            else
            {
                setPlainText(QString::fromStdString(fieldValue.GetValue().Cast<String>()));
            }
        }
    }

    if (descriptor.IsChanged(Fields::IsEnabled))
    {
        setEnabled(GetFieldValue<bool>(Fields::IsEnabled, true));
    }

    if (descriptor.IsChanged(Fields::PlaceHolder))
    {
        setPlaceholderText(QString::fromStdString(GetFieldValue<String>(Fields::PlaceHolder, "")));
    }
}

void PlainTextEdit::keyPressEvent(QKeyEvent* e)
{
    if ((maxLength <= 0) || (toPlainText().length() < maxLength))
    {
        QPlainTextEdit::keyPressEvent(e);
    }
    else
    {
        int key = e->key();
        bool delCondition = ((key == Qt::Key_Delete) || (key == Qt::Key_Backspace) || (key == Qt::Key_Cancel));
        if (e->text().isEmpty() || delCondition)
        {
            QPlainTextEdit::keyPressEvent(e);
        }
    }
}

void PlainTextEdit::insertFromMimeData(const QMimeData* source)
{
    QMimeData scopy;
    if (source->hasColor())
    {
        scopy.setColorData(source->colorData());
    }

    if (source->hasHtml())
    {
        scopy.setHtml(source->html());
    }

    if (source->hasImage())
    {
        scopy.setImageData(source->imageData());
    }

    if (source->hasText())
    {
        QString textToPaste = source->text();
        if (maxLength > 0)
        {
            int curTextLength = toPlainText().length();
            if (curTextLength < maxLength)
            {
                int totalLength = curTextLength + textToPaste.length();
                if (totalLength > maxLength)
                {
                    int numToDelete = totalLength - maxLength;
                    scopy.setText(textToPaste.left(textToPaste.length() - numToDelete));
                }
                else
                {
                    scopy.setText(textToPaste);
                }
            }
        }
        else
        {
            scopy.setText(textToPaste);
        }
    }

    if (source->hasUrls())
    {
        scopy.setUrls(source->urls());
    }

    QPlainTextEdit::insertFromMimeData(&scopy);
}

void PlainTextEdit::focusOutEvent(QFocusEvent* e)
{
    if (isReadOnly() == false)
    {
        wrapper.SetFieldValue(GetFieldName(Fields::Text), toPlainText().toStdString());
    }

    QPlainTextEdit::focusOutEvent(e);
}

} // namespace TArc
} // namespace DAVA
