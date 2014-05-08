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

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QAction>
#include <QVariant>

#include "MaterialEditor.h"
#include "ui_materialeditor.h"

#include "MaterialModel.h"
#include "MaterialFilterModel.h"

#include "Main/mainwindow.h"
#include "Main/QtUtils.h"
#include "QualitySwitcher/QualitySwitcher.h"
#include "Project/ProjectManager.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataIntrospection.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataInspMember.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataInspDynamic.h"
#include "Tools/QtPropertyEditor/QtPropertyDataValidator/TexturePathValidator.h"
#include "Qt/Settings/SettingsManager.h"
#include "Commands2/MaterialGlobalCommand.h"

#include "Scene3D/Systems/QualitySettingsSystem.h"

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"
#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"

#define MATERIAL_NAME_LABEL "Name"
#define MATERIAL_GROUP_LABEL "Group"
#define MATERIAL_PROPERTIES_LABEL "Properties"
#define MATERIAL_TEXTURES_LABEL "Textures"
#define MATERIAL_TEMPLATE_LABEL "Template"

MaterialEditor::MaterialEditor(QWidget *parent /* = 0 */)
: QDialog(parent)
, ui(new Ui::MaterialEditor)
, templatesFilterModel( NULL )
, lastCheckState(CHECKED_ALL)
{
	ui->setupUi(this);
	setWindowFlags(WINDOWFLAG_ON_TOP_OF_APPLICATION);

	ui->materialTree->setDragEnabled(true);
	ui->materialTree->setAcceptDrops(true);
	ui->materialTree->setDragDropMode(QAbstractItemView::DragDrop);

	ui->materialProperty->SetEditTracking(true);
    //ui->materialProperty->setSortingEnabled(true);
    //ui->materialProperty->header()->setSortIndicator(0, Qt::AscendingOrder);

	// global scene manager signals
	QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(sceneActivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(sceneDeactivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2 *, const Command2*, bool)), this, SLOT(commandExecuted(SceneEditor2 *, const Command2 *, bool)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)), this, SLOT( autoExpand() ));
	
	// material tree
	QObject::connect(ui->materialTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(materialSelected(const QItemSelection &, const QItemSelection &)));
    QObject::connect(ui->materialTree, SIGNAL(Updated()), this, SLOT(autoExpand()));
    QObject::connect(ui->materialTree, SIGNAL(ContextMenuPrepare(QMenu *)), this, SLOT(onContextMenuPrepare(QMenu *)));

	// material properties
	QObject::connect(ui->materialProperty, SIGNAL(PropertyEdited(const QModelIndex &)), this, SLOT(OnPropertyEdited(const QModelIndex &)));
	QObject::connect(ui->templateBox, SIGNAL(activated(int)), this, SLOT(OnTemplateChanged(int)));
    QObject::connect(ui->actionAddGlobalMaterial, SIGNAL(triggered(bool)), this, SLOT(OnMaterialAddGlobal(bool)));
    QObject::connect(ui->actionRemoveGlobalMaterial, SIGNAL(triggered(bool)), this, SLOT(OnMaterialRemoveGlobal(bool)));
    QObject::connect(ui->actionSaveMaterialPreset, SIGNAL(triggered(bool)), this, SLOT(OnMaterialSave(bool)));
    QObject::connect(ui->actionLoadMaterialPreset, SIGNAL(triggered(bool)), this, SLOT(OnMaterialLoad(bool)));

	posSaver.Attach(this);
	posSaver.LoadState(ui->splitter);
	treeStateHelper = new PropertyEditorStateHelper(ui->materialProperty, (QtPropertyModel *) ui->materialProperty->model());

	DAVA::VariantType v1 = posSaver.LoadValue("splitPosProperties");
	DAVA::VariantType v2 = posSaver.LoadValue("splitPosPreview");
	if(v1.GetType() == DAVA::VariantType::TYPE_INT32) ui->materialProperty->header()->resizeSection(0, v1.AsInt32());
	if(v2.GetType() == DAVA::VariantType::TYPE_INT32) ui->materialProperty->header()->resizeSection(1, v2.AsInt32());

    DAVA::VariantType savePath = posSaver.LoadValue("lastSavePath");
    DAVA::VariantType loadState = posSaver.LoadValue("lastLoadState");
    if(savePath.GetType() == DAVA::VariantType::TYPE_FILEPATH) lastSavePath = savePath.AsFilePath();
    if(loadState.GetType() == DAVA::VariantType::TYPE_UINT32) lastCheckState = loadState.AsUInt32();

    expandMap[MaterialFilteringModel::SHOW_ALL] = false;
    expandMap[MaterialFilteringModel::SHOW_ONLY_INSTANCES] = true;
    expandMap[MaterialFilteringModel::SHOW_INSTANCES_AND_MATERIALS] = true;

    initActions();
}

MaterialEditor::~MaterialEditor()
{ 
	delete treeStateHelper;

	DAVA::VariantType v1(ui->materialProperty->header()->sectionSize(0));
	DAVA::VariantType v2(ui->materialProperty->header()->sectionSize(1));
	posSaver.SaveValue("splitPosProperties", v1);
	posSaver.SaveValue("splitPosPreview", v2);
    posSaver.SaveValue("lastSavePath", DAVA::VariantType(lastSavePath));
    posSaver.SaveValue("lastLoadState", DAVA::VariantType(lastCheckState));
	posSaver.SaveState(ui->splitter);
}

void MaterialEditor::initActions()
{
    ui->actionShowAll->setData( MaterialFilteringModel::SHOW_ALL );
    ui->actionInstances->setData( MaterialFilteringModel::SHOW_ONLY_INSTANCES );
    ui->actionMaterialsInstances->setData( MaterialFilteringModel::SHOW_INSTANCES_AND_MATERIALS );

    const int filterType =  MaterialFilteringModel::SHOW_ALL;
    foreach( QAction *action, ui->filterType->actions() )
    {
        connect( action, SIGNAL( triggered() ), SLOT( onFilterChanged() ) );
        if ( action->data().toInt() == filterType )
            action->activate( QAction::Trigger );
    }

    connect( ui->actionAutoExpand, SIGNAL( triggered( bool ) ), SLOT( onCurrentExpandModeChange( bool ) ) );
}

void MaterialEditor::initTemplates()
{
    if ( templatesFilterModel )
        return ;

    QStandardItemModel *templatesModel = new QStandardItemModel( this );

    const QVector<ProjectManager::AvailableMaterialTemplate> *templates = ProjectManager::Instance()->GetAvailableMaterialTemplates();
    QStandardItem *emptyItem = new QStandardItem( QString() );
    templatesModel->appendRow( emptyItem );

    for(int i = 0; i < templates->size(); ++i)
    {
        QStandardItem *item = new QStandardItem();
        item->setText( templates->at(i).name );
        item->setData( templates->at(i).path, Qt::UserRole );
        templatesModel->appendRow( item );
    }

    templatesFilterModel = new MaterialTemplateModel( this );
    templatesFilterModel->setSourceModel( templatesModel );

    ui->templateBox->setModel( templatesFilterModel );
}

void MaterialEditor::setTemplatePlaceholder( const QString& text )
{
    QAbstractItemModel *model = ui->templateBox->model();
    const QModelIndex index = model->index( 0, 0 );
    model->setData( index, text, Qt::DisplayRole );
}

void MaterialEditor::autoExpand()
{
    QAction *action = ui->filterType->checkedAction();
    if ( action == NULL )
        return ;
    const int filterType = action->data().toInt();
    
    if ( expandMap[filterType] )
        ui->materialTree->expandAll();
}

void MaterialEditor::onFilterChanged()
{
    QAction *action = ui->filterType->checkedAction();
    if ( action == NULL )
        return ;
    const int filterType = action->data().toInt();
    ui->materialTree->setFilterType( filterType );

    ui->actionAutoExpand->setChecked( expandMap[filterType] );
    onCurrentExpandModeChange( expandMap[filterType] );
}

void MaterialEditor::SelectMaterial(DAVA::NMaterial *material)
{
	ui->materialTree->SelectMaterial(material);
}

void MaterialEditor::SelectEntities(DAVA::NMaterial *material)
{
    QList<DAVA::NMaterial *> materials;
    materials << material;
	ui->materialTree->SelectEntities(materials);
}

void MaterialEditor::SetCurMaterial(const QList< DAVA::NMaterial *>& materials)
{
	curMaterials = materials;

	treeStateHelper->SaveTreeViewState(false);

	FillMaterialProperties(materials);
    FillMaterialTemplates(materials);

	// Restore back the tree view state from the shared storage.
	if(!treeStateHelper->IsTreeStateStorageEmpty())
	{
		treeStateHelper->RestoreTreeViewState();
	}
	else
	{
		// Expand the root elements as default value.
		ui->materialProperty->expandToDepth(0);
	}

    // check if there is global material and enable appropriate actions
    SceneEditor2 *sceneEditor = QtMainWindow::Instance()->GetCurrentScene();
    if(NULL != sceneEditor)
    {
        bool isGlobalMaterialPresent = (NULL != sceneEditor->GetGlobalMaterial());
        ui->actionAddGlobalMaterial->setEnabled(!isGlobalMaterialPresent);
        ui->actionRemoveGlobalMaterial->setEnabled(isGlobalMaterialPresent);
    }
    else
    {
        ui->actionAddGlobalMaterial->setEnabled(false);
        ui->actionRemoveGlobalMaterial->setEnabled(false);
    }
}

void MaterialEditor::sceneActivated(SceneEditor2 *scene)
{
	if(isVisible())
	{
        SetCurMaterial(QList< DAVA::NMaterial *>());
		ui->materialTree->SetScene(scene);
        autoExpand();
	}
}

void MaterialEditor::sceneDeactivated(SceneEditor2 *scene)
{ 
    ui->materialTree->SetScene(NULL);
    SetCurMaterial(QList< DAVA::NMaterial *>());
}

void MaterialEditor::materialSelected(const QItemSelection & selected, const QItemSelection & deselected)
{
    QList< DAVA::NMaterial *> materials;
    QItemSelectionModel *selection = ui->materialTree->selectionModel();
    const QModelIndexList selectedRows = selection->selectedRows();

    foreach ( const QModelIndex& index, selectedRows )
    {
        if ( index.column() == 0 )
        {
            DAVA::NMaterial *material = ui->materialTree->GetMaterial( index );
            if ( material )
                materials << material;
        }
    }

    SetCurMaterial( materials );
}

void MaterialEditor::commandExecuted(SceneEditor2 *scene, const Command2 *command, bool redo)
{
    if(scene == QtMainWindow::Instance()->GetCurrentScene())
    {
        int cmdId = command->GetId();

        if(cmdId == CMDID_INSP_DYNAMIC_MODIFY ||
           cmdId == CMDID_INSP_MEMBER_MODIFY ||
           cmdId == CMDID_META_OBJ_MODIFY)
        {
            SetCurMaterial(curMaterials);
        }
        else if(cmdId == CMDID_MATERIAL_GLOBAL_SET)
        {
            sceneActivated(scene);
            SetCurMaterial(curMaterials);
        }
    }
}

void MaterialEditor::onCurrentExpandModeChange( bool mode )
{
    QAction *action = ui->filterType->checkedAction();
    if ( action == NULL )
        return ;
    const int filterType = action->data().toInt();
    expandMap[filterType] = mode;

    if ( mode )
    {
        ui->materialTree->expandAll();
    }
    else
    {
        ui->materialTree->collapseAll();
    }
}

void MaterialEditor::showEvent(QShowEvent * event)
{
    FillMaterialTemplates(QList<DAVA::NMaterial *>());
	sceneActivated(QtMainWindow::Instance()->GetCurrentScene());
}

void MaterialEditor::FillMaterialProperties(const QList<DAVA::NMaterial *>& materials)
{
    // Clear current properties
    // But don't remove properties immediately. Just extract them and remove later
    // this should be done, because we want to remove all properties when propertyEdited signal emited
    {
        QtPropertyData *propRoot = ui->materialProperty->GetRootProperty();
        while(propRoot->ChildCount() > 0)
        {
            QtPropertyData *child = propRoot->ChildGet(0);
            propRoot->ChildExtract(child);

            child->deleteLater();
        }
    }

    if ( materials.count() == 0 )
        return ;

    foreach ( DAVA::NMaterial *material, materials )
    {
        DVASSERT( material );

	    const DAVA::InspInfo *info = material->GetTypeInfo();
	    const DAVA::InspMember *materialProperties = info->Member("materialProperties");
	    const DAVA::InspMember *materialFlags = info->Member("materialSetFlags");
	    const DAVA::InspMember *materialIllumination = info->Member("illuminationParams");
        const DAVA::InspMember *materialTextures = info->Member("textures");

//         {
//             const DAVA::InspInfo *baseInfo = info->BaseInfo()->BaseInfo();
//             QtPropertyData *refCount = new QtPropertyDataInspMember(material, baseInfo->Member("referenceCount"));
//             refCount->SetName("referenceCount");
//             ui->materialProperty->MergeProperty(refCount);
//         }


	    // fill material name
	    const DAVA::InspMember *nameMember = info->Member("materialName");
	    if(NULL != nameMember)
	    {
		    QtPropertyDataInspMember *name = new QtPropertyDataInspMember(material, nameMember);
            name->SetName(MATERIAL_NAME_LABEL);
		    ui->materialProperty->MergeProperty(name);
	    }

        // fill material group, only for material type
        if(material->GetMaterialType() == DAVA::NMaterial::MATERIALTYPE_MATERIAL)
        {
            const DAVA::InspMember *groupMember = info->Member("materialGroup");
            if(NULL != groupMember)
            {
                QtPropertyDataInspMember *group = new QtPropertyDataInspMember(material, groupMember);
                group->SetName(MATERIAL_GROUP_LABEL);
                ui->materialProperty->MergeProperty(group);

                for(size_t i = 0; i < DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityGroupCount(); ++i)
                {
                    DAVA::FastName groupName = DAVA::QualitySettingsSystem::Instance()->GetMaterialQualityGroupName(i);
                    group->AddAllowedValue(DAVA::VariantType(groupName), groupName.c_str());
                }
            }
        }

	    QtPropertyData *propertiesParent = new QtPropertyData();

	    // fill material flags
	    if(NULL != materialFlags)
	    {
		    const DAVA::InspMemberDynamic* dynamicInsp = materialFlags->Dynamic();

		    if(NULL != dynamicInsp)
		    {
			    DAVA::InspInfoDynamic *dynamicInfo = dynamicInsp->GetDynamicInfo();
			    DAVA::Vector<DAVA::FastName> membersList = dynamicInfo->MembersList(material); // this function can be slow
			
			    for(size_t i = 0; i < membersList.size(); ++i)
			    {
                    int memberFlags = dynamicInfo->MemberFlags(material, membersList[i]);
				    QtPropertyDataInspDynamic *dynamicMember = new QtPropertyDataInspDynamic(material, dynamicInfo, membersList[i]);

                    if(material->GetMaterialType() != DAVA::NMaterial::MATERIALTYPE_GLOBAL)
                    {
                        // self property
                        if(memberFlags & DAVA::I_EDIT)
                        {
                            QtPropertyToolButton* btn = dynamicMember->AddButton();
                            btn->setIcon(QIcon(":/QtIcons/cminus.png"));
                            btn->setIconSize(QSize(14, 14));
                            QObject::connect(btn, SIGNAL(clicked()), this, SLOT(OnRemFlag()));
                        }
                        else if(memberFlags)
                        {
                            dynamicMember->SetEnabled(false);
                            for(int m = 0; m < dynamicMember->ChildCount(); ++m)
                            {
                                dynamicMember->ChildGet(m)->SetEnabled(false);
                            }

                            QtPropertyToolButton* btn = dynamicMember->AddButton();
                            btn->setIcon(QIcon(":/QtIcons/cplus.png"));
                            btn->setIconSize(QSize(14, 14));
                            QObject::connect(btn, SIGNAL(clicked()), this, SLOT(OnAddFlag()));

                            dynamicMember->SetBackground(QBrush(QColor(0, 0, 0, 10)));
                        }
                    }

				    propertiesParent->ChildAdd(membersList[i].c_str(), dynamicMember);
			    }
		    }
	    }

	    // fill material properties
	    if(NULL != materialProperties)
	    {
		    const DAVA::InspMemberDynamic* dynamicInsp = materialProperties->Dynamic();

		    if(NULL != dynamicInsp)
		    {
			    DAVA::InspInfoDynamic *dynamicInfo = dynamicInsp->GetDynamicInfo();
			    DAVA::Vector<DAVA::FastName> membersList = dynamicInfo->MembersList(material); // this function can be slow

			    for(size_t i = 0; i < membersList.size(); ++i)
			    {
				    QtPropertyDataInspDynamic *dynamicMember = new QtPropertyDataInspDynamic(material, dynamicInfo, membersList[i]);
                    int memberFlags = dynamicInfo->MemberFlags(material, membersList[i]);

                    if(material->GetMaterialType() != DAVA::NMaterial::MATERIALTYPE_GLOBAL)
                    {
				        // self property
				        if(memberFlags & DAVA::I_EDIT)
				        {
					        QtPropertyToolButton* btn = dynamicMember->AddButton();
					        btn->setIcon(QIcon(":/QtIcons/cminus.png"));
					        btn->setIconSize(QSize(14, 14));
					        QObject::connect(btn, SIGNAL(clicked()), this, SLOT(OnRemProperty()));

					        // isn't set in parent or shader
					        if(!(memberFlags & DAVA::I_VIEW) && !(memberFlags & DAVA::I_SAVE))
					        {
						        dynamicMember->SetBackground(QBrush(QColor(255, 0, 0, 10)));
					        }
				        }
				        // not self property (is set in parent or shader)
				        else
				        {
					        // disable property and it childs
					        dynamicMember->SetEnabled(false);
					        for(int m = 0; m < dynamicMember->ChildCount(); ++m)
					        {
						        dynamicMember->ChildGet(m)->SetEnabled(false);
					        }

					        QtPropertyToolButton* btn = dynamicMember->AddButton();
					        btn->setIcon(QIcon(":/QtIcons/cplus.png"));
					        btn->setIconSize(QSize(14, 14));
					        QObject::connect(btn, SIGNAL(clicked()), this, SLOT(OnAddProperty()));

					        dynamicMember->SetBackground(QBrush(QColor(0, 0, 0, 10)));

					        // required by shader
					        //if(!(memberFlags & DAVA::I_VIEW) && (memberFlags & DAVA::I_SAVE))
					        //{	}
				        }
                    }

				    propertiesParent->ChildAdd(membersList[i].c_str(), dynamicMember);
			    }
		    }
	    }

        propertiesParent->SetName(MATERIAL_PROPERTIES_LABEL);
	    ui->materialProperty->MergeProperty(propertiesParent);
        if ( propertiesParent->Parent() != NULL )
	        ui->materialProperty->ApplyStyle(propertiesParent, QtPropertyEditor::HEADER_STYLE);

	    // fill illumination params
	    if(NULL != materialIllumination)
	    {
		    QtPropertyData *illumParams = QtPropertyDataIntrospection::CreateMemberData(material, materialIllumination);

		    if(illumParams->ChildCount() > 0)
		    {
                illumParams->SetName(materialIllumination->Name());
			    ui->materialProperty->MergeProperty(illumParams);
                if ( illumParams->Parent() != NULL )
			        ui->materialProperty->ApplyStyle(illumParams, QtPropertyEditor::HEADER_STYLE);
		    }
		    else
		    {
			    delete illumParams;
		    }
	    }

        // fill own material textures
        if(NULL != materialTextures)
        {
            const DAVA::InspMemberDynamic* dynamicInsp = materialTextures->Dynamic();
            QtPropertyData *texturesParent = new QtPropertyData();

            if(NULL != dynamicInsp)
            {
                DAVA::InspInfoDynamic *dynamicInfo = dynamicInsp->GetDynamicInfo();
                DAVA::Vector<DAVA::FastName> membersList = dynamicInfo->MembersList(material); // this function can be slow

				QString defaultPath = ProjectManager::Instance()->CurProjectPath().GetAbsolutePathname().c_str();
				FilePath dataSourcePath = ProjectManager::Instance()->CurProjectDataSourcePath();
				if (dataSourcePath.Exists())
				{
					defaultPath = dataSourcePath.GetAbsolutePathname().c_str();
				}
				SceneEditor2* editor = QtMainWindow::Instance()->GetCurrentScene();
				if (NULL != editor && editor->GetScenePath().Exists())
				{
					DAVA::String scenePath = editor->GetScenePath().GetDirectory().GetAbsolutePathname();
					if (String::npos != scenePath.find(dataSourcePath.GetAbsolutePathname()))
					{
						defaultPath = scenePath.c_str();
					}
				}
                for(size_t i = 0; i < membersList.size(); ++i)
                {
                    int memberFlags = dynamicInfo->MemberFlags(material, membersList[i]);
                    QtPropertyDataInspDynamic *dynamicMember = new QtPropertyDataInspDynamic(material, dynamicInfo, membersList[i]);

                    dynamicMember->SetDefaultOpenDialogPath(defaultPath);
                    dynamicMember->SetOpenDialogFilter("All (*.tex *.png);;PNG (*.png);;TEX (*.tex)");
                    QStringList path;
					path.append(dataSourcePath.GetAbsolutePathname().c_str());
                    dynamicMember->SetValidator(new TexturePathValidator(path));
                    
                    if(material->GetMaterialType() != DAVA::NMaterial::MATERIALTYPE_GLOBAL)
                    {
                        // self property
                        if(memberFlags & DAVA::I_EDIT)
                        {
                            QtPropertyToolButton* btn = dynamicMember->AddButton();
                            btn->setIcon(QIcon(":/QtIcons/cminus.png"));
                            btn->setIconSize(QSize(14, 14));
                            QObject::connect(btn, SIGNAL(clicked()), this, SLOT(OnRemTexture()));

                            // isn't set in parent or shader
                            if(!(memberFlags & DAVA::I_VIEW) && !(memberFlags & DAVA::I_SAVE))
                            {
                                dynamicMember->SetBackground(QBrush(QColor(255, 0, 0, 10)));
                            }
                        }
                        // not self property (is set in parent or shader)
                        else
                        {
                            // disable property and it childs
                            dynamicMember->SetEnabled(false);
                            for(int m = 0; m < dynamicMember->ChildCount(); ++m)
                            {
                                dynamicMember->ChildGet(m)->SetEnabled(false);
                            }

                            QtPropertyToolButton* btn = dynamicMember->AddButton();
                            btn->setIcon(QIcon(":/QtIcons/cplus.png"));
                            btn->setIconSize(QSize(14, 14));
                            QObject::connect(btn, SIGNAL(clicked()), this, SLOT(OnAddTexture()));

                            dynamicMember->SetBackground(QBrush(QColor(0, 0, 0, 10)));
                        }
                    }

                    texturesParent->ChildAdd(membersList[i].c_str(), dynamicMember);
                }

                texturesParent->SetName(MATERIAL_TEXTURES_LABEL);
                ui->materialProperty->MergeProperty(texturesParent);
                if ( texturesParent->Parent() != NULL )
                    ui->materialProperty->ApplyStyle(texturesParent, QtPropertyEditor::HEADER_STYLE);
            }
        }        
    }
}

void MaterialEditor::FillMaterialTemplates(const QList<DAVA::NMaterial *>& materials)
{
    initTemplates();

    const int nMaterials = materials.count();
    bool enableTemplate = ( nMaterials > 0 );
    bool isTemplatesSame = true;
    int rowToSelect = -1;
    const QString curMaterialTemplate = ( nMaterials > 0 && NULL != materials[0]->GetMaterialTemplate()) ? materials[0]->GetMaterialTemplate()->name.c_str() : QString();
    QString placeHolder;

    if ( nMaterials > 0)
    {
        for ( int i = 0; i < nMaterials; i++ )
        {
            DAVA::NMaterial *material = materials[i];
            // Test template name
            if ( isTemplatesSame && (NULL != material->GetMaterialTemplate()) && (curMaterialTemplate != material->GetMaterialTemplate()->name.c_str()) )
            {
                isTemplatesSame = false;
            }
            // Test material flags
            if( material->GetMaterialType() != DAVA::NMaterial::MATERIALTYPE_MATERIAL ||
               (material->GetNodeGlags() & DAVA::DataNode::NodeRuntimeFlag) )
            {
                enableTemplate = false;
            }
            if ( !isTemplatesSame && !enableTemplate )
                break;
        }

        if ( !isTemplatesSame )
            enableTemplate = false;
    }

    if ( nMaterials !=  1)
        enableTemplate = false;

    if ( isTemplatesSame )
    {
        QAbstractItemModel *model = ui->templateBox->model();
        const int n = model->rowCount();
        const int pathRole = Qt::UserRole;
        for ( int i = 0; i < n; i++ )
        {
            const QModelIndex index = model->index( i, 0 );
            if ( index.data( pathRole ).toString() == curMaterialTemplate )
            {
                rowToSelect = i;
                break;
            }
        }
    }

    const bool isTemplateFound = (rowToSelect != -1);

    if ( isTemplatesSame )
    {
        if ( !isTemplateFound )
        {
            placeHolder = QString( "NON-ASSIGNABLE: %1" ).arg( curMaterialTemplate );
            rowToSelect = 0;
        }
    }
    else
    {
        if ( nMaterials > 0 )
        {
            placeHolder = QString( "Different templates selected" );    // TODO: fix text?
            rowToSelect = 0;
        }
    }
    setTemplatePlaceholder( placeHolder );
    ui->templateBox->setCurrentIndex( rowToSelect );
    ui->templateBox->setEnabled( enableTemplate );
}

void MaterialEditor::OnAddProperty()
{
	QtPropertyToolButton *btn = dynamic_cast<QtPropertyToolButton *>(QObject::sender());
	
    if(NULL != btn && curMaterials.size() > 0 )
	{
		QtPropertyDataInspDynamic *data = dynamic_cast<QtPropertyDataInspDynamic *>(btn->GetPropertyData());
		if(NULL != data)
		{
            QList< QtPropertyDataInspDynamic * > dataList;
            const int nMerged = data->GetMergedCount();
            dataList.reserve( nMerged );
            for ( int i = 0; i < nMerged; i++ )
            {
                QtPropertyDataInspDynamic *dynamicData = dynamic_cast<QtPropertyDataInspDynamic *>(data->GetMergedData(i));
                if (dynamicData != NULL)
                    dataList << dynamicData;
            }

            data->SetVariantValue(data->GetAliasVariant());
            for ( int i = 0; i < dataList.size(); i++ )
            {
                QtPropertyDataInspDynamic *dynamicData = dataList.at(i);
			    dynamicData->SetVariantValue(data->GetAliasVariant());
            }
            data->SetValue(data->GetValue(), QtPropertyData::VALUE_EDITED);
			data->EmitDataChanged(QtPropertyData::VALUE_EDITED);

			// reload material properties
			SetCurMaterial(curMaterials);
		}
	}
}

void MaterialEditor::OnRemProperty()
{
	QtPropertyToolButton *btn = dynamic_cast<QtPropertyToolButton *>(QObject::sender());
	
    if(NULL != btn && curMaterials.size() > 0 )
	{
		QtPropertyDataInspDynamic *data = dynamic_cast<QtPropertyDataInspDynamic *>(btn->GetPropertyData());
		if(NULL != data)
		{
            QList< QtPropertyDataInspDynamic * > dataList;
            const int nMerged = data->GetMergedCount();
            dataList.reserve( nMerged + 1 );
            dataList << data;
            for ( int i = 0; i < nMerged; i++ )
            {
                QtPropertyDataInspDynamic *dynamicData = dynamic_cast<QtPropertyDataInspDynamic *>(data->GetMergedData(i));
                if (dynamicData != NULL)
                    dataList << dynamicData;
            }

            for ( int i = 0; i < dataList.size(); i++ )
            {
                QtPropertyDataInspDynamic *dynamicData = dataList.at(i);
                dynamicData->SetValue(QVariant(), QtPropertyData::VALUE_EDITED);
            }

			// reload material properties
			SetCurMaterial(curMaterials);
		}
	}
}

void MaterialEditor::OnAddTexture()
{
	QtPropertyToolButton *btn = dynamic_cast<QtPropertyToolButton *>(QObject::sender());
	
    if(NULL != btn && curMaterials.size() > 0 )
	{
		QtPropertyDataInspDynamic *data = dynamic_cast<QtPropertyDataInspDynamic *>(btn->GetPropertyData());
		if(NULL != data)
		{
            QList< QtPropertyDataInspDynamic * > dataList;
            const int nMerged = data->GetMergedCount();
            dataList.reserve( nMerged );
            for ( int i = 0; i < nMerged; i++ )
            {
                QtPropertyDataInspDynamic *dynamicData = dynamic_cast<QtPropertyDataInspDynamic *>(data->GetMergedData(i));
                if (dynamicData != NULL)
                    dataList << dynamicData;
            }

            data->SetVariantValue(data->GetAliasVariant());
            for ( int i = 0; i < dataList.size(); i++ )
            {
                QtPropertyDataInspDynamic *dynamicData = dataList.at(i);
			    dynamicData->SetVariantValue(data->GetAliasVariant());
            }
            data->SetValue(data->GetValue(), QtPropertyData::VALUE_EDITED);
			data->EmitDataChanged(QtPropertyData::VALUE_EDITED);

			// reload material properties
			SetCurMaterial(curMaterials);
		}
	}
}

void MaterialEditor::OnRemTexture()
{
	QtPropertyToolButton *btn = dynamic_cast<QtPropertyToolButton *>(QObject::sender());
	
    if(NULL != btn && curMaterials.size() > 0 )
	{
		QtPropertyDataInspDynamic *data = dynamic_cast<QtPropertyDataInspDynamic *>(btn->GetPropertyData());
		if(NULL != data)
		{
            QList< QtPropertyDataInspDynamic * > dataList;
            const int nMerged = data->GetMergedCount();
            dataList.reserve( nMerged + 1 );
            dataList << data;
            for ( int i = 0; i < nMerged; i++ )
            {
                QtPropertyDataInspDynamic *dynamicData = dynamic_cast<QtPropertyDataInspDynamic *>(data->GetMergedData(i));
                if (dynamicData != NULL)
                    dataList << dynamicData;
            }

            for ( int i = 0; i < dataList.size(); i++ )
            {
                QtPropertyDataInspDynamic *dynamicData = dataList.at(i);
                dynamicData->SetValue(QVariant(), QtPropertyData::VALUE_EDITED);
            }

			// reload material properties
			SetCurMaterial(curMaterials);
		}
	}
}

void MaterialEditor::OnAddFlag()
{
    QtPropertyToolButton *btn = dynamic_cast<QtPropertyToolButton *>(QObject::sender());

    if(NULL != btn && curMaterials.size() > 0)
    {
        QtPropertyDataInspDynamic *data = dynamic_cast<QtPropertyDataInspDynamic *>(btn->GetPropertyData());
        if(NULL != data)
        {
            const QVariant value = data->GetValue();
            data->SetValue(value, QtPropertyData::VALUE_EDITED);

            // reload material properties
            SetCurMaterial(curMaterials);
        }
    }
}

void MaterialEditor::OnRemFlag()
{
    QtPropertyToolButton *btn = dynamic_cast<QtPropertyToolButton *>(QObject::sender());

    if(NULL != btn && curMaterials.size() > 0)
    {
        QtPropertyDataInspDynamic *data = dynamic_cast<QtPropertyDataInspDynamic *>(btn->GetPropertyData());
        if(NULL != data)
        {
            data->SetValue(QVariant(), QtPropertyData::VALUE_EDITED);

            // reload material properties
            SetCurMaterial(curMaterials);
        }
    }
}

void MaterialEditor::OnTemplateChanged(int index)
{
    if(curMaterials.size() == 1 && index > 0)
    {
        DAVA::NMaterial *material = curMaterials.at(0);
        QString newTemplatePath = GetTemplatePath(index);
        if(!newTemplatePath.isEmpty())
        {
            const DAVA::InspMember *templateMember = material->GetTypeInfo()->Member("materialTemplate");

            if(NULL != templateMember)
            {
                QtMainWindow::Instance()->GetCurrentScene()->Exec(new InspMemberModifyCommand(templateMember, material, 
                    DAVA::VariantType(DAVA::FastName(newTemplatePath.toStdString().c_str()))));
            }
        }
    }

    SetCurMaterial(curMaterials);
}

QString MaterialEditor::GetTemplatePath(int index) const
{
    return ui->templateBox->itemData(index, Qt::UserRole).toString();
}

void MaterialEditor::OnPropertyEdited(const QModelIndex &index)
{
	QtPropertyEditor *editor = dynamic_cast<QtPropertyEditor *>(QObject::sender());
	if(NULL != editor)
	{
		QtPropertyData *propData = editor->GetProperty(index);

		if(NULL != propData)
		{
            QList<QtPropertyData *> propDatList; //propData->GetMergedData();
            propDatList.reserve( propData->GetMergedCount() + 1 );
            for ( int i = 0; i < propData->GetMergedCount(); i++ )
                propDatList << propData->GetMergedData( i );
            propDatList << propData;

            QList<Command2 *> commands;
            foreach ( QtPropertyData *data, propDatList )
            {
                Command2 *command = (Command2 *) data->CreateLastCommand();
                if ( command )
                {
                    commands << command;
                }
            }
            const bool usebatch = (commands.count() > 1);

            SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();
            if (curScene)
            {
                if (usebatch)
                {
                    QObject::disconnect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2 *, const Command2*, bool)), this, SLOT(commandExecuted(SceneEditor2 *, const Command2 *, bool)));
                    curScene->BeginBatch( "Property multiedit" );
                }

                for (int i = 0; i < commands.size(); i++)
                {
                    Command2 *cmd = commands.at( i );
                    curScene->Exec( cmd );
                }

                if (usebatch)
                {
                    curScene->EndBatch();
                    QObject::connect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2 *, const Command2*, bool)), this, SLOT(commandExecuted(SceneEditor2 *, const Command2 *, bool)));
                    SetCurMaterial( curMaterials );
                }
            }
		}
	}
}

QVariant MaterialEditor::CheckForTextureDescriptor(const QVariant& value)
{
    if (value.type() == QVariant::String)
    {
        String s = value.toString().toStdString();
        FilePath fp = FilePath(s);
        if (!fp.IsEmpty() && fp.Exists())
        {
            if (fp.GetExtension() == ".png")
            {
                TextureDescriptorUtils::CreateDescriptorIfNeed(fp);
                FilePath texFile = TextureDescriptor::GetDescriptorPathname(fp);
                return QVariant(QString::fromStdString(texFile.GetAbsolutePathname()));
            }
        }
    }
    return QVariant();
}

void MaterialEditor::OnMaterialAddGlobal(bool checked)
{
    SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();
    if(NULL != curScene)
    {
        DAVA::NMaterial *global = NMaterial::CreateGlobalMaterial(FastName("Scene_Global_Material"));
        curScene->Exec(new MaterialGlobalSetCommand(curScene, global));
        SafeRelease(global);

        sceneActivated(curScene);
        SelectMaterial(curScene->GetGlobalMaterial());
    }
}

void MaterialEditor::OnMaterialRemoveGlobal(bool checked)
{
    SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();
    if(NULL != curScene)
    {
        curScene->Exec(new MaterialGlobalSetCommand(curScene, NULL));
        sceneActivated(curScene);
    }
}

void MaterialEditor::onContextMenuPrepare(QMenu *menu)
{
    if(curMaterials.size() > 0)
    {
        ui->actionSaveMaterialPreset->setEnabled(curMaterials.size() == 1);
        menu->addSeparator();
        menu->addAction(ui->actionLoadMaterialPreset);
        menu->addAction(ui->actionSaveMaterialPreset);
    }
}

void MaterialEditor::OnMaterialSave(bool checked)
{
    if(curMaterials.size() == 1)
    {
        QString outputFile = QFileDialog::getSaveFileName(this, "Save Material Preset", lastSavePath.GetAbsolutePathname().c_str(), "Material Preset (*.mpreset)");
        SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();

        if(!outputFile.isEmpty() && NULL != curScene)
        {
            DAVA::NMaterial *material = curMaterials[0];

            DAVA::KeyedArchive *materialArchive = new DAVA::KeyedArchive();
            DAVA::SerializationContext materialContext;

            materialContext.SetScene(curScene);
            materialContext.SetScenePath(ProjectManager::Instance()->CurProjectPath());

            lastSavePath = outputFile.toAscii().data();
            material->Save(materialArchive, &materialContext);
            materialArchive->Save(lastSavePath);
            materialArchive->Release();
        }
    }
    else
    {
        QMessageBox::warning(this, "Material properties save", "It is allowed to save only single material");
    }
}

void MaterialEditor::OnMaterialLoad(bool checked)
{
    if(curMaterials.size() > 0)
    {
        QString inputFile = QFileDialog::getOpenFileName(this, "Load Material Preset", lastSavePath.GetAbsolutePathname().c_str(), "Material Preset (*.mpreset)");
        SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();
        DAVA::NMaterial *material = curMaterials[0];

        if(!inputFile.isEmpty() && NULL != curScene)
        {
            DAVA::KeyedArchive *materialArchive = new DAVA::KeyedArchive();

            lastSavePath = inputFile.toAscii().data();
            materialArchive->Load(lastSavePath);

            DAVA::uint32 userChoiseWhatToLoad = ExecMaterialLoadingDialog(lastCheckState, inputFile);
            if(0 != userChoiseWhatToLoad)
            {
	            const DAVA::InspInfo *info = material->GetTypeInfo();
	            const DAVA::InspMember *materialProperties = info->Member("materialProperties");
	            const DAVA::InspMember *materialFlags = info->Member("materialSetFlags");
                const DAVA::InspMember *materialTextures = info->Member("textures");

                lastCheckState = userChoiseWhatToLoad;

                if(!(lastCheckState & CHECKED_GROUP))
                {
                    materialArchive->DeleteKey("materialGroup");
                }

                if(!(lastCheckState & CHECKED_TEMPLATE))
                {
                    materialArchive->DeleteKey("materialTemplate");
                }

                if(!(lastCheckState & CHECKED_PROPERTIES))
                {
                    materialArchive->DeleteKey("properties");
                    materialArchive->DeleteKey("setFlags");
                }
                else
                {
                    ClearMaterialDynamicMember(material, materialFlags->Dynamic());
                    ClearMaterialDynamicMember(material, materialProperties->Dynamic());
                }

                if(!(lastCheckState & CHECKED_TEXTURES))
                {
                    materialArchive->DeleteKey("textures");
                }
                else
                {
                    ClearMaterialDynamicMember(material, materialTextures->Dynamic());
                }

                materialArchive->DeleteKey("##name");
                materialArchive->DeleteKey("#id");
                materialArchive->DeleteKey("#index");
                materialArchive->DeleteKey("materialName");
                materialArchive->DeleteKey("materialType");
                materialArchive->DeleteKey("parentMaterialKey");

                DAVA::SerializationContext materialContext;
                materialContext.SetScene(curScene);
                materialContext.SetScenePath(ProjectManager::Instance()->CurProjectPath());
                material->Load(materialArchive, &materialContext);
            }            materialArchive->Release();        }
    }

    SetCurMaterial(curMaterials);
}

void MaterialEditor::ClearMaterialDynamicMember(DAVA::NMaterial *material, const DAVA::InspMemberDynamic *dynamicMember)
{
	if(material->GetMaterialType() != DAVA::NMaterial::MATERIALTYPE_GLOBAL && NULL != dynamicMember)
	{
		DAVA::InspInfoDynamic *dynamicInfo = dynamicMember->GetDynamicInfo();
		DAVA::Vector<DAVA::FastName> membersList = dynamicInfo->MembersList(material); // this function can be slow
        for(size_t i = 0; i < membersList.size(); ++i)
		{
            dynamicInfo->MemberValueSet(material, membersList[i], DAVA::VariantType());
        }
    }
}

DAVA::uint32 MaterialEditor::ExecMaterialLoadingDialog(DAVA::uint32 initialState, const QString &inputFile)
{
    DAVA::uint32 ret = 0;

    QDialog *dlg = new QDialog(this);
    QVBoxLayout *dlgLayout = new QVBoxLayout();
    dlgLayout->setMargin(10);
    dlgLayout->setSpacing(15);

    dlg->setWindowTitle("Loading material preset...");
    dlg->setWindowFlags(Qt::Tool);
    dlg->setLayout(dlgLayout);

    QLineEdit* pathLine = new QLineEdit(dlg);
    pathLine->setText(inputFile);
    pathLine->setReadOnly(false);
    pathLine->setToolTip(inputFile);
    dlgLayout->addWidget(pathLine);

    QGroupBox *groupbox = new QGroupBox("Load parameters", dlg);
    dlgLayout->addWidget(groupbox);

    QCheckBox *templateChBox = new QCheckBox(MATERIAL_TEMPLATE_LABEL, groupbox);
    QCheckBox *groupChBox = new QCheckBox(MATERIAL_GROUP_LABEL, groupbox);
    QCheckBox *propertiesChBox = new QCheckBox(MATERIAL_PROPERTIES_LABEL, groupbox);
    QCheckBox *texturesChBox = new QCheckBox(MATERIAL_TEXTURES_LABEL, groupbox);

    templateChBox->setChecked((bool) (initialState & CHECKED_TEMPLATE));
    groupChBox->setChecked((bool) (initialState & CHECKED_GROUP));
    propertiesChBox->setChecked((bool) (initialState & CHECKED_PROPERTIES));
    texturesChBox->setChecked((bool) (initialState & CHECKED_TEXTURES));

    QGridLayout *gridLayout = new QGridLayout();
    groupbox->setLayout(gridLayout);
    gridLayout->setHorizontalSpacing(50);
    gridLayout->addWidget(templateChBox, 0, 0);
    gridLayout->addWidget(groupChBox, 1, 0);
    gridLayout->addWidget(propertiesChBox, 0, 1);
    gridLayout->addWidget(texturesChBox, 1, 1);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, dlg);
    dlgLayout->addWidget(buttons);

    QObject::connect(buttons, SIGNAL(accepted()), dlg, SLOT(accept()));
    QObject::connect(buttons, SIGNAL(rejected()), dlg, SLOT(reject()));

    if(QDialog::Accepted == dlg->exec())
    {
        if(templateChBox->checkState() == Qt::Checked) ret |= CHECKED_TEMPLATE;
        if(groupChBox->checkState() == Qt::Checked) ret |= CHECKED_GROUP;
        if(propertiesChBox->checkState() == Qt::Checked) ret |= CHECKED_PROPERTIES;
        if(texturesChBox->checkState() == Qt::Checked) ret |= CHECKED_TEXTURES;
    }

    delete dlg;
    return ret;
}

