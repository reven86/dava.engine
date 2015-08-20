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

#include "Qt/DeviceInfo/MemoryTool/BacktraceSymbolTable.h"

#include "Qt/DeviceInfo/MemoryTool/Models/SymbolsListModel.h"

#include "Qt/DeviceInfo/MemoryTool/Widgets/SymbolsWidget.h"

#include <QListView>
#include <QLineEdit>
#include <QVBoxLayout>

using namespace DAVA;

SymbolsWidget::SymbolsWidget(const BacktraceSymbolTable& symbolTable_, QWidget* parent)
    : QWidget(parent)
    , symbolTable(symbolTable_)
{
    Init();
}

SymbolsWidget::~SymbolsWidget() = default;

Vector<const String*> SymbolsWidget::GetSelectedSymbols()
{
    Vector<const String*> result;
    QItemSelectionModel* selectionModel = listWidget->selectionModel();
    if (selectionModel->hasSelection())
    {
        QModelIndexList indexList = selectionModel->selectedRows();
        result.reserve(indexList.size());
        for (const QModelIndex& i : indexList)
        {
            QVariant v = symbolFilterModel->data(i, SymbolsListModel::ROLE_SYMBOL_POINTER);
            const String* name = static_cast<const String*>(v.value<void*>());
            Q_ASSERT(name != nullptr);
            result.push_back(name);
        }
    }
    return result;
}

void SymbolsWidget::Init()
{
    symbolListModel.reset(new SymbolsListModel(symbolTable));
    symbolFilterModel.reset(new SymbolsFilterModel(symbolListModel.get()));
    symbolFilterModel->sort(0);

    listWidget = new QListView;
    listWidget->setFont(QFont("Consolas", 10, 500));
    listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    listWidget->setModel(symbolFilterModel.get());

    QLineEdit* filterWidget = new QLineEdit;

    connect(filterWidget, &QLineEdit::textChanged, symbolFilterModel.get(), &SymbolsFilterModel::SetFilterString);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(filterWidget);
    layout->addWidget(listWidget);

    setLayout(layout);
}
