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


#ifndef __SCENE_VALIDATOR_H__
#define __SCENE_VALIDATOR_H__

#include "DAVAEngine.h"

class SceneValidator : public DAVA::Singleton<SceneValidator>
{
public:
    /*
     \brief Function to validate Scene errors and Displays errors log at Errors Dialog
     \param[in] scene scene for validation
     \returns true if errors were found
	 */
    bool ValidateSceneAndShowErrors(DAVA::Scene* scene, const DAVA::FilePath& scenePath);

    /*
     \brief Function to validate Scene errors
     \param[in] scene scene for validation
     \param[out] errorsLog set for validation errors
	 */
    void ValidateScene(DAVA::Scene* scene, const DAVA::FilePath& scenePath, DAVA::Set<DAVA::String>& errorsLog);

    /*
     \brief Function to find Scales in models transformations
     \param[in] scene scene for validation
     \param[out] errorsLog set for validation errors
	 */
    void ValidateScales(DAVA::Scene* scene, DAVA::Set<DAVA::String>& errorsLog);

    /*
     \brief Function to validate Texture errors

     Displays errors log at Errors Dialog

     \param[in] texture texture for validation
	 */
    void ValidateTextureAndShowErrors(DAVA::Texture* texture, const DAVA::String& validatedObjectName);

    /*
     \brief Function to validate Texture errors
     \param[in] texture texture for validation
     \param[out] errorsLog set for validation errors
	 */

    void ValidateTexture(DAVA::Texture* texture, const DAVA::String& validatedObjectName, DAVA::Set<DAVA::String>& errorsLog);

    /*
     \brief Function to validate LandscapeNode errors
     \param[in] landscape landscape for validation
     \param[out] errorsLog set for validation errors
	 */
    void ValidateLandscape(DAVA::Landscape* landscape, DAVA::Set<DAVA::String>& errorsLog);

    /*
     \brief Function to validate Entity errors
     \param[in] sceneNode sceneNode for validation
     \param[out] errorsLog set for validation errors
	 */
    void ValidateSceneNode(DAVA::Entity* sceneNode, DAVA::Set<DAVA::String>& errorsLog);

    /*
     \brief Function to validate Materials errors
     \param[in] scene that has materials for validation
     \param[out] errorsLog set for validation errors
	 */
    void ValidateMaterials(DAVA::Scene* scene, DAVA::Set<DAVA::String>& errorsLog);

    /*
     \brief Function sets 3d folder path for checking texture pathnames
     \param[in] pathname path to DataSource/3d folder
     \return old path for checking
	 */

    void ValidateNodeCustomProperties(DAVA::Entity* sceneNode);

    DAVA::FilePath SetPathForChecking(const DAVA::FilePath& pathname);

    void EnumerateNodes(DAVA::Scene* scene);

    static bool IsTextureChanged(const DAVA::TextureDescriptor* descriptor, DAVA::eGPUFamily forGPU);
    static bool IsTextureChanged(const DAVA::FilePath& texturePathname, DAVA::eGPUFamily forGPU);

    bool ValidateTexturePathname(const DAVA::FilePath& pathForValidation, DAVA::Set<DAVA::String>& errorsLog);
    bool ValidateHeightmapPathname(const DAVA::FilePath& pathForValidation, DAVA::Set<DAVA::String>& errorsLog);

    bool IsPathCorrectForProject(const DAVA::FilePath& pathname);

    DAVA_DEPRECATED(static void FindSwitchesWithDifferentLODs(DAVA::Entity* entity, DAVA::Set<DAVA::FastName>& names));
    DAVA_DEPRECATED(static bool IsEntityHasDifferentLODsCount(DAVA::Entity* entity));
    DAVA_DEPRECATED(static bool IsObjectHasDifferentLODsCount(DAVA::RenderObject* renderObject));

    static void ExtractEmptyRenderObjectsAndShowErrors(DAVA::Entity* entity);
    static void ExtractEmptyRenderObjects(DAVA::Entity* entity, DAVA::Set<DAVA::String>& errorsLog);

protected:
    void ValidateRenderComponent(DAVA::Entity* ownerNode, DAVA::Set<DAVA::String>& errorsLog);
    void ValidateRenderBatch(DAVA::Entity* ownerNode, DAVA::RenderBatch* renderBatch, DAVA::Set<DAVA::String>& errorsLog);

    void ValidateParticleEffectComponent(DAVA::Entity* ownerNode, DAVA::Set<DAVA::String>& errorsLog) const;
    void ValidateParticleEmitter(DAVA::ParticleEmitterInstance* emitter, DAVA::Set<DAVA::String>& errorsLog, DAVA::Entity* owner) const;

    void ValidateLandscapeTexture(DAVA::Landscape* landscape, const DAVA::FastName& texLevel, DAVA::Set<DAVA::String>& errorsLog);
    void ValidateCustomColorsTexture(DAVA::Entity* landscapeEntity, DAVA::Set<DAVA::String>& errorsLog);

    void FixIdentityTransform(DAVA::Entity* ownerNode,
                              DAVA::Set<DAVA::String>& errorsLog,
                              const DAVA::String& errorMessage);

    bool ValidateColor(DAVA::Color& color);

    DAVA::int32 EnumerateSceneNodes(DAVA::Entity* node);

    void ValidateScalesInternal(DAVA::Entity* sceneNode, DAVA::Set<DAVA::String>& errorsLog);

    bool ValidatePathname(const DAVA::FilePath& pathForValidation, const DAVA::String& validatedObjectName);

    bool NodeRemovingDisabled(DAVA::Entity* node);

    bool WasTextureChanged(DAVA::Texture* texture, DAVA::eGPUFamily forGPU);

    bool IsTextureDescriptorPath(const DAVA::FilePath& path);

    bool IsFBOTexture(DAVA::Texture* texture);

    DAVA::VariantType* GetCustomPropertyFromParentsTree(DAVA::Entity* ownerNode, const DAVA::String& key);

    DAVA::Set<DAVA::Entity*> emptyNodesForDeletion;
    DAVA::Set<DAVA::String> errorMessages;

    DAVA::FilePath pathForChecking;
    DAVA::String sceneName;
};

#endif // __SCENE_VALIDATOR_H__