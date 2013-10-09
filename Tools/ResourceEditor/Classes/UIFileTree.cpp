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




#include "UIFileTree.h"
#include "UIFileTreeCell.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileList.h"
#include "Utils/Utils.h"
#include "Base/ObjectFactory.h"

namespace DAVA 
{
	
REGISTER_CLASS(UIFileTree);
	

UIFileTree::UIFileTree(const Rect &rect, bool rectInAbsoluteCoordinates)
	: UIList(rect, UIList::ORIENTATION_VERTICAL, rectInAbsoluteCoordinates)
{
	treeHead = 0;
	delegate = 0;
	isFolderNavigationEnabled = false;
    isRootFolderChangeEnabled = true;
    isRootFolderExpandingDisabled = false;
}

UIFileTree::~UIFileTree()
{
	SafeRelease(treeHead);
}

void UIFileTree::SetPath(const FilePath & fullpath, const String & extensionsString)
{
	originalExtensionsString = extensionsString;
	Split(extensionsString, ";", extensions);
	
	SafeRelease(treeHead);
	
	treeHead = new UITreeItemInfo(this);
	treeHead->Set(0, String(), fullpath, true);
	treeHead->isExpanded = true;
    
	RecursiveTreeWalk(fullpath, treeHead);
	
	UIList::SetDelegate(this);
	Refresh();
}

	
int32 UIFileTree::ElementsCount(UIList *forList)
{
	return treeHead->GetTotalCount();
}

UIListCell *UIFileTree::CellAtIndex(UIList *forList, int32 index)
{
    UIFileTreeCell *c = NULL;
    if(delegate)
    {
        UITreeItemInfo * entry = treeHead->EntryByIndex(index);
        c = delegate->CellAtIndex(this, entry, index);
        c->SetItemInfo(entry);
        
        float32 width = forList->GetRect().dx;
        float32 shiftX = entry->GetLevel() * 10.0f;
        c->SetRect(Rect(shiftX, 0, width - shiftX, 16.0));


        c->RemoveEvent(UIControl::EVENT_TOUCH_DOWN, Message(this, &UIFileTree::OnDirectoryChange));
        if (entry->IsDirectory())
        {
            c->AddEvent(UIControl::EVENT_TOUCH_DOWN, Message(this, &UIFileTree::OnDirectoryChange));
        }
    }
    
	return c;//returns cell
    
    
//    int32 width = forList->GetRect().dx;
//    
//	UIFileTreeCell *c = (UIFileTreeCell *)forList->GetReusableCell("FileTreeCell"); //try to get cell from the reusable cells store
//	if(!c)
//	{ //if cell of requested type isn't find in the store create new cell
////		c = new UIFileTreeCell(Rect(0, 0, 200, 20), "FileTreeCell");
//		c = new UIFileTreeCell(Rect(0, 0, width, 20), "FileTreeCell");
//	}
//	//fill cell whith data
//	//c->serverName = GameServer::Instance()->totalServers[index].name + LocalizedString("'s game");
//
//	UITreeItemInfo * entry = treeHead->EntryByIndex(index);
//
////	String empty;
////	for (int k = 0; k < entry->GetLevel(); ++k)
////	{
////		empty += ' ';
////		empty += ' ';
////	}
//	float32 shiftX = entry->GetLevel() * 10.0f;
////	c->SetRect(Rect(shiftX, 0, 200 - shiftX, 16));
//	c->SetRect(Rect(shiftX, 0, width - shiftX, 16));
//	c->SetStateText(UIControl::STATE_NORMAL, StringToWString(entry->GetName()));
//    c->GetStateTextControl(UIControl::STATE_NORMAL)->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
//    c->SetStateText(UIControl::STATE_SELECTED, StringToWString(entry->GetName()));
//	c->GetStateTextControl(UIControl::STATE_SELECTED)->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
//
//    c->SetSelected(false, false);
//    
//	c->SetItemInfo(entry);
//	
//	/*
//		WTF ??? I can't call RemoveAllEvents here.
//	 */
//	c->RemoveEvent(UIControl::EVENT_TOUCH_DOWN, Message(this, &UIFileTree::OnDirectoryChange));
//	
//	if (entry->IsDirectory())
//		c->AddEvent(UIControl::EVENT_TOUCH_DOWN, Message(this, &UIFileTree::OnDirectoryChange));
//	
//	//c->connection = GameServer::Instance()->totalServers[index].connection;
//	//c->serverIndex = GameServer::Instance()->totalServers[index].index;
//
//	return c;//returns cell
//	//your application don't need to manage cells. UIList do all cells management.
//	//you can create cells of your own types derived from the UIListCell
}
	
void UIFileTree::OnDirectoryChange(BaseObject * obj, void * userData, void * callerData)
{
	UIFileTreeCell * cell = dynamic_cast<UIFileTreeCell*> (obj);
	UIEvent * event = reinterpret_cast<UIEvent*> (callerData);	
	if (cell && event)
	{
		Logger::FrameworkDebug("Click count: %d", event->tapCount);
		if (isRootFolderChangeEnabled && (event->tapCount == 2))
		{
			UITreeItemInfo * info = cell->GetItemInfo();
			FilePath pathname = info->GetPathname();
			Logger::FrameworkDebug("Switch to path: %s", pathname.GetAbsolutePathname().c_str());
			SetPath(pathname, originalExtensionsString);
			treeHead->ToggleExpanded();
		}
	}
}


int32 UIFileTree::CellWidth(UIList *forList, int32 index)
{
	return 20; //rect.dx;
}
	
int32 UIFileTree::CellHeight(UIList *forList, int32 index)
{
    if(delegate)
    {
        return delegate->CellHeight(forList, index);
    }
    return 16;
}

    
void UIFileTree::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
	if (delegate)
		delegate->OnCellSelected(this, dynamic_cast<UIFileTreeCell*>(selectedCell));

    int32 index = selectedCell->GetIndex();
    
	UITreeItemInfo * entry = treeHead->EntryByIndex(index);
    
    if(!(isRootFolderExpandingDisabled && 0 == index))
    {
        entry->ToggleExpanded();
        if(entry->IsDirectory())
        {
            Refresh();
        }
    }
	
	
//	Refresh();
};

	
void UIFileTree::RecursiveTreeWalk(const FilePath & path, UITreeItemInfo * current)
{
	FileList * fileList = new FileList(path);
    fileList->Sort();
	
	// Find flags and setup them
	for (int fi = 0; fi < fileList->GetCount(); ++fi)
	{
		bool addElement = true;
		if (!fileList->IsDirectory(fi))
		{
			size_t extsSize = extensions.size();
			if (extsSize > 0)
			{
				addElement = false;
				String ext = fileList->GetPathname(fi).GetExtension();
				for (size_t ei = 0; ei < extsSize; ++ei)
                {
                    if(0 == CompareCaseInsensitive(extensions[ei], ext))
					{
						addElement = true;
						break;
					}
                }
			}
		}
		if (!isFolderNavigationEnabled)
			if (fileList->IsNavigationDirectory(fi))
				addElement = false;

		if (fileList->GetFilename(fi) == ".")
			addElement = false;

		if (addElement)
		{
			UITreeItemInfo *child = new UITreeItemInfo(this);
			child->Set(current->GetLevel() + 1, fileList->GetFilename(fi), fileList->GetPathname(fi), fileList->IsDirectory(fi));
			current->AddChild(child);

//			if (fileList->IsDirectory(fi) )
//			{
//				if (!fileList->IsNavigationDirectory(fi))
//				{
//					RecursiveTreeWalk(path + String("/") + fileList->GetFilename(fi), child);
//				}
//			}
		}
	}
	SafeRelease(fileList);
}

	
void UITreeItemInfo::ToggleExpanded()
{
    String name = GetName();
    
	if ((name == "..") || (name == "."))return;
    
	isExpanded = !isExpanded;
	if (isExpanded)
	{
		RemoveChildren();
		ownerTree->RecursiveTreeWalk(this->GetPathname(), this);
	}
}

void UITreeItemInfo::RemoveChildren()
{
	for (int32 k = 0; k < (int32) children.size(); ++k) 
		SafeRelease(children[k]);
	children.clear();	
}
	
int32 UITreeItemInfo::GetTotalCount()
{
	int32 cnt = 0;
	if (isExpanded)
	{
        if(ownerTree)
        {
            UITreeItemInfo *inf = new UITreeItemInfo(ownerTree);
            inf->level = level;
            inf->pathname = GetPathname();
            inf->name = GetName();
            
            inf->isDirectory = isDirectory;
            inf->isExpanded = isExpanded;
            
            ownerTree->RecursiveTreeWalk(this->GetPathname(), inf);
            
            //remove unused
            for (Vector<UITreeItemInfo *>::iterator it = children.begin(); 
                 it < children.end(); )
            {
                UITreeItemInfo *item = (UITreeItemInfo *)(*it);
                bool wasFound = false;
                for (int32 iNew = 0; iNew < (int32)inf->children.size(); ++iNew)
                {
                    if(item->name == inf->children[iNew]->name)
                    {
                        wasFound = true;
                        break;
                    }
                }
                
                if(!wasFound)
                {
                    SafeRelease(item);
                    children.erase(it);
                    it = children.begin();
                }
                else
                {
                    ++it;
                }
            }
            //add new
            Vector<UITreeItemInfo *>::iterator it = children.begin();
            for (int32 iNew = 0; iNew < (int32)inf->children.size(); ++iNew, ++it)
            {
                bool wasFound = false;
                for (int32 iOld = 0; iOld < (int32)children.size(); ++iOld)
                {
                    if(children[iOld]->name == inf->children[iNew]->name)
                    {
                        wasFound = true;
                        break;
                    }
                }
                
                if(!wasFound)
                {
                    children.insert(it, SafeRetain(inf->children[iNew]));
                    for (it = children.begin(); it < children.end(); ++it)
                    {
                        if((*it)->name == inf->children[iNew]->name)
                        {
                            break;
                        }
                    }
                    
//                    children.push_back(SafeRetain(inf->children[iNew]));
                }
            }
            
            SafeRelease(inf);
        }        
        
		for (int32 c = 0; c < (int32)children.size(); ++c)
			cnt += children[c]->GetTotalCount();
	}
	return cnt + 1;
}
	
UITreeItemInfo * UITreeItemInfo::EntryByIndex(int32 index)
{
	int32 cnt = 0;
	if (index == 0)return this;
	index--;
	if (isExpanded)
	{
		for (int32 c = 0; c < (int32)children.size(); ++c)
		{
			int32 childCnt = children[c]->GetTotalCount();
			if ((index >= cnt) && (index < cnt + childCnt))
				return children[c]->EntryByIndex(index - cnt);
			cnt += childCnt;
		}
	}
	return 0;
}
	
void UIFileTree::SetDelegate(UIFileTreeDelegate * _delegate)
{
	delegate = _delegate;
}
	
void UIFileTree::SetFolderNavigation(bool isEnabled)
{
	isFolderNavigationEnabled = isEnabled;
}
				
void UIFileTree::Refresh()
{
    UIList::Refresh();
}
    
void UIFileTree::EnableRootFolderChange(bool isEnabled)
{
    isRootFolderChangeEnabled = isEnabled;
}
    
void UIFileTree::DisableRootFolderExpanding(bool isDisabled)
{
    isRootFolderExpandingDisabled = isDisabled;
}
    
    
};

