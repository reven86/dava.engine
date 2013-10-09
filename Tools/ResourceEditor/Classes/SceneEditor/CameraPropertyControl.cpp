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



#include "CameraPropertyControl.h"


CameraPropertyControl::CameraPropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
}

CameraPropertyControl::~CameraPropertyControl()
{

}

void CameraPropertyControl::ReadFrom(Entity * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

    Camera *camera = GetCamera(sceneNode);
	DVASSERT(camera);

    propertyList->AddSection("property.camera.camera", GetHeaderState("property.camera.camera", true));
        
    propertyList->AddFloatProperty("property.camera.fov", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.camera.znear", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.camera.zfar", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddBoolProperty("property.camera.isortho", PropertyList::PROPERTY_IS_EDITABLE);
    
    propertyList->AddFloatProperty("property.camera.position.x", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.camera.position.y", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.camera.position.z", PropertyList::PROPERTY_IS_EDITABLE);
    
    propertyList->AddFloatProperty("property.camera.target.x", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.camera.target.y", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.camera.target.z", PropertyList::PROPERTY_IS_EDITABLE);
    
    propertyList->SetFloatPropertyValue("property.camera.fov", camera->GetFOV());
    propertyList->SetFloatPropertyValue("property.camera.znear", camera->GetZNear());
    propertyList->SetFloatPropertyValue("property.camera.zfar", camera->GetZFar());
    propertyList->SetBoolPropertyValue("property.camera.isortho", camera->GetIsOrtho());
    
    Vector3 pos = camera->GetPosition();
    propertyList->SetFloatPropertyValue("property.camera.position.x", pos.x);
    propertyList->SetFloatPropertyValue("property.camera.position.y", pos.y);
    propertyList->SetFloatPropertyValue("property.camera.position.z", pos.z);
    
    Vector3 target = camera->GetTarget();
    propertyList->SetFloatPropertyValue("property.camera.target.x", target.x);
    propertyList->SetFloatPropertyValue("property.camera.target.y", target.y);
    propertyList->SetFloatPropertyValue("property.camera.target.z", target.z);
}

void CameraPropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    Camera *camera = GetCamera(currentSceneNode);
    if(     "property.camera.fov" == forKey 
       ||   "property.camera.znear" == forKey 
       ||   "property.camera.zfar" == forKey)
    {
        camera->SetupPerspective(
                      propertyList->GetFloatPropertyValue("property.camera.fov"),
                      camera->GetAspect(),
                      propertyList->GetFloatPropertyValue("property.camera.znear"),
                      propertyList->GetFloatPropertyValue("property.camera.zfar"));
    }
    else if(    "property.camera.position.x" == forKey 
            ||  "property.camera.position.y" == forKey 
            ||  "property.camera.position.z" == forKey)
    {
        camera->SetPosition(Vector3(
                                    propertyList->GetFloatPropertyValue("property.camera.position.x"),
                                    propertyList->GetFloatPropertyValue("property.camera.position.y"),
                                    propertyList->GetFloatPropertyValue("property.camera.position.z")));
    }
    else if(    "property.camera.target.x" == forKey 
            ||  "property.camera.target.y" == forKey 
            ||  "property.camera.target.z" == forKey)
    {
        camera->SetTarget(Vector3(
                                  propertyList->GetFloatPropertyValue("property.camera.target.x"),
                                  propertyList->GetFloatPropertyValue("property.camera.target.y"),
                                  propertyList->GetFloatPropertyValue("property.camera.target.z")));
    }

    NodesPropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}

void CameraPropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if("property.camera.isortho" == forKey)
    {
        Camera *camera = GetCamera(currentSceneNode);
        camera->SetupPerspective(
                      propertyList->GetFloatPropertyValue("property.camera.fov"),
                      320.0f / 480.0f,
                      propertyList->GetFloatPropertyValue("property.camera.znear"),
                      propertyList->GetFloatPropertyValue("property.camera.zfar"));
    }

    NodesPropertyControl::OnBoolPropertyChanged(forList, forKey, newValue);
}




