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


#include "SoundComponentEditor.h"
#include "FMODSoundBrowser.h"
#include "ui_soundcomponenteditor.h"
#include "Commands2/SoundComponentEditCommands.h"

#include <QTreeWidget>
#include <QMessageBox>
#include <QLabel>
#include <QSlider>
#include <QToolTip>

SoundComponentEditor::SoundComponentEditor(SceneEditor2* _scene, QWidget *parent) :
    QDialog(parent),
    component(0),
    scene(_scene),
    ui(new Ui::SoundComponentEditor)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);

    ui->playButton->setIcon(QIcon(":/QtIcons/play.png"));
    ui->stopButton->setIcon(QIcon(":/QtIcons/stop.png"));

    ui->playButton->setDisabled(true);
    ui->stopButton->setDisabled(true);

    QObject::connect(ui->playButton, SIGNAL(clicked()), this, SLOT(OnPlay()));
    QObject::connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(OnStop()));
    QObject::connect(ui->addEventBut, SIGNAL(clicked()), this, SLOT(OnAddEvent()));
    QObject::connect(ui->removeEventBut, SIGNAL(clicked()), this, SLOT(OnRemoveEvent()));
    QObject::connect(ui->autoTriggerCheckBox, SIGNAL(clicked(bool)), this, SLOT(OnAutoTrigger(bool)));

    QObject::connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(OnEventSelected(QListWidgetItem *)));

    OnEventSelected(0);

    setModal(true);
}

SoundComponentEditor::~SoundComponentEditor()
{
    delete ui;
}

void SoundComponentEditor::SetEditableEntity(DAVA::Entity * _entity)
{
    DVASSERT(_entity);
    entity = _entity;

    component = GetSoundComponent(entity);
    DVASSERT(component);

    FillEventsList();
}

void SoundComponentEditor::OnEventSelected(QListWidgetItem * item)
{
    if(item)
    {
        QVariant data = item->data(Qt::UserRole);
        if(!data.isNull())
        {
            DAVA::int32 index = data.toInt();
            selectedEventIndex = index;
        }
        ui->removeEventBut->setDisabled(false);
    }
    else
    {
        selectedEventIndex = -1;
        ui->removeEventBut->setDisabled(true);
    }

    FillEventParamsFrame();
}

void SoundComponentEditor::OnAutoTrigger(bool checked)
{
    if(selectedEventIndex != -1)
    {
        uint32 flags = component->GetSoundEventFlags(selectedEventIndex);
        if(checked)
        {
            scene->Exec(new SetSoundEventFlagsCommand(entity, selectedEventIndex, flags | SoundComponent::FLAG_AUTO_DISTANCE_TRIGGER));
        }
        else
        {
            scene->Exec(new SetSoundEventFlagsCommand(entity, selectedEventIndex, flags & ~SoundComponent::FLAG_AUTO_DISTANCE_TRIGGER));
        }
    }
}

void SoundComponentEditor::FillEventsList()
{
    ui->listWidget->clear();

    if(!component)
        return;

    DAVA::int32 eventsCount = component->GetEventsCount();
    for(DAVA::int32 i = 0; i < eventsCount; ++i)
    {
        QListWidgetItem * item = new QListWidgetItem(QString(component->GetSoundEvent(i)->GetEventName().c_str()));
        item->setData(Qt::UserRole, i);
        ui->listWidget->addItem(item);

        if(i == selectedEventIndex)
        {
            item->setSelected(true);
            OnEventSelected(item);
        }
    }
}

void SoundComponentEditor::FillEventParamsFrame()
{
    ClearParamsFrame();

    if(selectedEventIndex == -1)
        return;

    SoundEvent * soundEvent = component->GetSoundEvent(selectedEventIndex);

    ui->eventNameLabel->setText(QString(soundEvent->GetEventName().c_str()));

    ui->playButton->setDisabled(false);
    ui->stopButton->setDisabled(false);
    ui->autoTriggerCheckBox->setDisabled(false);

    ui->autoTriggerCheckBox->setChecked((component->GetSoundEventFlags(selectedEventIndex) & SoundComponent::FLAG_AUTO_DISTANCE_TRIGGER) > 0);

    DAVA::Vector<DAVA::SoundEvent::SoundEventParameterInfo> params;
    soundEvent->GetEventParametersInfo(params);
    DAVA::int32 paramsCount = params.size();
    for(DAVA::int32 i = 0; i < paramsCount; i++)
    {
        DAVA::SoundEvent::SoundEventParameterInfo & param = params[i];
        if(param.name != "(distance)" && param.name != "(event angle)" && param.name != "(listener angle)")
        {
            float32 currentParamValue = soundEvent->GetParameterValue(FastName(param.name));
            AddSliderWidget(param, currentParamValue);
        }
    }
}

void SoundComponentEditor::OnPlay()
{
    if(selectedEventIndex != -1 && !component->GetSoundEvent(selectedEventIndex)->IsActive())
        component->Trigger(selectedEventIndex);
}

void SoundComponentEditor::OnStop()
{
    if(selectedEventIndex != -1)
        component->Stop(selectedEventIndex);
}

void SoundComponentEditor::OnAddEvent()
{
    if(component)
    {
        FMODSoundBrowser * browser = FMODSoundBrowser::Instance();
        if(browser->exec() == QDialog::Accepted)
        {
            DAVA::String selectedEventName = browser->GetSelectSoundEvent();
            DAVA::SoundEvent * sEvent = DAVA::SoundSystem::Instance()->CreateSoundEventByID(FastName(selectedEventName), DAVA::FastName("FX"));

            scene->Exec(new AddSoundEventCommand(component->GetEntity(), sEvent));

            selectedEventIndex = component->GetEventsCount() - 1;

            SafeRelease(sEvent);
        }
    }

    FillEventsList();
}

void SoundComponentEditor::OnRemoveEvent()
{
    if(selectedEventIndex != -1 && component)
    {
        scene->Exec(new RemoveSoundEventCommand(component->GetEntity(), component->GetSoundEvent(selectedEventIndex)));
    }

    OnEventSelected(0);

    FillEventsList();
}

void SoundComponentEditor::OnSliderPressed()
{
    QSlider * slider = dynamic_cast<QSlider *>(QObject::sender());

    DAVA::float32 minValue = slider->property("minValue").toFloat();
    DAVA::float32 maxValue = slider->property("maxValue").toFloat();

    DAVA::float32 curParamValue = slider->value() / 1000.f * (maxValue - minValue) + minValue;

    QToolTip::showText(QCursor::pos(), QString("%1").arg(curParamValue), slider);
}

void SoundComponentEditor::OnSliderMoved(int value)
{
    QSlider * slider = dynamic_cast<QSlider *>(QObject::sender());

    DAVA::String paramName = slider->property("paramName").toString().toStdString();
    DAVA::float32 minValue = slider->property("minValue").toFloat();
    DAVA::float32 maxValue = slider->property("maxValue").toFloat();

    DAVA::float32 newParamValue = value / 1000.f * (maxValue - minValue) + minValue;

    QToolTip::showText(QCursor::pos(), QString("%1").arg(newParamValue), slider);

    if(selectedEventIndex != -1)
        component->GetSoundEvent(selectedEventIndex)->SetParameterValue(DAVA::FastName(paramName), newParamValue);
}

void SoundComponentEditor::AddSliderWidget(const DAVA::SoundEvent::SoundEventParameterInfo & param, float32 currentParamValue)
{
    QGridLayout * layout = dynamic_cast<QGridLayout *>(ui->paramsFrame->layout());
    
    QLabel * nameLabel = new QLabel(QString(param.name.c_str()));
    QLabel * minLabel = new QLabel(QString(DAVA::Format("%.1f", param.minValue).c_str()));
    minLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QLabel * maxLabel = new QLabel(QString(DAVA::Format("%.1f", param.maxValue).c_str()));
    QSlider * slider = new QSlider(Qt::Horizontal);
    slider->setMinimum(0);
    slider->setMaximum(1000);
    slider->setValue(0);
    slider->setProperty("paramName", QString(param.name.c_str()));
    slider->setProperty("minValue", param.minValue);
    slider->setProperty("maxValue", param.maxValue);

    if(selectedEventIndex != -1)
    {
        int currentValue = (currentParamValue - param.minValue) / (param.maxValue - param.minValue) * 1000;
        slider->setValue(currentValue);
    }

    QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderMoved(int)));
    QObject::connect(slider, SIGNAL(sliderPressed()), this, SLOT(OnSliderPressed()));

    layout->addWidget(nameLabel);
    layout->addWidget(minLabel);
    layout->addWidget(slider);
    layout->addWidget(maxLabel);
}

void SoundComponentEditor::ClearParamsFrame()
{
    QGridLayout * layout = dynamic_cast<QGridLayout *>(ui->paramsFrame->layout());

    QLayoutItem* item = 0;
    while((item = layout->takeAt(0)))
    {
        delete item->widget();
        delete item;
    }

    layout->setColumnStretch(0, 3);
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(2, 8);
    layout->setColumnStretch(3, 1);

    ui->playButton->setDisabled(true);
    ui->stopButton->setDisabled(true);
    ui->autoTriggerCheckBox->setDisabled(true);

    ui->eventNameLabel->clear();
}
