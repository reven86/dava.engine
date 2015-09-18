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


#ifndef __QUICKED_QT_MODEL_PACKAGE_COMMAND_EXECUTOR_H__
#define __QUICKED_QT_MODEL_PACKAGE_COMMAND_EXECUTOR_H__

#include "Base/BaseObject.h"
#include "Base/Result.h"

#include <QString>

class Document;
class PackageBaseNode;
class QUndoStack;
class QUndoCommand;

class ControlNode;
class StyleSheetNode;
class StyleSheetsNode;
class PackageControlsNode;
class PackageNode;
class AbstractProperty;
class ControlsContainerNode;
class ComponentPropertiesSection;

class QtModelPackageCommandExecutor : public DAVA::BaseObject
{
public:
    QtModelPackageCommandExecutor(Document *_document);
    
private:
    virtual ~QtModelPackageCommandExecutor();
    
public:
    void AddImportedPackagesIntoPackage(const DAVA::Vector<DAVA::FilePath> packagePaths, PackageNode *package);
    void RemoveImportedPackagesFromPackage(const DAVA::Vector<PackageNode*> &importedPackage, PackageNode *package);

public:
    void ChangeProperty(ControlNode* _node, const DAVA::Vector<std::pair<AbstractProperty*, DAVA::VariantType>>& properties, size_t hash = 0);
    void ChangeProperty(ControlNode* node, AbstractProperty* property, const DAVA::VariantType& value, size_t hash = 0);
    void ResetProperty(ControlNode *node, AbstractProperty *property);

    void AddComponent(ControlNode *node, DAVA::uint32 componentType);
    void RemoveComponent(ControlNode *node, DAVA::uint32 componentType, DAVA::uint32 componentIndex);

    void ChangeProperty(StyleSheetNode *node, AbstractProperty *property, const DAVA::VariantType &value);

    void AddStyleProperty(StyleSheetNode *node, DAVA::uint32 propertyIndex);
    void RemoveStyleProperty(StyleSheetNode *node, DAVA::uint32 propertyIndex);

    void AddStyleSelector(StyleSheetNode *node);
    void RemoveStyleSelector(StyleSheetNode *node, DAVA::int32 selectorIndex);

    DAVA::ResultList InsertControl(ControlNode *control, ControlsContainerNode *dest, DAVA::int32 destIndex);
    void InsertInstances(const DAVA::Vector<ControlNode*> &controls, ControlsContainerNode *dest, DAVA::int32 destIndex);
    void CopyControls(const DAVA::Vector<ControlNode*> &nodes, ControlsContainerNode *dest, DAVA::int32 destIndex);
    void MoveControls(const DAVA::Vector<ControlNode*> &nodes, ControlsContainerNode *dest, DAVA::int32 destIndex);

    DAVA::ResultList InsertStyle(StyleSheetNode *node, StyleSheetsNode *dest, DAVA::int32 destIndex);
    void CopyStyles(const DAVA::Vector<StyleSheetNode*> &nodes, StyleSheetsNode *dest, DAVA::int32 destIndex);
    void MoveStyles(const DAVA::Vector<StyleSheetNode*> &nodes, StyleSheetsNode *dest, DAVA::int32 destIndex);

    void Remove(const DAVA::Vector<ControlNode*> &controls, const DAVA::Vector<StyleSheetNode*> &styles);
    bool Paste(PackageNode *root, PackageBaseNode *dest, DAVA::int32 destIndex, const DAVA::String &data);

private:
    void AddImportedPackageIntoPackageImpl(PackageNode *importedPackage, PackageNode *package);
    void InsertControlImpl(ControlNode *control, ControlsContainerNode *dest, DAVA::int32 destIndex);
    void RemoveControlImpl(ControlNode *node);
    void AddComponentImpl(ControlNode *node, DAVA::int32 type, DAVA::int32 index, ComponentPropertiesSection *prototypeSection);
    void RemoveComponentImpl(ControlNode *node, ComponentPropertiesSection *section);
    bool IsNodeInHierarchy(const PackageBaseNode *node) const;
    
private:
    QUndoStack *GetUndoStack();
    void PushCommand(QUndoCommand *cmd);
    void BeginMacro(const QString &name);
    void EndMacro();
    
private:
    Document *document;
};

#endif // __QUICKED_QT_MODEL_PACKAGE_COMMAND_EXECUTOR_H__
