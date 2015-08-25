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


#ifndef __QUICKED_PROPERTIES_WIDGET_H__
#define __QUICKED_PROPERTIES_WIDGET_H__

#include <QDockWidget>
#include "ui_PropertiesWidget.h"

class SharedData;

class ControlNode;
class StyleSheetNode;

class PropertiesWidget : public QDockWidget, public Ui::PropertiesWidget
{
    Q_OBJECT
public:
    PropertiesWidget(QWidget *parent = NULL);

public slots:
    void OnDocumentChanged(SharedData *sharedData);
    void OnDataChanged(const QByteArray &role);

    void OnAddComponent(QAction *action);
    void OnAddStyleProperty(QAction *action);
    void OnAddStyleSelector();
    void OnRemove();
    
    void OnSelectionChanged(const QItemSelection &selected,
                            const QItemSelection &deselected);

private:
    QAction *CreateAddComponentAction();
    QAction *CreateAddStyleSelectorAction();
    QAction *CreateAddStylePropertyAction();
    QAction *CreateRemoveAction();
    QAction *CreateSeparator();

    ControlNode *GetSelectedControlNode() const;
    StyleSheetNode *GetSelectedStyleSheetNode() const;
    
    void UpdateSelection();
    void UpdateActions();
    
private:
    SharedData *sharedData;
    QAction *addComponentAction;
    QAction *addStylePropertyAction;
    QAction *addStyleSelectorAction;
    QAction *removeAction;
};

#endif //__QUICKED_PROPERTIES_WIDGET_H__
