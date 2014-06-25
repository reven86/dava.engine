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


#include "Classes/UI/controllist.h"
#include "HierarchyTreeController.h"

ControlMimeData::ControlMimeData(const QString& controlName)
{
	this->controlName = controlName;
}

ControlMimeData::~ControlMimeData()
{

}

bool ControlMimeData::hasFormat( const QString & mimeType ) const
{
    // We need this to drop controls from Library to Hierarchy tree as native.
    if(mimeType == "application/x-qabstractitemmodeldatalist")
    {
        return true;
    }
    return QMimeData::hasFormat(mimeType);
}

ControlList::ControlList(QWidget *parent) :
    QTreeWidget(parent)
{
    // Recover the original font size - it is lost by some reason.
    QFont currentFont = this->font();
    currentFont.setPointSize(13);
    this->setFont(currentFont);
}

QStringList ControlList::mimeTypes() const
{
	QStringList list = QTreeWidget::mimeTypes();
	return list;
}

QMimeData* ControlList::mimeData(const QList<QTreeWidgetItem*> items) const
{
	Q_ASSERT(items.size() == 1);
	if (items.size() != 1)
		return NULL;
	
	QTreeWidgetItem* item = items[0];
	ControlMimeData* data = new ControlMimeData(item->text(0));
	
	HierarchyTreeController::Instance()->ResetSelectedControl();
	
	return data;
}

bool ControlList::dropMimeData(QTreeWidgetItem *parent, int index,
						  const QMimeData *data, Qt::DropAction action)
{
	bool bRes = QTreeWidget::dropMimeData(parent, index, data, action);
	return bRes;
}

Qt::DropActions ControlList::supportedDropActions() const
{
	Qt::DropActions	actions = QTreeWidget::supportedDropActions();
	return actions;
}
