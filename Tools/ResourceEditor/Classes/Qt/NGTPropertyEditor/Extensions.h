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

#pragma once

#include "Commands2/Base/Command2.h"
#include "Functional/Signal.h"
#include "QtTools/WarningGuard/QtWarningsHandler.h"

#include <core_data_model/reflection/property_model_extensions.hpp>

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

struct DAVAProperiesEnum
{
    enum Type
    {
        EntityRoot = PropertyNode::DomainSpecificProperty,
    };
};

class PropertyPanelGetExtension : public GetterExtension
{
public:
    Variant getValue(const RefPropertyItem* item, int column, size_t roleId, IDefinitionManager& definitionManager) const override;
};

class EntityChildCreatorExtension : public ChildCreatorExtension
{
public:
    void exposeChildren(const PropertyNode& node, std::vector<const PropertyNode*>& children, IDefinitionManager& defMng) const override;
};

class EntityMergeValueExtension : public MergeValuesExtension
{
public:
    RefPropertyItem* lookUpItem(const PropertyNode* node, const std::vector<std::unique_ptr<RefPropertyItem>>& items,
                                IDefinitionManager& definitionManager) const override;
};

class AddCustomPropertyWidget : public QWidget
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT;
    POP_QT_WARNING_SUPRESSOR

public:
    AddCustomPropertyWidget(int defaultType = DAVA::VariantType::TYPE_STRING, QWidget* parent = NULL);

    DAVA::Signal<const DAVA::String&, const DAVA::VariantType&> ValueReady;

    void showEvent(QShowEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

protected slots:
    void OkKeyPressed();
    void PreSetSelected(int index);

private:
    QLineEdit* keyWidget;
    QComboBox* valueWidget;
    QComboBox* presetWidget;
    QPushButton* defaultBtn;
};

class EntityInjectDataExtension : public InjectDataExtension
{
public:
    class Delegate
    {
    public:
        virtual void StartBatch(const DAVA::String& name, DAVA::uint32 commandCount) = 0;
        virtual void Exec(Command2::Pointer&& command) = 0;
        virtual void EndBatch() = 0;
    };

    EntityInjectDataExtension(Delegate& delegateObj, IDefinitionManager& defManager);

    void inject(RefPropertyItem* item) override;
    void updateInjection(RefPropertyItem* item) override;

private:
    void RemoveComponent(const RefPropertyItem* item);

    void RemoveRenderBatch(const RefPropertyItem* item);
    void ConvertBatchToShadow(const RefPropertyItem* item);
    void RebuildTangentSpace(const RefPropertyItem* item);

    void AddCustomProperty(const RefPropertyItem* item);
    void OpenMaterials(const RefPropertyItem* item);

private:
    Delegate& delegateObj;
    IDefinitionManager& defManager;
};
