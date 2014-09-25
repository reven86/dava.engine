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


#ifndef __UIEDITOR__IMPORTDIALOG__
#define __UIEDITOR__IMPORTDIALOG__

#include <QDialog>
#include "DAVAEngine.h"
#include "HierarchyTreeNode.h"

using namespace DAVA;

namespace Ui {
	class ImportDialog;
}

class ImportDialogFileWidget;
class ImportDialog : public QDialog
{
	Q_OBJECT

public:
	enum eImportType
	{
		IMPORT_PLATFORM = 0,
		IMPORT_SCREEN,
		IMPORT_TYPE_COUNT
	};

	enum eAction
	{
		ACTION_IGNORE = 0,
		ACTION_SCREEN,
		ACTION_AGGREGATOR,
		ACTIONS_COUNT
	};

	struct FileItem
	{
		QString fileName;
		eAction action;
		QSize size;
	};

	explicit ImportDialog(eImportType type, QWidget *parent = 0, const FilePath& platformPath = FilePath(""));

	~ImportDialog();

	HierarchyTreeNode::HIERARCHYTREENODEID GetPlatformId();
	Vector<FileItem> GetFiles();
	QSize GetPlatformSize();

private slots:
	void OnActionChanged(uint32 id);
	void UpdateOkButtonState();

private:
	static const eAction DEFAULT_ACTION = ACTION_IGNORE;
	static const uint32 CONTROL_WIDGET_ID = (uint32)-1;

	Ui::ImportDialog *ui;

	eImportType importType;
	ImportDialogFileWidget* controlWidget;
	Vector<ImportDialogFileWidget*> fileWidgets;
	HierarchyTreeNode::HIERARCHYTREENODEID platformId;
	FilePath platformPath;

	void ClearCurrentList();
	bool CheckAggregatorsSize();
	void InitWithFileList(const Vector<String>& fileList);
	void InitScreensImport();
	void InitPlatformImport();

	Vector<String> GetDirectoryContent(const FilePath& path, bool getFiles = false,
									   const String& fileMask = "",
									   bool removeExtension = true);
	Vector<String> GetChildrenNames(HierarchyTreeNode* node);
	Vector<String> GetStringNotInList(const Vector<String>& strings,
									  const Vector<String>& listToSearchIn);
};

#endif /* defined(__UIEDITOR__IMPORTDIALOG__) */
