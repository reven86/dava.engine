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

#include <QComboBox>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>

QualitySwitcher::QualitySwitcher(QWidget *parent /* = NULL */)
: QDialog(parent , Qt::Tool)
{
    int mainRow = 0;
    int height = 10;
    const int spacing = 5;
    const int minColumnW = 150;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QGroupBox *texturesGroup = new QGroupBox(this);
    QGroupBox *materialsGroup = new QGroupBox(this);

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

        DAVA::FastName curTxQuality = DAVA::QualitySettingsSystem::Instance()->GetCurTxQuality();

        for(size_t i = 0; i < DAVA::QualitySettingsSystem::Instance()->GetTxQualityCount(); ++i)
        {
            DAVA::FastName txQualityName = DAVA::QualitySettingsSystem::Instance()->GetTxQualityName(i);
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

        for(size_t i = 0; i < DAVA::QualitySettingsSystem::Instance()->GetMaQualityGroupCount(); ++i)
        {
            DAVA::FastName groupName = DAVA::QualitySettingsSystem::Instance()->GetMaQualityGroupName(i);
            DAVA::FastName curGroupQuality = DAVA::QualitySettingsSystem::Instance()->GetCurMaQuality(groupName);
            
            QLabel *labMa = new QLabel(QString(groupName.c_str()) + ":", materialsGroup);
            QComboBox *comboMa = new QComboBox(materialsGroup);

            QObject::connect(comboMa, SIGNAL(activated(int)), this, SLOT(OnMaQualitySelect(int)));

            materialsLayout->addWidget(labMa, i, 0);
            materialsLayout->addWidget(comboMa, i, 1);

            for(size_t j = 0; j < DAVA::QualitySettingsSystem::Instance()->GetMaQualityCount(groupName); ++j)
            {
                DAVA::FastName maQualityName = DAVA::QualitySettingsSystem::Instance()->GetMaQualityName(groupName, j);
                comboMa->addItem(maQualityName.c_str(), QString(groupName.c_str()));

                if(curGroupQuality == maQualityName)
                {
                    comboMa->setCurrentIndex(comboMa->count() - 1);
                }
            }
        }
    }

    mainLayout->addWidget(texturesGroup);
    mainLayout->addWidget(materialsGroup);
    mainLayout->addStretch();
    mainLayout->setMargin(5);
    mainLayout->setSpacing(spacing);

    setLayout(mainLayout);
    adjustSize();
}

QualitySwitcher::~QualitySwitcher()
{ }

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

        DAVA::Map<DAVA::NMaterial*, DAVA::Set<DAVA::NMaterial *> > materialsTree;
        sceneEditor->materialSystem->BuildMaterialsTree(materialsTree);

        DAVA::Map<DAVA::NMaterial*, DAVA::Set<DAVA::NMaterial *> >::iterator begin = materialsTree.begin();
        DAVA::Map<DAVA::NMaterial*, DAVA::Set<DAVA::NMaterial *> >::iterator end = materialsTree.end();

        for(; begin != end; begin++)
        {
            DAVA::NMaterial *material = begin->first;
            if(material->GetMaterialType() == DAVA::NMaterial::MATERIALTYPE_MATERIAL)
            {
                material->ReloadQuality();
            }
        }
    }
}

void QualitySwitcher::Show()
{
    QualitySwitcher sw(QtMainWindow::Instance());
    sw.exec();
}

void QualitySwitcher::OnTxQualitySelect(int index)
{
    QComboBox *combo = dynamic_cast<QComboBox *>(QObject::sender());
    if(NULL != combo)
    {
        DAVA::FastName newTxQuality(combo->itemText(index).toAscii());
        if(newTxQuality != DAVA::QualitySettingsSystem::Instance()->GetCurTxQuality())
        {
            DAVA::QualitySettingsSystem::Instance()->SetCurTxQuality(newTxQuality);
            ApplyTx();
        }
    }
}

void QualitySwitcher::OnMaQualitySelect(int index)
{
    QComboBox *combo = dynamic_cast<QComboBox *>(QObject::sender());
    if(NULL != combo)
    {
        DAVA::FastName newMaQuality(combo->itemText(index).toAscii());
        DAVA::FastName group(combo->itemData(index).toString().toAscii());

        if(newMaQuality != DAVA::QualitySettingsSystem::Instance()->GetCurMaQuality(group))
        {
            DAVA::QualitySettingsSystem::Instance()->SetCurMaQuality(group, newMaQuality);
            ApplyMa();
        }
    }
}
