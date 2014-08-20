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


#ifndef __UIEDITOR__IMPORTCOMMANDS__
#define __UIEDITOR__IMPORTCOMMANDS__

#include "BaseCommand.h"
#include "ItemsCommand.h"
#include "../UI/Dialogs/importdialog.h"

// Base command for Import Platform/Screen/Aggregator commands
// Command can control it's execution status by overriding IsUndoRedoSupported() to return executionResult
// Which should be set to true if execution was succesfull, or to false otherwise
// Thereby if command fails - it would not be added to undo queue
class ImportNodesCommand: public UndoableHierarchyTreeNodeCommand
{
public:
	ImportNodesCommand(const Vector<ImportDialog::FileItem>& files);

	virtual bool IsUndoRedoSupported();

protected:
	HierarchyTreeNode::HIERARCHYTREENODEID platformId;
	HierarchyTreeNode::HIERARCHYTREENODESLIST importedNodes;
	Vector<ImportDialog::FileItem> aggregatorFiles;
	Vector<ImportDialog::FileItem> screenFiles;

	Vector<QString> importErrorsAggregators;
	MultiMap<QString, QString> importErrorsScreens;

	static Set<QString> GetNodeAggregatorControls(const HierarchyTreeNode* node);
	static Set<QString> GetNodeAggregators(HierarchyTreeNode* node);
	Set<QString> GetUnimportedAggregators(HierarchyTreeNode* node,
										  HierarchyTreePlatformNode* platform);

	bool LoadAggregators();
	bool LoadScreens();
	virtual void Cleanup() = 0;
	virtual void ShowErrorMessage(const QString& message);

	void SetExecutionResult(bool result);
	bool GetExecutionResult();
	
	virtual void IncrementUnsavedChanges();
	virtual void DecrementUnsavedChanges();

	void ReplaceAggregatorControls();

private:
	bool executionResult;
};

class ImportScreensCommand: public ImportNodesCommand
{
public:
	ImportScreensCommand(HierarchyTreeNode::HIERARCHYTREENODEID platformId,
						 const Vector<ImportDialog::FileItem>& files);

	virtual void Execute();
	virtual void Rollback();

private:
	void ReturnNodesToScene();
	void PrepareRemoveFromSceneInformationForNodes();
	void SetParents();
	virtual void Cleanup();
};

class ImportPlatformCommand: public ImportNodesCommand
{
public:
	ImportPlatformCommand(const QString& platformPath,
						  const QString& platformName,
						  const QSize& platformSize,
						  const Vector<ImportDialog::FileItem>& files);

	virtual void Execute();
	virtual void Rollback();

private:
	QString platformPath;
	QString platformName;
	QSize platformSize;

	virtual void Cleanup();
};

#endif /* defined(__UIEDITOR__IMPORTCOMMANDS__) */
