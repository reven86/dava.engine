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



#include "DAVAEngine.h"
#include "Entity/Component.h"
#include "Main/mainwindow.h"

#include <QPushButton>
#include <QFile>
#include <QTextStream>

#include "DockProperties/PropertyEditor.h"
#include "Tools/QtPropertyEditor/QtPropertyDataProxy.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataIntrospection.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataDavaVariant.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataDavaKeyedArchive.h"
#include "Commands2/MetaObjModifyCommand.h"
#include "Commands2/InspMemberModifyCommand.h"

#include "PropertyEditorStateHelper.h"

#include "ActionComponentEditor.h"

PropertyEditor::PropertyEditor(QWidget *parent /* = 0 */, bool connectToSceneSignals /*= true*/)
	: QtPropertyEditor(parent)
	, viewMode(VIEW_NORMAL)
	, curNode(NULL)
	, treeStateHelper(this, curModel)
	, favoriteGroup(NULL)
{
	if(connectToSceneSignals)
	{
		QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(sceneActivated(SceneEditor2 *)));
		QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(sceneDeactivated(SceneEditor2 *)));
		QObject::connect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2 *, const Command2*, bool)), this, SLOT(CommandExecuted(SceneEditor2 *, const Command2*, bool )));
		QObject::connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)), this, SLOT(sceneSelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)));
	}
	posSaver.Attach(this, "DocPropetyEditor");

	DAVA::VariantType v = posSaver.LoadValue("splitPos");
	if(v.GetType() == DAVA::VariantType::TYPE_INT32) header()->resizeSection(0, v.AsInt32());

	SetUpdateTimeout(5000);
	SetEditTracking(true);
	setMouseTracking(true);

	LoadScheme("~doc:/PropEditorDefault.scheme");
}

PropertyEditor::~PropertyEditor()
{
	DAVA::VariantType v(header()->sectionSize(0));
	posSaver.SaveValue("splitPos", v);

	SafeRelease(curNode);
}

void PropertyEditor::SetEntities(const EntityGroup *selected)
{
/*
	DAVA::KeyedArchive *ka = new DAVA::KeyedArchive();
	ResetProperties();
	AppendProperty("test", new QtPropertyDataDavaKeyedArcive(ka));

	return;
*/

    //TODO: support multiselected editing

	SafeRelease(curNode);
    if(NULL != selected && selected->Size() == 1)
 	{
         curNode = SafeRetain(selected->GetEntity(0));

		 // ensure that custom properties exist
		 // this call will create them if they are not created yet
		 curNode->GetCustomProperties();
 	}

    ResetProperties();
	SaveScheme("~doc:/PropEditorDefault.scheme");
}

void PropertyEditor::SetViewMode(eViewMode mode)
{
	if(viewMode != mode)
	{
		viewMode = mode;
        ResetProperties();
	}
}

PropertyEditor::eViewMode PropertyEditor::GetViewMode() const
{
	return viewMode;
}

void PropertyEditor::SetFavoritesEditMode(bool set)
{
	if(favoritesEditMode != set)
	{
		favoritesEditMode = set;
		ResetProperties();
	}
}

bool PropertyEditor::GetFavoritesEditMode() const
{
	return favoritesEditMode;
}

void PropertyEditor::ResetProperties()
{
    // Store the current Property Editor Tree state before switching to the new node.
	// Do not clear the current states map - we are using one storage to share opened
	// Property Editor nodes between the different Scene Nodes.
	treeStateHelper.SaveTreeViewState(false);

	RemovePropertyAll();
	favoriteGroup = NULL;

	if(NULL != curNode)
	{
		// create data tree, but don't add it to the property editor
		QtPropertyData *root = new QtPropertyData();

		// add info about current entity
		QtPropertyData *curEntityData = CreateInsp(curNode, curNode->GetTypeInfo());
		root->ChildAdd(curNode->GetTypeInfo()->Name(), curEntityData);

		// add info about components
		for(int32 i = 0; i < Component::COMPONENT_COUNT; ++i)
		{
			Component *component = curNode->GetComponent(i);
			if(component)
			{
				QtPropertyData *componentData = CreateInsp(component, component->GetTypeInfo());
				root->ChildAdd(component->GetTypeInfo()->Name(), componentData);
			}
		}

		ApplyModeFilter(root);
		ApplyFavorite(root);
		ApplyCustomButtons(root);

		// add not empty rows from root
		while(0 != root->ChildCount())
		{
			QtPropertyData *row = root->ChildGet(0);
			root->ChildExtract(row);

			if(row->ChildCount() > 0)
			{
				AppendProperty(row->GetName(), row);
				ApplyStyle(row, QtPropertyEditor::HEADER_STYLE);
			}
		}

		delete root;
	}
    
	// Restore back the tree view state from the shared storage.
	if (!treeStateHelper.IsTreeStateStorageEmpty())
	{
		treeStateHelper.RestoreTreeViewState();
	}
	else
	{
		// Expand the root elements as default value.
		expandToDepth(0);
	}
}

void PropertyEditor::ApplyModeFilter(QtPropertyData *parent)
{
	if(NULL != parent)
	{
		for(int i = 0; i < parent->ChildCount(); ++i)
		{
			bool toBeRemove = false;
			QtPropertyData *data = parent->ChildGet(i);

			// show only editable items and favorites
			if(viewMode == VIEW_NORMAL)
			{
				if(!data->IsEditable())
				{
					toBeRemove = true;
				}
			}
			// show all editable/viewable items
			else if(viewMode == VIEW_ADVANCED)
			{

			}
			// show only favorite items
			else if(viewMode == VIEW_FAVORITES_ONLY)
			{
				toBeRemove = true;
			}

			// apply mode to data childs
			ApplyModeFilter(data);

			if(toBeRemove)
			{
				if(scheme.contains(data->GetPath()))
				{
					parent->ChildExtract(data);

					GetUserData(data)->isOriginalFavorite = true;
					SetFavorite(data, true);
				}
				else
				{
					parent->ChildRemove(data);
				}

				i--;
			}
		}
	}
}

void PropertyEditor::ApplyFavorite(QtPropertyData *data)
{
	if(NULL != data)
	{
		if(scheme.contains(data->GetPath()))
		{
			SetFavorite(data, true);
		}

		// go through childs
		for(int i = 0; i < data->ChildCount(); ++i)
		{
			ApplyFavorite(data->ChildGet(i));
		}
	}
}

void PropertyEditor::ApplyCustomButtons(QtPropertyData *data)
{
	if(NULL != data)
	{
		const DAVA::MetaInfo *meta = data->MetaInfo();

		if(NULL != meta)
		{
			if(DAVA::MetaInfo::Instance<DAVA::ActionComponent>() == meta)
			{
				// Add optional button to edit action component
				QToolButton *editActions = data->AddButton();
				editActions->setIcon(QIcon(":/QtIcons/settings.png"));
				editActions->setAutoRaise(true);

				QObject::connect(editActions, SIGNAL(pressed()), this, SLOT(ActionEditComponent()));
			}
			else if(DAVA::MetaInfo::Instance<DAVA::RenderObject>() == meta)
			{
				// Add optional button to bake transform render object
				QToolButton *bakeButton = data->AddButton();
				bakeButton->setToolTip("Bake Transform");
				bakeButton->setIcon(QIcon(":/QtIcons/transform_bake.png"));
				bakeButton->setIconSize(QSize(12, 12));
				bakeButton->setAutoRaise(true);

				QObject::connect(bakeButton, SIGNAL(pressed()), this, SLOT(ActionBakeTransform()));
			}
		}

		// go through childs
		for(int i = 0; i < data->ChildCount(); ++i)
		{
			ApplyCustomButtons(data->ChildGet(i));
		}
	}
}

QtPropertyData* PropertyEditor::CreateInsp(void *object, const DAVA::InspInfo *info)
{
	QtPropertyData *ret = NULL;

	if(NULL != info)
	{
		bool hasMembers = false;
		const InspInfo *baseInfo = info;

		// check if there are any members in introspection
		while (NULL != baseInfo)
		{
			if(baseInfo->MembersCount() > 0)
			{
				hasMembers = true;
				break;
			}
			baseInfo = baseInfo->BaseInfo();
		}

		// add them
        if(hasMembers)
        {
			ret = new QtPropertyData();
			ret->SetEnabled(false);

			while(NULL != baseInfo)
			{
				for(int i = 0; i < baseInfo->MembersCount(); ++i)
				{
					const DAVA::InspMember *member = baseInfo->Member(i);
					QtPropertyData *memberData = CreateInspMember(object, member);

					ret->ChildAdd(member->Name(), memberData);
				}

				baseInfo = baseInfo->BaseInfo();
			}
		}
    }

	return ret;
}

QtPropertyData* PropertyEditor::CreateInspMember(void *object, const DAVA::InspMember *member)
{
	QtPropertyData* ret = NULL;

	if(NULL != member && (member->Flags() & DAVA::I_VIEW))
	{
		void *momberObject = member->Data(object);
		const DAVA::InspInfo *memberIntrospection = member->Type()->GetIntrospection(momberObject);

		if(NULL != memberIntrospection && 
			member->Type() != DAVA::MetaInfo::Instance<DAVA::KeyedArchive*>())
		{
			ret = CreateInsp(momberObject, memberIntrospection);
		}
		else
		{
			ret = QtPropertyDataIntrospection::CreateMemberData(object, member);
		}
	}

	return ret;
}

void PropertyEditor::sceneActivated(SceneEditor2 *scene)
{
	if(NULL != scene)
	{
        const EntityGroup selection = scene->selectionSystem->GetSelection();
		SetEntities(&selection);
	}
}

void PropertyEditor::sceneDeactivated(SceneEditor2 *scene)
{
	SetEntities(NULL);
}

void PropertyEditor::sceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected)
{
    SetEntities(selected);
}

void PropertyEditor::CommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo)
{
	int cmdId = command->GetId();

	switch (cmdId)
	{
	case CMDID_COMPONENT_ADD:
	case CMDID_COMPONENT_REMOVE:
	case CMDID_CONVERT_TO_SHADOW:
	case CMDID_PARTICLE_EMITTER_LOAD_FROM_YAML:
		ResetProperties();
		break;
	default:
		Update();
		break;
	}
}

void PropertyEditor::OnItemEdited(const QModelIndex &index)
{
	QtPropertyEditor::OnItemEdited(index);

	QtPropertyData *propData = GetProperty(index);

	if(NULL != propData)
	{
		Command2 *command = (Command2 *) propData->CreateLastCommand();
		if(NULL != command)
		{
			SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();
			if(NULL != curScene)
			{
				curScene->Exec(command);
			}
		}
	}
}

void PropertyEditor::mouseReleaseEvent(QMouseEvent *event)
{
	bool skipEvent = false;
	QModelIndex index = indexAt(event->pos());

	// handle favorite state toggle for item under mouse
	if(favoritesEditMode && index.parent().isValid() && index.column() == 0)
	{
		QRect rect = visualRect(index);
		rect.setX(0);
		rect.setWidth(16);

		if(rect.contains(event->pos()))
		{
			QtPropertyData *data = GetProperty(index);
			if(NULL != data && !IsParentFavorite(data))
			{
				SetFavorite(data, !IsFavorite(data));
				skipEvent = true;
			}
		}
	}

	if(!skipEvent)
	{
		QtPropertyEditor::mouseReleaseEvent(event);
	}
}

void PropertyEditor::drawRow(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	static QIcon favIcon = QIcon(":/QtIcons/star.png");
	static QIcon nfavIcon = QIcon(":/QtIcons/star_empty.png");

	// custom draw for favorites edit mode
	QStyleOptionViewItemV4 opt = option;
	if(index.parent().isValid() && favoritesEditMode)
	{
		QtPropertyData *data = GetProperty(index);
		if(NULL != data)
		{
			if(!IsParentFavorite(data))
			{
				if(IsFavorite(data))
				{
					favIcon.paint(painter, opt.rect.x(), opt.rect.y(), 16, opt.rect.height());
				}
				else
				{
					nfavIcon.paint(painter, opt.rect.x(), opt.rect.y(), 16, opt.rect.height());
				}
			}
		}
	}

	QtPropertyEditor::drawRow(painter, opt, index);
}

void PropertyEditor::ActionEditComponent()
{
	if(NULL != curNode)
	{
		ActionComponentEditor editor;

		editor.SetComponent((DAVA::ActionComponent*)curNode->GetComponent(DAVA::Component::ACTION_COMPONENT));
		editor.exec();
	}	
}

void PropertyEditor::ActionBakeTransform()
{
	if(NULL != curNode)
	{
		DAVA::RenderObject * ro = GetRenderObject(curNode);
		if(NULL != ro)
		{
			ro->BakeTransform(curNode->GetLocalTransform());
			curNode->SetLocalTransform(DAVA::Matrix4::IDENTITY);
		}
	}
}

bool PropertyEditor::IsParentFavorite(QtPropertyData *data) const
{
	bool ret = false;

	QtPropertyData *parent = data->Parent();
	while(NULL != parent)
	{
		if(IsFavorite(parent))
		{
			ret = true;
			break;
		}
		else
		{
			parent = parent->Parent();
		}
	}

	return ret;
}

bool PropertyEditor::IsFavorite(QtPropertyData *data) const
{
	bool ret = false;

	if(NULL != data)
	{
		PropEditorUserData *userData = GetUserData(data);
		ret = userData->isFavorite;
	}

	return ret;
}

void PropertyEditor::SetFavorite(QtPropertyData *data, bool favorite)
{
	if(NULL == favoriteGroup)
	{
		favoriteGroup = GetProperty(InsertHeader("Favorites", 0));
	}

	if(NULL != data)
	{
		QtPropertyData *original = data->GetProxyOriginal();
		PropEditorUserData *originalUserData = GetUserData(original);

		// original data
		if(data == original)
		{
			originalUserData->isFavorite = favorite;

			// this original data is already in favorite group or should be added there
			// (happens, when data is marked as favorite, but it shouldn't be displayed due
			// to current view mode filtering)
			if(originalUserData->isOriginalFavorite)
			{
				if(favorite)
				{
					favoriteGroup->ChildAdd(original->GetName(), original);
				}
				else
				{
					favoriteGroup->ChildRemove(original);
				}
			}
			// original data should have proxy to be displayed in favorites
			else
			{
				if(favorite)
				{
					QtPropertyData* proxy = new QtPropertyDataProxy(original);
					PropEditorUserData *proxyUserData = GetUserData(proxy);

					proxyUserData->isFavorite = true;
					originalUserData->favoriteProxy = proxy;

					favoriteGroup->ChildAdd(original->GetName(), proxy);
					scheme.insert(original->GetPath());
				}
				else
				{
					favoriteGroup->ChildRemove(originalUserData->favoriteProxy);
					scheme.remove(original->GetPath());
				}

				original->EmitDataChanged(QtPropertyData::VALUE_SET);
			}
		}
		// proxy data (can only be removed)
		else
		{
			originalUserData->isFavorite = false;

			favoriteGroup->ChildRemove(data);
			scheme.remove(original->GetPath());
		}
	}

	// if there is no favorite items - remove favorite group
	if(favoriteGroup->ChildCount() == 0)
	{
		RemoveProperty(favoriteGroup);
		favoriteGroup = NULL;
	}
}

PropEditorUserData* PropertyEditor::GetUserData(QtPropertyData *data) const
{
	PropEditorUserData *userData = (PropEditorUserData*) data->GetUserData();
	if(NULL == userData)
	{
		userData = new PropEditorUserData();
		data->SetUserData(userData);
	}

	return userData;
}

void PropertyEditor::LoadScheme(const DAVA::FilePath &path)
{
	// first, we open the file
	QFile file(path.GetAbsolutePathname().c_str());
	if(file.open(QIODevice::ReadOnly))
	{
		scheme.clear();

		QTextStream qin(&file);
		while(!qin.atEnd())
		{
			scheme.insert(qin.readLine());
		}

 		file.close();
	}
}

void PropertyEditor::SaveScheme(const DAVA::FilePath &path)
{
	// first, we open the file
	QFile file(path.GetAbsolutePathname().c_str());
	if(file.open(QIODevice::WriteOnly))
	{
		QTextStream qout(&file);
		foreach(const QString &value, scheme)
		{
			qout << value << endl;
		}
		file.close();
	}
}
