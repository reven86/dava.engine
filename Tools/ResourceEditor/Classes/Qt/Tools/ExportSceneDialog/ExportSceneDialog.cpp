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


#include "Tools/ExportSceneDialog/ExportSceneDialog.h"
#include "Tools/Widgets/FilePathBrowser.h"

#include "Base/GlobalEnum.h"
#include "Debug/DVAssert.h"

#include <QCheckBox>
#include <QComboBox>

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

ExportSceneDialog::ExportSceneDialog(QWidget* parent)
    : QDialog(parent)
{
    SetupUI();
    InitializeValues();
}

ExportSceneDialog::~ExportSceneDialog()
{
}

namespace ExportSceneDialogLocal
{
void SetupLayout(QLayout* layout)
{
    layout->setMargin(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
}
}

void ExportSceneDialog::SetupUI()
{
    setMinimumSize(480, 320);

    DVASSERT(projectPathBrowser == nullptr); // to prevent several calls of this functions

    QGridLayout* dialogLayout = new QGridLayout();

    projectPathBrowser = new FilePathBrowser(this);
    dialogLayout->addWidget(projectPathBrowser, 0, 0, 1, 0);

    {
        QVBoxLayout* gpuLayout = new QVBoxLayout();
        ExportSceneDialogLocal::SetupLayout(gpuLayout);

        QLabel* gpuLabel = new QLabel(this);
        gpuLabel->setText("Select GPU:");
        gpuLayout->addWidget(gpuLabel);

        for (DAVA::int32 gpu = DAVA::GPU_POWERVR_IOS; gpu < DAVA::GPU_FAMILY_COUNT; ++gpu) //use int to put it into action::data
        {
            QString gpuText = GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(static_cast<DAVA::eGPUFamily>(gpu));

            gpuSelector[gpu] = new QCheckBox(gpuText, this);
            gpuLayout->addWidget(gpuSelector[gpu]);
        }

        dialogLayout->addLayout(gpuLayout, 1, 0);
    }

    QVBoxLayout* optionsLayout = new QVBoxLayout();
    ExportSceneDialogLocal::SetupLayout(optionsLayout);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    ExportSceneDialogLocal::SetupLayout(buttonsLayout);

    //    QComboBox *qualitySelector = nullptr;
    //
    //    QCheckBox *optimizeOnExport = nullptr;
    //    QCheckBox *useHDtextures = nullptr;
    //

    //    ExportSceneDialogLocal::SetupLayout(dialogLayout);

    dialogLayout->addLayout(optionsLayout, 1, 1);
    dialogLayout->addLayout(buttonsLayout, 0, 2, 1, 2);

    setLayout(dialogLayout);
}

void ExportSceneDialog::InitializeValues()
{
}
