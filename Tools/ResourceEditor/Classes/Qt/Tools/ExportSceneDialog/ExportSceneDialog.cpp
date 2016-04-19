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

#include "Project/ProjectManager.h"
#include "Settings/SettingsManager.h"
#include "TextureCompression/TextureConverter.h"

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

void ExportSceneDialog::SetupUI()
{
    setModal(true);

    static const int UI_WIDTH = 160;
    static const int UI_HEIGHT = 20;

    QGridLayout* dialogLayout = new QGridLayout();
    dialogLayout->setColumnStretch(0, 1);
    dialogLayout->setColumnStretch(1, 1);

    DVASSERT(projectPathBrowser == nullptr); // to prevent several calls of this functions
    projectPathBrowser = new FilePathBrowser(this);
    projectPathBrowser->SetType(FilePathBrowser::Folder);
    projectPathBrowser->setMinimumSize(UI_WIDTH * 3, UI_HEIGHT);
    dialogLayout->addWidget(projectPathBrowser, 0, 0, 1, 0);

    { //GPU
        QVBoxLayout* gpuLayout = new QVBoxLayout();

        QLabel* gpuLabel = new QLabel(this);
        gpuLabel->setText("Select GPU:");
        gpuLabel->setMinimumSize(UI_WIDTH, UI_HEIGHT);
        gpuLayout->addWidget(gpuLabel);

        for (DAVA::int32 gpu = DAVA::GPU_POWERVR_IOS; gpu < DAVA::GPU_FAMILY_COUNT; ++gpu)
        {
            QString gpuText = GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(static_cast<DAVA::eGPUFamily>(gpu));

            gpuSelector[gpu] = new QCheckBox(gpuText, this);
            gpuSelector[gpu]->setMinimumSize(UI_WIDTH, UI_HEIGHT);

            gpuLayout->addWidget(gpuSelector[gpu]);
        }

        gpuLayout->addStretch();

        dialogLayout->addLayout(gpuLayout, 1, 0);
    }

    { // options
        QVBoxLayout* optionsLayout = new QVBoxLayout();

        QLabel* qualityLabel = new QLabel(this);
        qualityLabel->setText("Select Quality:");
        qualityLabel->setMinimumSize(UI_WIDTH, UI_HEIGHT);
        optionsLayout->addWidget(qualityLabel);

        qualitySelector = new QComboBox(this);
        qualitySelector->setMinimumSize(UI_WIDTH, UI_HEIGHT);
        optionsLayout->addWidget(qualitySelector);

        const auto& qualityMap = GlobalEnumMap<DAVA::TextureConverter::eConvertQuality>::Instance();
        for (size_t i = 0; i < qualityMap->GetCount(); ++i)
        {
            int value;
            bool ok = qualityMap->GetValue(i, value);
            if (ok)
            {
                qualitySelector->addItem(qualityMap->ToString(value), value);
            }
        }

        optimizeOnExport = new QCheckBox("Optimize on export", this);
        optimizeOnExport->setMinimumSize(UI_WIDTH, UI_HEIGHT);
        optionsLayout->addWidget(optimizeOnExport);

        useHDtextures = new QCheckBox("Use HD Textures", this);
        useHDtextures->setMinimumSize(UI_WIDTH, UI_HEIGHT);
        optionsLayout->addWidget(useHDtextures);

        optionsLayout->addStretch();
        dialogLayout->addLayout(optionsLayout, 1, 1);
    }

    { //buttons
        QPushButton* cancelButton = new QPushButton("Cancel", this);
        cancelButton->setMinimumSize(UI_WIDTH, UI_HEIGHT);
        cancelButton->setFixedHeight(UI_HEIGHT);

        connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

        QPushButton* exportButton = new QPushButton("Export", this);
        exportButton->setMinimumSize(UI_WIDTH, UI_HEIGHT);
        exportButton->setFixedHeight(UI_HEIGHT);
        connect(exportButton, &QPushButton::clicked, this, &ExportSceneDialog::accept);

        dialogLayout->addWidget(cancelButton, 2, 0);
        dialogLayout->addWidget(exportButton, 2, 1);
    }

    setLayout(dialogLayout);
    setFixedHeight(220);
}

void ExportSceneDialog::InitializeValues()
{
    DAVA::String path = ProjectManager::Instance()->GetProjectPath().GetAbsolutePathname();
    projectPathBrowser->SetPath(QString::fromStdString(path + "Data/3d/"));

    for (DAVA::int32 gpu = DAVA::GPU_POWERVR_IOS; gpu < DAVA::GPU_FAMILY_COUNT; ++gpu)
    {
        gpuSelector[gpu]->setCheckState(Qt::Unchecked);
    }

    DAVA::VariantType quality = SettingsManager::Instance()->GetValue(Settings::General_CompressionQuality);
    qualitySelector->setCurrentIndex(quality.AsInt32());
    optimizeOnExport->setCheckState(Qt::Checked);
    useHDtextures->setCheckState(Qt::Unchecked);
}

void ExportSceneDialog::accept()
{
    QDialog::accept();
}

DAVA::FilePath ExportSceneDialog::GetDataFolder() const
{
    DAVA::FilePath path = projectPathBrowser->GetPath().toStdString();
    path.MakeDirectoryPathname();
    return path;
}

DAVA::Vector<DAVA::eGPUFamily> ExportSceneDialog::GetGPUs() const
{
    DAVA::Vector<DAVA::eGPUFamily> gpus;

    for (DAVA::int32 gpu = DAVA::GPU_POWERVR_IOS; gpu < DAVA::GPU_FAMILY_COUNT; ++gpu)
    {
        if (gpuSelector[gpu]->checkState() == Qt::Checked)
        {
            gpus.push_back(static_cast<DAVA::eGPUFamily>(gpu));
        }
    }

    return gpus;
}

DAVA::TextureConverter::eConvertQuality ExportSceneDialog::GetQuality() const
{
    return static_cast<DAVA::TextureConverter::eConvertQuality>(qualitySelector->currentIndex());
}

bool ExportSceneDialog::GetOptimizeOnExport() const
{
    return optimizeOnExport->checkState() == Qt::Checked;
}

bool ExportSceneDialog::GetUseHDTextures() const
{
    return useHDtextures->checkState() == Qt::Checked;
}
