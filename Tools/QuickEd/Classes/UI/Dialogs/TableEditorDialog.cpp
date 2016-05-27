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


#include "UI/Dialogs/TableEditorDialog.h"
#include "Helpers/ResourcesManageHelper.h"
#include "FileSystem/LocalizationSystem.h"
#include "EditorCore.h"

#include <QStandardItemModel>

using namespace DAVA;

TableEditorDialog::TableEditorDialog(const QString& values_, const QList<QString>& header_, QWidget* parent)
    : QDialog(parent)
    , header(header_)
    , values(values_)
{
    ui.setupUi(this);

    ui.addButton->setIcon(QIcon(":/Icons/add.png"));
    ui.removeButton->setIcon(QIcon(":/Icons/editclear.png"));

    connect(ui.addButton, &QPushButton::clicked, this, &TableEditorDialog::OnAddRow);
    connect(ui.removeButton, &QPushButton::clicked, this, &TableEditorDialog::OnRemoveRow);
    connect(ui.pushButton_ok, &QPushButton::clicked, this, &TableEditorDialog::OnOk);
    connect(ui.pushButton_cancel, &QPushButton::clicked, this, &TableEditorDialog::OnCancel);

    model = new QStandardItemModel(0, header.size(), this);
    model->setHorizontalHeaderLabels(header);

    QStringList rows = values.split(";");
    for (QString& rowStr : rows)
    {
        QStringList row = rowStr.split(",");
        QList<QStandardItem*> items;

        for (int i = 0; i < header.size(); i++)
        {
            QString str = "";
            if (row.size() > i)
            {
                str = row[i].trimmed();
            }

            QStandardItem* item = new QStandardItem();
            item->setData(str, Qt::DisplayRole);
            items.push_back(item);
        }
        model->appendRow(items);
    }

    ui.tableView->setModel(model);
    ui.tableView->resizeColumnsToContents();
}

const QString& TableEditorDialog::GetValues() const
{
    return values;
}

void TableEditorDialog::OnOk()
{
    values.clear();
    bool firstRow = true;
    for (int row = 0; row < model->rowCount(); row++)
    {
        bool empty = true;
        for (int col = 0; col < header.size(); col++)
        {
            QString str = model->data(model->index(row, col), Qt::DisplayRole).toString();
            if (!str.trimmed().isEmpty())
            {
                empty = false;
                break;
            }
        }

        if (!empty)
        {
            if (firstRow)
            {
                firstRow = false;
            }
            else
            {
                values.append(";");
            }

            for (int col = 0; col < header.size(); col++)
            {
                if (col > 0)
                {
                    values.append(",");
                }
                values.append(model->data(model->index(row, col), Qt::DisplayRole).toString());
            }
        }
    }
    accept();
}

void TableEditorDialog::OnCancel()
{
    reject();
}

void TableEditorDialog::OnAddRow()
{
    QList<QStandardItem*> items;
    for (int i = 0; i < header.size(); i++)
    {
        QStandardItem* item = new QStandardItem();
        item->setData("");
        items.push_back(item);
    }
    model->appendRow(items);
}

void TableEditorDialog::OnRemoveRow()
{
    QModelIndex current = ui.tableView->selectionModel()->currentIndex();
    if (current.isValid())
    {
        model->removeRow(current.row());
    }
}
