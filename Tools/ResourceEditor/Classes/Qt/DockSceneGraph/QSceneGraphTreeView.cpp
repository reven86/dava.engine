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



#include "QSceneGraphTreeView.h"

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Scene/SceneData.h"
#include "../Scene/SceneDataManager.h"

#include "../Commands/CommandsManager.h"
#include "../Commands/SceneGraphCommands.h"
#include "../Commands/LibraryCommands.h"
#include "../Commands/FileCommands.h"

#include "SceneGraphModel.h"

#include "../../LandscapeEditor/LandscapesController.h"
#include "../../SceneEditor/SceneEditorScreenMain.h"
#include "../../SceneEditor/EditorBodyControl.h"
#include "../../AppScreens.h"
#include "../StringConstants.h"

#include <QKeyEvent>

QSceneGraphTreeView::QSceneGraphTreeView(QWidget *parent)
    :   QTreeView(parent),
		sceneGraphModel(NULL)
{
	sceneGraphModel = new SceneGraphModel();
    sceneGraphModel->SetScene(NULL);
	
	setDragDropMode(QAbstractItemView::InternalMove);
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);

	ConnectToSignals();
}

QSceneGraphTreeView::~QSceneGraphTreeView()
{
	DisconnectFromSignals();
	SafeDelete(sceneGraphModel);
}

void QSceneGraphTreeView::ConnectToSignals()
{
	connect(sceneGraphModel, SIGNAL(SceneNodeSelected(DAVA::Entity *)), this, SLOT(OnSceneNodeSelectedInGraph(DAVA::Entity *)));

	// Signals to rebuild the particular node and the whole graph.
	connect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedRebuildNode(DAVA::Entity*)), this, SLOT(OnSceneGraphNeedRebuildNode(DAVA::Entity*)));
	connect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedRebuild()), this, SLOT(OnSceneGraphNeedRebuild()));
	connect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedRefreshLayer(DAVA::ParticleLayer*)), this, SLOT(OnSceneGraphNeedRefreshLayer(DAVA::ParticleLayer*)));

	connect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedSetScene(SceneData*, EditorScene*)),
			this, SLOT(OnSceneGraphNeedSetScene(SceneData*, EditorScene*)));
	connect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedSelectNode(SceneData*, DAVA::Entity*)),
			this, SLOT(OnSceneGraphNeedSelectNode(SceneData*, DAVA::Entity*)));

	// Signals related to the whole Scene.
	connect(SceneDataManager::Instance(), SIGNAL(SceneCreated(SceneData*)),	this, SLOT(OnSceneCreated(SceneData*)));
	connect(SceneDataManager::Instance(), SIGNAL(SceneReleased(SceneData*)), this, SLOT(OnSceneReleased(SceneData*)));
	connect(SceneDataManager::Instance(), SIGNAL(SceneActivated(SceneData*)),	this, SLOT(OnSceneActivated(SceneData*)));
	connect(SceneDataManager::Instance(), SIGNAL(SceneDeactivated(SceneData*)), this, SLOT(OnSceneDeactivated(SceneData*)));
}

void QSceneGraphTreeView::DisconnectFromSignals()
{
	disconnect(sceneGraphModel, SIGNAL(SceneNodeSelected(DAVA::Entity *)), this, SLOT(OnSceneNodeSelectedInGraph(DAVA::Entity *)));
	
    if(SceneDataManager::Instance())
    {
        disconnect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedRebuildNode(DAVA::Entity*)), this, SLOT(OnSceneGraphNeedRebuildNode(DAVA::Entity*)));
        disconnect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedRebuild()), this, SLOT(OnSceneGraphNeedRebuild()));
        disconnect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedRefreshLayer(DAVA::ParticleLayer*)), this, SLOT(OnSceneGraphNeedRefreshLayer(DAVA::ParticleLayer*)));
        
        
        disconnect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedSetScene(SceneData*, EditorScene*)),
                   this, SLOT(OnSceneGraphNeedSetScene(SceneData*, EditorScene*)));
        disconnect(SceneDataManager::Instance(), SIGNAL(SceneGraphNeedSelectNode(SceneData*, DAVA::Entity*)),
                   this, SLOT(OnSceneGraphNeedSelectNode(SceneData*, DAVA::Entity*)));
        
        // Signals related to the whole Scene.
        disconnect(SceneDataManager::Instance(), SIGNAL(SceneCreated(SceneData*)),	this, SLOT(OnSceneCreated(SceneData*)));
        disconnect(SceneDataManager::Instance(), SIGNAL(SceneReleased(SceneData*)), this, SLOT(OnSceneReleased(SceneData*)));
        disconnect(SceneDataManager::Instance(), SIGNAL(SceneActivated(SceneData*)),	this, SLOT(OnSceneActivated(SceneData*)));
        disconnect(SceneDataManager::Instance(), SIGNAL(SceneDeactivated(SceneData*)), this, SLOT(OnSceneDeactivated(SceneData*)));
    }
}

void QSceneGraphTreeView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_X)
    {
        SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
        if(screen)
        {
            screen->ProcessIsSolidChanging();
        }
    }
    
    QTreeView::keyPressEvent(event);
}

void QSceneGraphTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
	QTreeView::mouseDoubleClickEvent(event);

	if(SceneDataManager::Instance()->SceneGetActive())
		SceneDataManager::Instance()->SceneGetActive()->LockAtSelectedNode();
}
void QSceneGraphTreeView::OnSceneNodeSelectedInGraph(DAVA::Entity *node)
{
	// TODO: Yuri Coder, 12/21/2012. Think about the nicer method.
	SceneDataManager::Instance()->SceneNodeSelectedInSceneGraph(node);
}

void QSceneGraphTreeView::OnSceneCreated(SceneData* scene)
{
	this->sceneGraphModel->SetScene(scene->GetScene());
}

void QSceneGraphTreeView::OnSceneReleased(SceneData* scene)
{
	this->sceneGraphModel->SetScene(NULL);
}

void QSceneGraphTreeView::OnSceneActivated(SceneData* scene)
{
	this->sceneGraphModel->SetScene(scene->GetScene());
    this->sceneGraphModel->Activate(this);
    
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnSceneGraphContextMenuRequested(const QPoint &)));
}

void QSceneGraphTreeView::OnSceneDeactivated(SceneData* scene)
{
    sceneGraphModel->Deactivate();
    
	disconnect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OnSceneGraphContextMenuRequested(const QPoint &)));
}

void QSceneGraphTreeView::OnSceneGraphNeedRebuildNode(DAVA::Entity* node)
{
	sceneGraphModel->RebuildNode(node);
}

void QSceneGraphTreeView::OnSceneGraphNeedRebuild()
{
	sceneGraphModel->Rebuild();
}

void QSceneGraphTreeView::OnSceneGraphNeedSetScene(SceneData *sceneData, EditorScene *scene)
{
	sceneGraphModel->SetScene(scene);
}

void QSceneGraphTreeView::OnSceneGraphNeedSelectNode(SceneData *sceneData, DAVA::Entity* node)
{
	sceneGraphModel->SelectNode(node);
}

void QSceneGraphTreeView::OnSceneGraphContextMenuRequested(const QPoint &point)
{
    QModelIndex itemIndex = indexAt(point);
    ShowSceneGraphMenu(itemIndex, QCursor::pos());
}

void QSceneGraphTreeView::ShowSceneGraphMenu(const QModelIndex &index, const QPoint &point)
{
    if(!index.isValid())
    {
        return;
    }
    
    QMenu menu;
    
	// For "custom" Particles Editor nodes the "generic" ones aren't needed".
    if (sceneGraphModel->GetParticlesEditorSceneModelHelper().NeedDisplaySceneEditorPopupMenuItems(index))
    {
		SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
		LandscapesController *landsacpesController = activeScene->GetLandscapesController();

		SceneEditorScreenMain *screen = static_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen(SCREEN_MAIN_OLD));
		EditorBodyControl *c = screen->FindCurrentBody()->bodyControl;

		bool canChangeScene = !landsacpesController->EditorLandscapeIsActive() && !c->LandscapeEditorActive();
		if(!canChangeScene)
			return;



		AddActionToMenu(&menu, QString("Look at Object"), new CommandLockAtObject());
		AddActionToMenu(&menu, QString("Remove Object"), new CommandRemoveSceneNode());
	
		AddActionToMenu(&menu, QString("Debug Flags"), new CommandDebugFlags());
    
		Entity *node = static_cast<Entity *>(sceneGraphModel->ItemData(index));
		if (node)
		{
            SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
            if(node->GetParent() == activeScene->GetScene())
            {
                KeyedArchive *properties = node->GetCustomProperties();
                if (properties && properties->IsKeyExists(String(ResourceEditor::EDITOR_REFERENCE_TO_OWNER)))
                {
                    String filePathname = properties->GetString(String(ResourceEditor::EDITOR_REFERENCE_TO_OWNER));

                    AddActionToMenu(&menu, QString("Remove Root Nodes"), new CommandRemoveRootNodes());
				}
			}
			FilePath filePathForSaveAs(activeScene->GetScenePathname());
			AddActionToMenu(&menu, QString("Save Scene As"), new CommandSaveSpecifiedScene(node, filePathForSaveAs));
		}
	}
	
	// For "custom" Particles Editor nodes the "generic" ones aren't needed".
    // We might need more menu items/actions for Particles Editor.
    sceneGraphModel->GetParticlesEditorSceneModelHelper().AddPopupMenuItems(menu, index);

    connect(&menu, SIGNAL(triggered(QAction *)), this, SLOT(SceneGraphMenuTriggered(QAction *)));
    menu.exec(point);
}

void QSceneGraphTreeView::SceneGraphMenuTriggered(QAction *action)
{
	ProcessContextMenuAction(action);
}

void QSceneGraphTreeView::AddActionToMenu(QMenu *menu, const QString &actionTitle, Command *command)
{
    QAction *action = menu->addAction(actionTitle);
    action->setData(PointerHolder<Command *>::ToQVariant(command));
}

void QSceneGraphTreeView::ProcessContextMenuAction(QAction *action)
{
	Command *command = PointerHolder<Command *>::ToPointer(action->data());
	CommandsManager::Instance()->ExecuteAndRelease(command,
												   SceneDataManager::Instance()->SceneGetActive()->GetScene());
}

void QSceneGraphTreeView::OnSceneGraphNeedRefreshLayer(DAVA::ParticleLayer* layer)
{
	sceneGraphModel->RefreshParticlesLayer(layer);
}
