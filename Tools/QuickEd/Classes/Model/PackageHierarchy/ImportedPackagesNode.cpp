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


#include "ImportedPackagesNode.h"

#include "PackageControlsNode.h"
#include "../PackageSerializer.h"
#include "PackageRef.h"

using namespace DAVA;

ImportedPackagesNode::ImportedPackagesNode(PackageBaseNode *parent) : PackageBaseNode(parent)
{
}

ImportedPackagesNode::~ImportedPackagesNode()
{
    for (auto it = packageControlsNode.begin(); it != packageControlsNode.end(); ++it)
        (*it)->Release();
    packageControlsNode.clear();
}

void ImportedPackagesNode::Add(PackageControlsNode *node)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    packageControlsNode.push_back(SafeRetain(node));
}

void ImportedPackagesNode::InsertAtIndex(DAVA::int32 index, PackageControlsNode *node)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    
    packageControlsNode.insert(packageControlsNode.begin() + index, SafeRetain(node));
}

void ImportedPackagesNode::Remove(PackageControlsNode *node)
{
    auto it = find(packageControlsNode.begin(), packageControlsNode.end(), node);
    if (it != packageControlsNode.end())
    {
        DVASSERT(node->GetParent() == this);
        node->SetParent(NULL);
        
        packageControlsNode.erase(it);
        SafeRelease(node);
    }
    else
    {
        DVASSERT(false);
    }
}

int ImportedPackagesNode::GetCount() const
{
    return (int) packageControlsNode.size();
}

PackageControlsNode *ImportedPackagesNode::Get(int index) const
{
    return packageControlsNode[index];
}

String ImportedPackagesNode::GetName() const
{
    return "Imported Packages";
}

PackageControlsNode *ImportedPackagesNode::FindPackageControlsNodeByName(const DAVA::String &name) const
{
    for (PackageControlsNode *node : packageControlsNode)
    {
        if (node->GetPackageRef()->GetName() == name)
            return node;
    }
    return nullptr;
}

int ImportedPackagesNode::GetFlags() const
{
    return FLAG_READ_ONLY;
}

bool ImportedPackagesNode::CanInsertImportedPackage() const
{
    return true;
}

void ImportedPackagesNode::Serialize(PackageSerializer *serializer) const
{
    serializer->BeginArray("ImportedPackages");
    
    for (PackageControlsNode *controlsNode : packageControlsNode)
        serializer->PutValue(controlsNode->GetPackageRef()->GetPath().GetFrameworkPath());
    
    serializer->EndArray();
}

void ImportedPackagesNode::Serialize(PackageSerializer *serializer, const DAVA::Set<PackageRef*> &packageRefs) const
{
    serializer->BeginArray("ImportedPackages");
    
    for (PackageControlsNode *controlsNode : packageControlsNode)
    {
        PackageRef *ref = controlsNode->GetPackageRef();
        if (packageRefs.find(ref) != packageRefs.end())
            serializer->PutValue(ref->GetPath().GetFrameworkPath());
    }
    
    serializer->EndArray();
}

bool ImportedPackagesNode::IsReadOnly() const
{
    return true;
}
