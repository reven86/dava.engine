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


#ifndef CHECKABLECOMBOBOX_H
#define CHECKABLECOMBOBOX_H


#include <QComboBox>
#include <QStandardItemModel>

class ComboBoxModel : public QStandardItemModel
{
    Q_OBJECT
public:
    ComboBoxModel(QObject *parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex & index) const override;
};


class CheckableComboBox
    : public QComboBox
{
    Q_OBJECT

signals:
    void done();

public:
    explicit CheckableComboBox(QWidget* parent = nullptr);
    ~CheckableComboBox();

    QVariantList selectedUserData() const;
    void selectUserData(const QVariantList &data);
signals:
    void selectedUserDataChanged(const QVariantList &data);
private slots:
    void onDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> &roles);
    void updateTextHints();

private:
    QModelIndexList checkedIndexes() const;

    bool eventFilter(QObject* obj, QEvent* e) override;
    void paintEvent(QPaintEvent* event) override;

    QString textHint;
};

#endif // CHECKABLECOMBOBOX_H
