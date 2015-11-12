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


#include "ColorPropertyDelegate.h"
#include <QToolButton>
#include <QPainter>
#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QColorDialog>

#include "PropertiesTreeItemDelegate.h"
#include "PropertiesModel.h"
#include "Utils/QtDavaConvertion.h"


namespace
{
    QPixmap CreateIcon(const QColor &color)
    {
        QPixmap pix(16, 16);
        QPainter p(&pix);
        p.setPen(QColor(0, 0, 0, 0));

        if (color.alpha() < 255)
        {
            p.setBrush(QColor(250, 250, 250));
            p.drawRect(QRect(0, 0, 15, 15));
            p.setPen(QColor(200, 200, 200));
            p.setBrush(QColor(150, 150, 150));
            p.drawRect(QRect(0, 0, 7, 7));
            p.drawRect(QRect(8, 8, 15, 15));
        }

        p.setBrush(QBrush(color));
        p.drawRect(QRect(0, 0, 15, 15));
        return pix;
    }
}

ColorPropertyDelegate::ColorPropertyDelegate(PropertiesTreeItemDelegate *delegate)
    : BasePropertyDelegate(delegate)
{
}

ColorPropertyDelegate::~ColorPropertyDelegate()
{

}

QWidget *ColorPropertyDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index )
{
    lineEdit = new QLineEdit(parent); //will be deleted outside this class
    lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
    QRegExpValidator *validator = new QRegExpValidator();
    validator->setRegExp(QRegExp("#?([A-F0-9]{8}|[A-F0-9]{6})", Qt::CaseInsensitive));
    lineEdit->setValidator(validator);

    connect(lineEdit, &QLineEdit::editingFinished, this, &ColorPropertyDelegate::OnEditingFinished);
    connect(lineEdit, &QLineEdit::textChanged, this, &ColorPropertyDelegate::OnTextChanged);
    return lineEdit;
}

void ColorPropertyDelegate::enumEditorActions( QWidget *parent, const QModelIndex &index, QList<QAction *> &actions)
{
    BasePropertyDelegate::enumEditorActions(parent, index, actions);

    chooseColorAction = new QAction(parent); //will be deleted outside this class
    connect(chooseColorAction, SIGNAL(triggered(bool)), this, SLOT(OnChooseColorClicked()));
    actions.push_front(chooseColorAction);
}

void ColorPropertyDelegate::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{
    DVASSERT(nullptr != lineEdit);
    QColor color = ColorToQColor(index.data(Qt::EditRole).value<DAVA::VariantType>().AsColor());
    lineEdit->setText(QColorToHex(color));
    lineEdit->setProperty("color", color);
}

bool ColorPropertyDelegate::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (BasePropertyDelegate::setModelData(editor, model, index))
        return true;

    DVASSERT(nullptr != lineEdit);

    QColor newColor = HexToQColor(lineEdit->text());
    DAVA::VariantType color( QColorToColor(newColor) );
    QVariant colorVariant;
    colorVariant.setValue<DAVA::VariantType>(color);
    return model->setData(index, colorVariant, Qt::EditRole);
}

void ColorPropertyDelegate::OnChooseColorClicked()
{
    DVASSERT(nullptr != lineEdit);
    QWidget *editor = lineEdit->parentWidget();
    DVASSERT(nullptr != editor)

    QColorDialog dlg(editor);

    dlg.setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
    dlg.setCurrentColor(lineEdit->property("color").value<QColor>());

    if (dlg.exec() == QDialog::Accepted)
    {
        lineEdit->setText(QColorToHex(dlg.selectedColor()));
        lineEdit->setProperty("color", dlg.selectedColor());
        BasePropertyDelegate::SetValueModified(editor, true);
        itemDelegate->emitCommitData(editor);
    }
}

void ColorPropertyDelegate::OnEditingFinished()
{
    DVASSERT(nullptr != lineEdit);
    QWidget *editor = lineEdit->parentWidget();
    DVASSERT(nullptr != editor);

    BasePropertyDelegate::SetValueModified(editor, lineEdit->isModified());
    itemDelegate->emitCommitData(editor);
}

void ColorPropertyDelegate::OnTextChanged(const QString& text)
{
    QColor color(HexToQColor(text));
    chooseColorAction->setIcon(CreateIcon(color));
    QPalette palette(lineEdit->palette());
    int pos = -1;
    QString textCopy(text);
    bool valid = lineEdit->validator()->validate(textCopy, pos) == QValidator::Acceptable;
    palette.setColor(QPalette::Text, valid ? Qt::black : Qt::red);
    lineEdit->setPalette(palette);
}
