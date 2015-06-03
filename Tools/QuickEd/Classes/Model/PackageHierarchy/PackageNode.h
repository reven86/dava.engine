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


#ifndef __UI_EDITOR_PACKAGE_NODE_H__
#define __UI_EDITOR_PACKAGE_NODE_H__

#include "PackageBaseNode.h"

#include "FileSystem/VariantType.h"

class ImportedPackagesNode;
class PackageControlsNode;
class ControlsContainerNode;
class PackageSerializer;
class ControlNode;
class PackageRef;
class PackageListener;
class AbstractProperty;
class ComponentPropertiesSection;

class PackageNode : public PackageBaseNode
{
public:
    PackageNode(PackageRef *_packageRef);
private:
    virtual ~PackageNode();
    
public:
    virtual int GetCount() const override;
    virtual PackageBaseNode *Get(int index) const override;

    virtual DAVA::String GetName() const override;
    int GetFlags() const override;
    
    virtual PackageRef *GetPackageRef() const override;
    
    ImportedPackagesNode *GetImportedPackagesNode() const;
    PackageControlsNode *GetPackageControlsNode() const;

    PackageControlsNode *FindImportedPackage(const DAVA::FilePath &path);

    void AddListener(PackageListener *listener);
    void RemoveListener(PackageListener *listener);
    
    void SetControlProperty(ControlNode *node, AbstractProperty *property, const DAVA::VariantType &newValue);
    void SetControlDefaultProperty(ControlNode *node, AbstractProperty *property, const DAVA::VariantType &newValue);
    void ResetControlProperty(ControlNode *node, AbstractProperty *property);
    void RefreshProperty(ControlNode *node, AbstractProperty *property);

    void AddComponent(ControlNode *node, ComponentPropertiesSection *section);
    void RemoveComponent(ControlNode *node, ComponentPropertiesSection *section);
    
    void InsertControl(ControlNode *node, ControlsContainerNode *dest, DAVA::int32 index);
    void RemoveControl(ControlNode *node, ControlsContainerNode *from);

    void InsertImportedPackage(PackageControlsNode *node, DAVA::int32 index);
    void RemoveImportedPackage(PackageControlsNode *node);

    void Serialize(PackageSerializer *serializer) const;
    void Serialize(PackageSerializer *serializer, const DAVA::Vector<ControlNode*> &nodes) const;
    
    
private:
    void CollectPackages(DAVA::Set<PackageRef*> &packageRefs, ControlNode *node) const;
    void RefreshPropertiesInInstances(ControlNode *node, AbstractProperty *property);

protected:
    virtual bool IsReadOnly() const override;
    
private:
    PackageRef *packageRef;
    
    ImportedPackagesNode *importedPackagesNode;
    PackageControlsNode *packageControlsNode;
    DAVA::Vector<PackageListener*> listeners;
};

#endif // __UI_EDITOR_PACKAGE_NODE_H__
