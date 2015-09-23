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


#include "Main/mainwindow.h"
#include "QualitySwitcher.h"
#include "Project/ProjectManager.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "MaterialEditor/MaterialEditor.h"

#include <QComboBox>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>

QualitySwitcher::QualitySwitcher(QWidget *parent /* = nullptr */)
    : QDialog(parent, Qt::Tool)
{
    int mainRow = 0;
    int height = 10;
    const int spacing = 5;
    const int minColumnW = 150;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QGroupBox *texturesGroup = new QGroupBox(this);
    QGroupBox *materialsGroup = new QGroupBox(this);
    QGroupBox *optionsGroup = new QGroupBox(this);

    // textures quality
    {
        QGridLayout *texturesLayout = new QGridLayout(texturesGroup);
        texturesLayout->setColumnMinimumWidth(0, minColumnW);
        texturesLayout->setColumnMinimumWidth(1, minColumnW);

        texturesGroup->setTitle("Textures");
        texturesGroup->setLayout(texturesLayout);

        QLabel *labTx = new QLabel("Textures:", texturesGroup);
        QComboBox *comboTx = new QComboBox(texturesGroup);

        QObject::connect(comboTx, SIGNAL(activated(int)), this, SLOT(OnTxQualitySelect(int)));

        texturesLayout->addWidget(labTx, 0, 0);
        texturesLayout->addWidget(comboTx, 0, 1);

        DAVA::FastName curTxQuality = DAVA::QualitySettingsSystem::Instance()->GetCurTextureQuality();

        for(size_t i = 0; i < DAVA::QualitySettingsSystem::Instance()->GetTextureQualityCount(); ++i)
        {
            DAVA::FastName txQualityName = DAVA::QualitySettingsSystem::Instance()->GetTextureQualityName(i);
            comboTx->addItem(txQualityName.c_str());

            if(txQualityName == curTxQuality)
            {
                comboTx->setCurrentIndex(comboTx->count() - 1);
            }
        }
    }


    // materials quality
    {
        QGridLayout *materialsLayout = new QGridLayout(materialsGroup);
        materialsLayout->setColumnMinimumWidth(0, minColumnW);
        materialsLayout->setColumnMinimumWidth(1, minColumnW);

        materialsGroup->setTitle("Materials");
        materialsGroup->setLayout(materialsLayout);

        for(size_t i = 0; i < DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityGroupCount(); ++i)
        {
            DAVA::FastName groupName = DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityGroupName(i);
            DAVA::FastName curGroupQuality = DAVA::QualitySettingsSystem::Instance()->GetCurMaterialQuality(groupName);
            
            QLabel *labMa = new QLabel(QString(groupName.c_str()) + ":", materialsGroup);
            QComboBox *comboMa = new QComboBox(materialsGroup);

            QObject::connect(comboMa, SIGNAL(activated(int)), this, SLOT(OnMaQualitySelect(int)));

            materialsLayout->addWidget(labMa, i, 0);
            materialsLayout->addWidget(comboMa, i, 1);

            for(size_t j = 0; j < DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityCount(groupName); ++j)
            {
                DAVA::FastName maQualityName = DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityName(groupName, j);
                comboMa->addItem(maQualityName.c_str(), QString(groupName.c_str()));

                if(curGroupQuality == maQualityName)
                {
                    comboMa->setCurrentIndex(comboMa->count() - 1);
                }
            }
        }
    }

    // quality options
    {
        QGridLayout *optionsLayout = new QGridLayout(optionsGroup);
        optionsLayout->setColumnMinimumWidth(0, minColumnW);
        optionsLayout->setColumnMinimumWidth(1, minColumnW);

        optionsGroup->setTitle("Options");
        optionsGroup->setLayout(optionsLayout);

        int32 optionsCount = QualitySettingsSystem::Instance()->GetOptionsCount();
        for(int32 i = 0; i < optionsCount; ++i)
        {
            DAVA::FastName optionName = QualitySettingsSystem::Instance()->GetOptionName(i);

            QLabel *labOp = new QLabel(QString(optionName.c_str()) + ":", materialsGroup);
            QCheckBox *checkOp = new QCheckBox(materialsGroup);
            checkOp->setChecked(QualitySettingsSystem::Instance()->IsOptionEnabled(optionName));
            checkOp->setProperty("qualityOptionName", QVariant(optionName.c_str()));

            QObject::connect(checkOp, SIGNAL(clicked(bool)), this, SLOT(OnOptionClick(bool)));

            optionsLayout->addWidget(labOp, i, 0);
            optionsLayout->addWidget(checkOp, i, 1);
        }
    }

    mainLayout->addWidget(texturesGroup);
    mainLayout->addWidget(materialsGroup);
    mainLayout->addWidget(optionsGroup);
    mainLayout->addStretch();
    mainLayout->setMargin(5);
    mainLayout->setSpacing(spacing);

    setLayout(mainLayout);
    adjustSize();
}

void QualitySwitcher::ApplyTx()
{
    QtMainWindow::Instance()->OnReloadTextures();
}

void QualitySwitcher::ApplyMa()
{
    SceneTabWidget *tabWidget = QtMainWindow::Instance()->GetSceneWidget();
    for(int tab = 0; tab < tabWidget->GetTabCount(); ++tab)
    {
        SceneEditor2 *sceneEditor = tabWidget->GetTabScene(tab);

        const DAVA::Set<DAVA::NMaterial*>& topParents = sceneEditor->materialSystem->GetTopParents();

        for (auto material : topParents)
        {
            material->InvalidateRenderVariants();
        }

        sceneEditor->renderSystem->SetForceUpdateLights();
    }
}

void QualitySwitcher::UpdateEntitiesToQuality(DAVA::Entity *e)
{
    DAVA::QualitySettingsSystem::Instance()->UpdateEntityVisibility(e);
    for (int32 i = 0, sz = e->GetChildrenCount(); i < sz; ++i)
    {
        UpdateEntitiesToQuality(e->GetChild(i));
    }
}

void QualitySwitcher::Show()
{
    QualitySwitcher *sw = new QualitySwitcher(QtMainWindow::Instance());
    sw->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(sw, &QualitySwitcher::QualityChanged, MaterialEditor::Instance(), &MaterialEditor::OnQualityChanged);
    connect(sw, &QualitySwitcher::QualityChanged, QtMainWindow::Instance()->GetUI()->sceneInfo, &SceneInfo::OnQualityChanged);
    sw->show();
}

void QualitySwitcher::ShowModal()
{
    QualitySwitcher sw(QtMainWindow::Instance());
    connect(&sw, &QualitySwitcher::QualityChanged, MaterialEditor::Instance(), &MaterialEditor::OnQualityChanged);
    connect(&sw, &QualitySwitcher::QualityChanged, QtMainWindow::Instance()->GetUI()->sceneInfo, &SceneInfo::OnQualityChanged);
    sw.exec();
}

void QualitySwitcher::OnTxQualitySelect(int index)
{
    QComboBox *combo = dynamic_cast<QComboBox *>(QObject::sender());
    if(nullptr != combo)
    {
        DAVA::FastName newTxQuality(combo->itemText(index).toLatin1());
        if(newTxQuality != DAVA::QualitySettingsSystem::Instance()->GetCurTextureQuality())
        {
            DAVA::QualitySettingsSystem::Instance()->SetCurTextureQuality(newTxQuality);
            ApplyTx();
        }
    }
}

void QualitySwitcher::OnMaQualitySelect(int index)
{
    QComboBox *combo = dynamic_cast<QComboBox *>(QObject::sender());
    if(nullptr != combo)
    {
        DAVA::FastName newMaQuality(combo->itemText(index).toLatin1());
        DAVA::FastName group(combo->itemData(index).toString().toLatin1());

        if(newMaQuality != DAVA::QualitySettingsSystem::Instance()->GetCurMaterialQuality(group))
        {
            DAVA::QualitySettingsSystem::Instance()->SetCurMaterialQuality(group, newMaQuality);
            ApplyMa();

            SceneTabWidget *tabWidget = QtMainWindow::Instance()->GetSceneWidget();
            for (int tab = 0, sz = tabWidget->GetTabCount(); tab < sz; ++tab)
            {
                Scene* scene = tabWidget->GetTabScene(tab);
                UpdateEntitiesToQuality(scene);
            }

            emit QualityChanged();
        }
    }
}

void QualitySwitcher::OnOptionClick(bool checked)
{
    QCheckBox *checkBox = dynamic_cast<QCheckBox *>(QObject::sender());
    if(nullptr != checkBox)
    {
        FastName optionName(checkBox->property("qualityOptionName").toString().toStdString().c_str());
        QualitySettingsSystem::Instance()->EnableOption(optionName, checked);
        
        SceneTabWidget *tabWidget = QtMainWindow::Instance()->GetSceneWidget();
        for (int tab = 0, sz = tabWidget->GetTabCount(); tab < sz; ++tab)
        {
            Scene* scene = tabWidget->GetTabScene(tab);
            UpdateEntitiesToQuality(scene);
        }
    }
}
