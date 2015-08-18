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

#include "Debug/DVAssert.h"

#include "Qt/DeviceInfo/MemoryTool/BlockLink.h"
#include "Qt/DeviceInfo/MemoryTool/Models/MemoryBlocksModel.h"

#include "Qt/DeviceInfo/MemoryTool/MemoryBlocksWidget.h"

#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>

using namespace DAVA;

MemoryBlocksWidget::MemoryBlocksWidget(const ProfilingSession* session_, const BlockLink* blockLink_, QWidget* parent)
    : QWidget(parent)
    , session(session_)
    , blockLink(blockLink_)
{
    DVASSERT(session != nullptr && blockLink != nullptr);
    Init();
}

MemoryBlocksWidget::~MemoryBlocksWidget() = default;

void MemoryBlocksWidget::Init()
{
    memoryBlocksModel.reset(new MemoryBlocksModel(session));
    memoryBlocksModel->SetBlockLink(blockLink);
    memoryBlocksFilterModel.reset(new MemoryBlocksFilterModel(memoryBlocksModel.get()));

    tableWidget = new QTableView;
    tableWidget->setFont(QFont("Consolas", 10, 500));
    tableWidget->setModel(memoryBlocksFilterModel.get());
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(tableWidget);

    setLayout(layout);
}
