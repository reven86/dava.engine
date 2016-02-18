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

QualitySwitcher* QualitySwitcher::switcherDialog = nullptr;

QualitySwitcher::QualitySwitcher(QWidget* parent /* = nullptr */)
    : QDialog(parent, Qt::Dialog | Qt::WindowStaysOnTopHint) //https://bugreports.qt.io/browse/QTBUG-34767
{
    int mainRow = 0;
    int height = 10;
    const int spacing = 5;
    const int minColumnW = 150;

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QGroupBox* texturesGroup = new QGroupBox(this);
    QGroupBox* materialsGroup = new QGroupBox(this);
    QGroupBox* optionsGroup = new QGroupBox(this);
    QGroupBox* particlesGroup = new QGroupBox(this);
    QWidget* buttonsGroup = new QWidget(this);

    // textures quality
    {
        QGridLayout* texturesLayout = new QGridLayout(texturesGroup);
        texturesLayout->setColumnMinimumWidth(0, minColumnW);
        texturesLayout->setColumnMinimumWidth(1, minColumnW);

        texturesGroup->setTitle("Textures");
        texturesGroup->setLayout(texturesLayout);

        QLabel* labTx = new QLabel("Textures:", texturesGroup);
        QComboBox* comboTx = new QComboBox(texturesGroup);
        comboTx->setObjectName("TexturesCombo");

        QObject::connect(comboTx, SIGNAL(activated(int)), this, SLOT(OnTxQualitySelect(int)));

        texturesLayout->addWidget(labTx, 0, 0);
        texturesLayout->addWidget(comboTx, 0, 1);

        DAVA::FastName curTxQuality = DAVA::QualitySettingsSystem::Instance()->GetCurTextureQuality();

        for (size_t i = 0; i < DAVA::QualitySettingsSystem::Instance()->GetTextureQualityCount(); ++i)
        {
            DAVA::FastName txQualityName = DAVA::QualitySettingsSystem::Instance()->GetTextureQualityName(i);
            comboTx->addItem(txQualityName.c_str());

            if (txQualityName == curTxQuality)
            {
                comboTx->setCurrentIndex(comboTx->count() - 1);
            }
        }
    }

    // materials quality
    {
        QGridLayout* materialsLayout = new QGridLayout(materialsGroup);
        materialsLayout->setColumnMinimumWidth(0, minColumnW);
        materialsLayout->setColumnMinimumWidth(1, minColumnW);

        materialsGroup->setTitle("Materials");
        materialsGroup->setLayout(materialsLayout);

        for (size_t i = 0; i < DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityGroupCount(); ++i)
        {
            DAVA::FastName groupName = DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityGroupName(i);
            DAVA::FastName curGroupQuality = DAVA::QualitySettingsSystem::Instance()->GetCurMaterialQuality(groupName);

            QLabel* labMa = new QLabel(QString(groupName.c_str()) + ":", materialsGroup);
            QComboBox* comboMa = new QComboBox(materialsGroup);
            comboMa->setObjectName(QString(groupName.c_str()) + "Combo");

            QObject::connect(comboMa, SIGNAL(activated(int)), this, SLOT(OnMaQualitySelect(int)));

            materialsLayout->addWidget(labMa, i, 0);
            materialsLayout->addWidget(comboMa, i, 1);

            for (size_t j = 0; j < DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityCount(groupName); ++j)
            {
                DAVA::FastName maQualityName = DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityName(groupName, j);
                comboMa->addItem(maQualityName.c_str(), QString(groupName.c_str()));

                if (curGroupQuality == maQualityName)
                {
                    comboMa->setCurrentIndex(comboMa->count() - 1);
                }
            }
        }
    }

    // particles quality
    {
        QGridLayout* particlesLayout = new QGridLayout(particlesGroup);
        particlesLayout->setColumnMinimumWidth(0, minColumnW);
        particlesLayout->setColumnMinimumWidth(1, minColumnW);

        particlesGroup->setTitle("Particles");
        particlesGroup->setLayout(particlesLayout);

        QLabel* labQuality = new QLabel("Quality:", particlesGroup);
        QComboBox* comboQuality = new QComboBox(particlesGroup);
        comboQuality->setObjectName("ParticlesQualityCombo");

        const ParticlesQualitySettings& particlesSettings = DAVA::QualitySettingsSystem::Instance()->GetParticlesQualitySettings();
        bool qualityAvailable = particlesSettings.GetQualitiesCount() > 0;

        FastName currentQualityName = particlesSettings.GetCurrentQuality();
        for (size_t i = 0, size = particlesSettings.GetQualitiesCount(); i < size; ++i)
        {
            FastName qualityName = particlesSettings.GetQualityName(i);
            comboQuality->addItem(qualityName.c_str());

            if (qualityName == currentQualityName)
            {
                comboQuality->setCurrentIndex(comboQuality->count() - 1);
            }
        }
        comboQuality->setEnabled(qualityAvailable);

        particlesLayout->addWidget(labQuality, 0, 0);
        particlesLayout->addWidget(comboQuality, 0, 1);
        connect(comboQuality, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &QualitySwitcher::OnParticlesQualityChanged);

        QLabel* labTagsCloud = new QLabel("Tags cloud:", particlesGroup);
        QLineEdit* editTagsCloud = new QLineEdit(particlesGroup);
        editTagsCloud->setObjectName("ParticlesTagsCloudEdit");

        QString tagsCloudText;
        for (const FastName& tag : particlesSettings.GetTagsCloud())
        {
            if (!tagsCloudText.isEmpty())
            {
                tagsCloudText += QString(" ");
            }
            tagsCloudText += QString(tag.c_str());
        }

        editTagsCloud->setText(tagsCloudText);
        editTagsCloud->setEnabled(qualityAvailable);

        particlesLayout->addWidget(labTagsCloud, 1, 0);
        particlesLayout->addWidget(editTagsCloud, 1, 1);
        connect(editTagsCloud, &QLineEdit::textChanged, this, &QualitySwitcher::OnParticlesTagsCloudChanged);
    }

    // quality options
    {
        QGridLayout* optionsLayout = new QGridLayout(optionsGroup);
        optionsLayout->setColumnMinimumWidth(0, minColumnW);
        optionsLayout->setColumnMinimumWidth(1, minColumnW);

        optionsGroup->setTitle("Options");
        optionsGroup->setLayout(optionsLayout);

        int32 optionsCount = QualitySettingsSystem::Instance()->GetOptionsCount();
        for (int32 i = 0; i < optionsCount; ++i)
        {
            DAVA::FastName optionName = QualitySettingsSystem::Instance()->GetOptionName(i);

            QLabel* labOp = new QLabel(QString(optionName.c_str()) + ":", materialsGroup);
            QCheckBox* checkOp = new QCheckBox(materialsGroup);
            checkOp->setObjectName(QString(optionName.c_str()) + "CheckBox");
            checkOp->setChecked(QualitySettingsSystem::Instance()->IsOptionEnabled(optionName));
            checkOp->setProperty("qualityOptionName", QVariant(optionName.c_str()));

            QObject::connect(checkOp, SIGNAL(clicked(bool)), this, SLOT(OnOptionClick(bool)));

            optionsLayout->addWidget(labOp, i, 0);
            optionsLayout->addWidget(checkOp, i, 1);
        }
    }

    // buttons
    {
        QHBoxLayout* buttonsLayout = new QHBoxLayout(buttonsGroup);
        buttonsGroup->setLayout(buttonsLayout);

        QPushButton* buttonOk = new QPushButton("OK", particlesGroup);

        QPushButton* buttonCancel = new QPushButton("Cancel", particlesGroup);

        QPushButton* buttonApply = new QPushButton("Apply", particlesGroup);
        buttonApply->setObjectName("ApplyButton");

        connect(buttonOk, &QPushButton::released, this, &QualitySwitcher::OnOkPressed);
        connect(buttonCancel, &QPushButton::released, this, &QualitySwitcher::OnCancelPressed);
        connect(buttonApply, &QPushButton::released, this, &QualitySwitcher::OnApplyPressed);

        buttonsLayout->addStretch();
        buttonsLayout->addWidget(buttonOk);
        buttonsLayout->addWidget(buttonCancel);
        buttonsLayout->addWidget(buttonApply);
        buttonsLayout->setMargin(5);
        buttonsLayout->setSpacing(spacing);
    }

    mainLayout->addWidget(texturesGroup);
    mainLayout->addWidget(materialsGroup);
    mainLayout->addWidget(particlesGroup);
    mainLayout->addWidget(optionsGroup);
    mainLayout->addStretch();
    mainLayout->addWidget(buttonsGroup);
    mainLayout->setMargin(5);
    mainLayout->setSpacing(spacing);

    setLayout(mainLayout);

    SetSettingsDirty(false);

    adjustSize();
}

QualitySwitcher::~QualitySwitcher()
{
    switcherDialog = nullptr;
}

void QualitySwitcher::ApplyTx()
{
    QtMainWindow::Instance()->OnReloadTextures();
}

void QualitySwitcher::ApplyMa()
{
    SceneTabWidget* tabWidget = QtMainWindow::Instance()->GetSceneWidget();
    for (int tab = 0; tab < tabWidget->GetTabCount(); ++tab)
    {
        SceneEditor2* sceneEditor = tabWidget->GetTabScene(tab);

        const DAVA::Set<DAVA::NMaterial*>& topParents = sceneEditor->materialSystem->GetTopParents();

        for (auto material : topParents)
        {
            material->InvalidateRenderVariants();
        }

        sceneEditor->renderSystem->SetForceUpdateLights();
    }
}

void QualitySwitcher::UpdateEntitiesToQuality(DAVA::Entity* e)
{
    DAVA::QualitySettingsSystem::Instance()->UpdateEntityVisibility(e);
    for (int32 i = 0, sz = e->GetChildrenCount(); i < sz; ++i)
    {
        UpdateEntitiesToQuality(e->GetChild(i));
    }
}

void QualitySwitcher::UpdateParticlesToQuality()
{
    SceneTabWidget* tabWidget = QtMainWindow::Instance()->GetSceneWidget();
    SceneSignals* sceneSignals = SceneSignals::Instance();
    for (int32 tab = 0, sz = tabWidget->GetTabCount(); tab < sz; ++tab)
    {
        SceneEditor2* scene = tabWidget->GetTabScene(tab);
        ReloadEntityEmitters(scene);
        sceneSignals->EmitStructureChanged(scene, nullptr);
    }
}

void QualitySwitcher::ReloadEntityEmitters(DAVA::Entity* e)
{
    ParticleEffectComponent* comp = GetEffectComponent(e);
    if (comp)
    {
        comp->ReloadEmitters();
    }

    for (int32 i = 0, sz = e->GetChildrenCount(); i < sz; ++i)
    {
        ReloadEntityEmitters(e->GetChild(i));
    }
}

void QualitySwitcher::SetSettingsDirty(bool dirty)
{
    settingsDirty = dirty;
    QPushButton* applyButton = findChild<QPushButton*>("ApplyButton");
    applyButton->setEnabled(settingsDirty);
}

void QualitySwitcher::ApplySettings()
{
    // textures
    {
        QComboBox* combo = findChild<QComboBox*>("TexturesCombo");
        if (nullptr != combo)
        {
            DAVA::FastName newTxQuality(combo->currentText().toLatin1());
            if (newTxQuality != DAVA::QualitySettingsSystem::Instance()->GetCurTextureQuality())
            {
                DAVA::QualitySettingsSystem::Instance()->SetCurTextureQuality(newTxQuality);
                ApplyTx();
            }
        }
    }

    // materials
    {
        for (size_t i = 0; i < DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityGroupCount(); ++i)
        {
            DAVA::FastName groupName = DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityGroupName(i);
            QComboBox* combo = findChild<QComboBox*>(QString(groupName.c_str()) + "Combo");
            if (nullptr != combo)
            {
                DAVA::FastName newMaQuality(combo->currentText().toLatin1());
                DAVA::FastName group(combo->currentData().toString().toLatin1());

                if (newMaQuality != DAVA::QualitySettingsSystem::Instance()->GetCurMaterialQuality(group))
                {
                    DAVA::QualitySettingsSystem::Instance()->SetCurMaterialQuality(group, newMaQuality);
                    ApplyMa();

                    SceneTabWidget* tabWidget = QtMainWindow::Instance()->GetSceneWidget();
                    for (int tab = 0, sz = tabWidget->GetTabCount(); tab < sz; ++tab)
                    {
                        Scene* scene = tabWidget->GetTabScene(tab);
                        UpdateEntitiesToQuality(scene);
                    }

                    emit QualityChanged();
                }
            }
        }
    }

    //particles
    {
        ParticlesQualitySettings& particlesSettings = QualitySettingsSystem::Instance()->GetParticlesQualitySettings();
        if (particlesSettings.GetQualitiesCount() > 0)
        {
            bool settingsChanged = false;
            QComboBox* combo = findChild<QComboBox*>("ParticlesQualityCombo");
            if (nullptr != combo)
            {
                DAVA::FastName newParticlesQuality(combo->currentText().toLatin1());
                if (particlesSettings.GetCurrentQuality() != newParticlesQuality)
                {
                    particlesSettings.SetCurrentQuality(newParticlesQuality);
                    settingsChanged = true;
                }
            }

            QLineEdit* edit = findChild<QLineEdit*>("ParticlesTagsCloudEdit");
            if (nullptr != edit)
            {
                Vector<String> tags;
                Split(edit->text().toStdString(), " ", tags);

                Set<FastName> newTagsCloud;
                for (const String& tag : tags)
                {
                    newTagsCloud.insert(FastName(tag));
                }

                if (particlesSettings.GetTagsCloud() != newTagsCloud)
                {
                    particlesSettings.SetTagsCloud(newTagsCloud);
                    settingsChanged = true;
                }
            }

            if (settingsChanged)
            {
                UpdateParticlesToQuality();
                emit ParticlesQualityChanged();
            }
        }
    }

    // options
    {
        int32 optionsCount = QualitySettingsSystem::Instance()->GetOptionsCount();
        for (int32 i = 0; i < optionsCount; ++i)
        {
            DAVA::FastName optionName = QualitySettingsSystem::Instance()->GetOptionName(i);
            QCheckBox* checkBox = findChild<QCheckBox*>(QString(optionName.c_str()) + "CheckBox");
            if (nullptr != checkBox)
            {
                FastName optionName(checkBox->property("qualityOptionName").toString().toStdString().c_str());
                QualitySettingsSystem::Instance()->EnableOption(optionName, checkBox->isChecked());

                SceneTabWidget* tabWidget = QtMainWindow::Instance()->GetSceneWidget();
                for (int tab = 0, sz = tabWidget->GetTabCount(); tab < sz; ++tab)
                {
                    Scene* scene = tabWidget->GetTabScene(tab);
                    UpdateEntitiesToQuality(scene);
                }
            }
        }
    }
}

void QualitySwitcher::ShowDialog()
{
    if (switcherDialog == nullptr)
    {
        //we don't need synchronization because of working in UI thread
        switcherDialog = new QualitySwitcher(QtMainWindow::Instance());
        switcherDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        connect(switcherDialog, &QualitySwitcher::QualityChanged, MaterialEditor::Instance(), &MaterialEditor::OnQualityChanged);
        connect(switcherDialog, &QualitySwitcher::QualityChanged, QtMainWindow::Instance()->GetUI()->sceneInfo, &SceneInfo::OnQualityChanged);
        connect(switcherDialog, &QualitySwitcher::ParticlesQualityChanged, QtMainWindow::Instance()->GetUI()->sceneInfo, &SceneInfo::OnQualityChanged);

        switcherDialog->show();
    }

    switcherDialog->raise();
    switcherDialog->activateWindow();
}

void QualitySwitcher::OnTxQualitySelect(int index)
{
    SetSettingsDirty(true);
}

void QualitySwitcher::OnMaQualitySelect(int index)
{
    SetSettingsDirty(true);
}

void QualitySwitcher::OnParticlesQualityChanged(int index)
{
    SetSettingsDirty(true);
}

void QualitySwitcher::OnParticlesTagsCloudChanged(const QString& text)
{
    SetSettingsDirty(true);
}

void QualitySwitcher::OnOptionClick(bool checked)
{
    SetSettingsDirty(true);
}

void QualitySwitcher::OnOkPressed()
{
    if (settingsDirty)
    {
        ApplySettings();
    }

    accept();
}

void QualitySwitcher::OnCancelPressed()
{
    reject();
}

void QualitySwitcher::OnApplyPressed()
{
    SetSettingsDirty(false);

    ApplySettings();
}
