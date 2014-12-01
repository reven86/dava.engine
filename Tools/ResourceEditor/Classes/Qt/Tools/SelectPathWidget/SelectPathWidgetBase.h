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



#ifndef __RESOURCEEDITORQT__SELECTPATHWIDGET__
#define __RESOURCEEDITORQT__SELECTPATHWIDGET__

#include <QWidget>
#include <QMimeData>
#include <qlineedit.h>
#include <qtoolbutton.h>

#include "DAVAEngine.h"
class SceneEditor2;

class SelectPathWidgetBase: public QLineEdit
{
	Q_OBJECT

public:
	explicit SelectPathWidgetBase( QWidget* parent = 0, bool checkForProjectPath = false,DAVA::String openDialoDefualtPath = "", DAVA::String relativPath = "",
								  DAVA::String openFileDialogTitle = "Open File", DAVA::String fileFormatDescriotion = "*.*");
	
	virtual ~SelectPathWidgetBase();
	
	void setText(const DAVA::String &);

	DAVA::String getText();

	virtual void EraseWidget();

	void SetAllowedFormatsList(const DAVA::List<DAVA::String>& _allowedFormatsList)
	{
		allowedFormatsList = _allowedFormatsList;
	}
    
    bool IsOpenButtonVisible() const;
    
    void SetOpenButtonVisible(bool value);
    
    bool IsClearButtonVisible() const;
    
    void SetClearButtonVisible(bool value);
    
    DAVA::String GetOpenDialogDefaultPath() const;
    
    void SetOpenDialogDefaultPath(const DAVA::FilePath& path);
    
    DAVA::String GetFileFormatFilter() const;
    
    void SetFileFormatFilter(const DAVA::String& filter);
	
public slots:

	void setText(const QString&);
		
	void acceptEditing();

    void setVisible(bool);
    
signals:
	
	void PathSelected(DAVA::String selectedFile);

protected:

	void resizeEvent(QResizeEvent *);
	
	virtual void Init(DAVA::String& _openDialogDefualtPath, DAVA::String& _relativPath,DAVA::String _openFileDialogTitle, DAVA::String _fileFormatDescriotion);
	
	virtual void HandlePathSelected(DAVA::String name);
		
	DAVA::String ConvertToRelativPath(const DAVA::String& path);
	
	QToolButton* CreateToolButton(const DAVA::String& iconPath);

	void dragEnterEvent(QDragEnterEvent* event);
	void dragMoveEvent(QDragMoveEvent* event);
	void dropEvent(QDropEvent * event);
	
	DAVA::FilePath			relativePath;
	
	DAVA::String			openDialogDefaultPath;
	
	DAVA::String			fileFormatFilter;// like "Scene File (*.sc2)"

	DAVA::List<DAVA::String> allowedFormatsList;
	
	DAVA::String			openFileDialogTitle;
		
	QMimeData				mimeData;
    
    bool                    checkForProjectPath;

protected slots:

	void EraseClicked();

	void OpenClicked();
	
private:

	QToolButton *clearButton;
	QToolButton *openButton;
};

#endif /* defined(__RESOURCEEDITORQT__SELECTPATHWIDGET__) */