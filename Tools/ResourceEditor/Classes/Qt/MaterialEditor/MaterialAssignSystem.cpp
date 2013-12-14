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


#include "MaterialAssignSystem.h"
#include "Scene/EntityGroup.h"
#include "Scene/SceneEditor2.h"


#include "Commands2/MaterialSwitchParentCommand.h"
#include "Tools/MimeData/MimeDataHelper2.h"

#include "Base/BaseTypes.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/Components/ComponentHelpers.h"

//Qt
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QCursor>

Q_DECLARE_METATYPE( DAVA::NMaterial * );


MaterialAssignSystem::DropTestResult::DropTestResult()
{
    hasEntitiesAvailableToDrop = hasEntityUnavailableToDrop = false;
    
    countEntitiesAvailableToDrop = countEntityUnavailableToDrop = 0;
    
    firstUnavailableEntity = NULL;
}


void MaterialAssignSystem::AssignMaterial(SceneEditor2 *scene, DAVA::NMaterial *oldMaterial, const DAVA::NMaterial *newMaterial)
{
    scene->Exec(new MaterialSwitchParentCommand(oldMaterial, newMaterial));
}


void MaterialAssignSystem::AssignMaterialToGroup(SceneEditor2 *scene, const EntityGroup *group, const DAVA::NMaterial *material)
{
    MaterialAssignSystem::DropTestResult result = TestEntityGroup(group, true);
    DAVA::Set<DAVA::NMaterial *> oldMaterials = GetAvailableMaterials(result, group, true);
    if(oldMaterials.size())
    {
        scene->BeginBatch("Set material");
        
        auto endIt = oldMaterials.end();
        for(auto it = oldMaterials.begin(); it != endIt; ++it)
        {
            scene->Exec(new MaterialSwitchParentCommand(*it, material));
        }

        scene->EndBatch();
    }
    
    if(result.countEntityUnavailableToDrop > 1)
    {
        DAVA::Vector<const DAVA::Entity *> rejectedEntities = GetDropRejectedEntities(group, true);
        
        DAVA::String names;
        for (DAVA::uint32 ie = 0; ie < (DAVA::uint32)rejectedEntities.size(); ++ie)
        {
            if(ie != 0) names += ",";
            
            names += rejectedEntities[ie]->GetName();
        }
        
        String errorString = Format("Cannot drop material to %s", names.c_str());
        QMessageBox::warning(NULL, QString("Drop error"), QString::fromStdString(errorString), QMessageBox::Ok);
    }
}


void MaterialAssignSystem::AssignMaterialToEntity(SceneEditor2 *scene, const DAVA::Entity *entity, const DAVA::NMaterial *material)
{
    MaterialAssignSystem::DropTestResult result = TestEntity(entity, true);
    DAVA::Set<DAVA::NMaterial *> oldMaterials = GetAvailableMaterials(result, entity, true);
    if(oldMaterials.size())
    {
        scene->BeginBatch("Set material");
        
        auto endIt = oldMaterials.end();
        for(auto it = oldMaterials.begin(); it != endIt; ++it)
        {
            scene->Exec(new MaterialSwitchParentCommand(*it, material));
        }
        
        scene->EndBatch();
    }
    
    if(result.countEntityUnavailableToDrop > 1)
    {
        DAVA::Vector<const DAVA::Entity *> rejectedEntities = GetDropRejectedEntities(entity, true);
        
        DAVA::String names;
        for (DAVA::uint32 ie = 0; ie < (DAVA::uint32)rejectedEntities.size(); ++ie)
        {
            if(ie != 0) names += ",";
            
            names += rejectedEntities[ie]->GetName();
        }
        
        String errorString = Format("Cannot drop material to %s", names.c_str());
        QMessageBox::warning(NULL, QString("Drop error"), QString::fromStdString(errorString), QMessageBox::Ok);
    }
}

MaterialAssignSystem::DropTestResult MaterialAssignSystem::TestEntityGroup(const EntityGroup *group, const bool recursive)
{
    if(!group) return MaterialAssignSystem::DropTestResult();
    
    MaterialAssignSystem::DropTestResult result;
    
    const size_t count = group->Size();
    for(size_t i = 0; i < count; ++i)
    {
        TestEntity(result, group->GetEntity(i), recursive);
    }
    
    return result;
}


MaterialAssignSystem::DropTestResult MaterialAssignSystem::TestEntity(const DAVA::Entity * entity, const bool recursive)
{
    MaterialAssignSystem::DropTestResult result;
    
    TestEntity(result, entity, recursive);
    
    return result;
}

void MaterialAssignSystem::TestEntity(MaterialAssignSystem::DropTestResult & result, const DAVA::Entity * entity, const bool recursive)
{
    DAVA::RenderObject *ro = DAVA::GetRenderObject(entity);
    if(ro)
    {
        DAVA::uint32 rbCount = ro->GetRenderBatchCount();
        if(rbCount == 1)
        {
            result.hasEntitiesAvailableToDrop = true;
            ++result.countEntitiesAvailableToDrop;
        }
        else
        {
            result.hasEntityUnavailableToDrop = true;
            ++result.countEntityUnavailableToDrop;
            
            if(result.firstUnavailableEntity == NULL) result.firstUnavailableEntity = entity;
        }
    }
    
    if(recursive)
    {
        const DAVA::int32 count = entity->GetChildrenCount();
        for(DAVA::int32 i = 0; i < count; ++i)
        {
            TestEntity(result, entity->GetChild(i), recursive);
        }
    }
}


DAVA::Set<DAVA::NMaterial *> MaterialAssignSystem::GetAvailableMaterials(const MaterialAssignSystem::DropTestResult & result, const EntityGroup *group, const bool recursive)
{
    DAVA::Set<DAVA::NMaterial *> materials;
    
    size_t count = group->Size();
    for(size_t i = 0; i < count; ++i)
    {
        GetAvailableMaterials(materials, group->GetEntity(i), recursive);
    }

    AddSelectedMaterial(materials, result);

    return materials;
}

DAVA::Set<DAVA::NMaterial *> MaterialAssignSystem::GetAvailableMaterials(const MaterialAssignSystem::DropTestResult & result, const DAVA::Entity *entity, const bool recursive)
{
    DAVA::Set<DAVA::NMaterial *> materials;
    
    GetAvailableMaterials(materials, entity, recursive);
    AddSelectedMaterial(materials, result);

    return materials;
}

void MaterialAssignSystem::GetAvailableMaterials(DAVA::Set<DAVA::NMaterial *> &materials, const DAVA::Entity *entity, const bool recursive)
{
    DAVA::RenderObject * ro = DAVA::GetRenderObject(entity);
    if(ro)
    {
        DAVA::uint32 count = ro->GetRenderBatchCount();
        if(1 == count)
        {
            materials.insert(ro->GetRenderBatch(0)->GetMaterial());
        }
    }
    
    if(recursive)
    {
        DAVA::int32 count = entity->GetChildrenCount();
        for(DAVA::int32 i = 0; i < count; ++i)
        {
            GetAvailableMaterials(materials, entity->GetChild(i), recursive);
        }
    }
}


DAVA::Vector<const DAVA::Entity *> MaterialAssignSystem::GetDropRejectedEntities(const EntityGroup *group, const bool recursive)
{
    DAVA::Vector<const DAVA::Entity *> rejectedEntities;
    
    size_t count = group->Size();
    for(size_t i = 0; i < count; ++i)
    {
        GetDropRejectedEntities(rejectedEntities, group->GetEntity(i), recursive);
    }
    
    return rejectedEntities;
}


DAVA::Vector<const DAVA::Entity *> MaterialAssignSystem::GetDropRejectedEntities(const DAVA::Entity *entity, const bool recursive)
{
    DAVA::Vector<const DAVA::Entity *> rejectedEntities;
    
    GetDropRejectedEntities(rejectedEntities, entity, recursive);
    
    return rejectedEntities;
}
    
void MaterialAssignSystem::GetDropRejectedEntities(DAVA::Vector<const DAVA::Entity *> &rejectedEntities, const DAVA::Entity *entity, const bool recursive)
{
    DAVA::RenderObject * ro = DAVA::GetRenderObject(entity);
    if(ro)
    {
        DAVA::uint32 count = ro->GetRenderBatchCount();
        if(count != 1)
        {
            rejectedEntities.push_back(entity);
        }
    }
    
    if(recursive)
    {
        DAVA::int32 count = entity->GetChildrenCount();
        for(DAVA::int32 i = 0; i < count; ++i)
        {
            GetDropRejectedEntities(rejectedEntities, entity->GetChild(i), recursive);
        }
    }
}

void MaterialAssignSystem::AddSelectedMaterial(DAVA::Set<DAVA::NMaterial *> &materials, const MaterialAssignSystem::DropTestResult &result)
{
    if(result.countEntityUnavailableToDrop == 1)
    {
        QMenu selectMaterialMenu;
        
        DAVA::RenderObject * ro = DAVA::GetRenderObject(result.firstUnavailableEntity);
        if(ro)
        {
            DAVA::uint32 count = ro->GetRenderBatchCount();
            for(DAVA::uint32 m = 0; m < count; ++m)
            {
                DAVA::NMaterial *material = ro->GetRenderBatch(m)->GetMaterial();
                QVariant materialAsVariant = QVariant::fromValue<DAVA::NMaterial *>(material);

                QString text = QString(material->GetParentName().c_str()) + ": " + material->GetMaterialName().c_str();
                QAction *action = selectMaterialMenu.addAction(text);
                action->setData(materialAsVariant);
            }
        }

        QAction * selectedMaterialAction = selectMaterialMenu.exec(QCursor::pos());
        if(selectedMaterialAction)
        {
            QVariant materialAsVariant = selectedMaterialAction->data();
            DAVA::NMaterial * material = materialAsVariant.value<DAVA::NMaterial *>();
            
            materials.insert(material);
        }
    }
}

