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


#ifndef __DAVAENGINE_H__
#define __DAVAENGINE_H__

#include "DAVAVersion.h"
#include "DAVAConfig.h"
#include "Base/Platform.h"
#include "Debug/Stats.h"
#include "MemoryManager/MemoryProfiler.h"

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Utils/StringFormat.h"

#include "DLC/DLC.h"
#include "DLC/Patcher/PatchFile.h"
#include "DLC/Downloader/DownloadManager.h"

#include "FileSystem/Logger.h"
#include "Platform/SystemTimer.h"
#include "Platform/DateTime.h"
#include "Platform/DeviceInfo.h"

// system stuff
#include "Utils/Utils.h"
#include "Utils/UTF8Utils.h"
#include "Utils/MD5.h"
#include "Base/Message.h"
#include "Base/BaseObject.h"
#include "Debug/DVAssert.h"
#include "Base/Singleton.h"
#include "Utils/StringFormat.h"
#include "UI/ScrollHelper.h"
#include "Debug/Replay.h"
#include "Utils/Random.h"

#include "Base/ObjectFactory.h"
#include "Base/FixedSizePoolAllocator.h"
#include "Base/HashMap.h"

// ptrs
#include "Base/RefPtr.h"
#include "Base/ScopedPtr.h"

// threads
#include "Concurrency/Concurrency.h"

// Accelerometer
#include "Input/Accelerometer.h"

#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"
#include "Input/GamepadDevice.h"

// Localization
#include "FileSystem/LocalizationSystem.h"

// Image formats stuff (PNG & JPG & other formats)
#include "Render/Image/LibPngHelper.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/LibDdsHelper.h"

// Files & Serialization
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/FileList.h"
#include "FileSystem/VariantType.h"
#include "FileSystem/KeyedArchiver.h"
#include "FileSystem/KeyedUnarchiver.h"
#include "FileSystem/KeyedArchive.h"

#include "FileSystem/XMLParser.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlEmitter.h"
#include "FileSystem/Parser.h"
#include "FileSystem/FilePath.h"


// Collisions
#include "Collision/Collisions.h"

// Animation manager
#include "Animation/Interpolation.h"
#include "Animation/AnimatedObject.h"
#include "Animation/Animation.h"
#include "Animation/AnimationManager.h"
#include "Animation/LinearAnimation.h"
#include "Animation/BezierSplineAnimation.h"


// 2D Graphics
#include "Render/2D/Sprite.h"
#include "Render/GPUFamilyDescriptor.h"
#include "Render/TextureDescriptor.h"
#include "Render/Texture.h"
#include "Render/Shader.h"
#include "Render/ShaderCache.h"
#include "Core/DisplayMode.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Render/Cursor.h"
#include "Render/MipmapReplacer.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"

// Fonts
#include "Render/2D/Font.h"
#include "Render/2D/GraphicsFont.h"
#include "Render/2D/FTFont.h"
#include "Render/2D/FontManager.h"
#include "Render/2D/TextBlock.h"
#include "Render/2D/DFFont.h"

// UI
#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/UIPackage.h"
#include "UI/UIPackagesCache.h"
#include "UI/UIPackageLoader.h"
#include "UI/DefaultUIPackageBuilder.h"
#include "UI/UIEvent.h"
#include "UI/UIButton.h"
#include "UI/UIStaticText.h"
#include "UI/UIControlBackground.h"
#include "UI/UIScreen.h"
#include "UI/UIList.h"
#include "UI/UIListCell.h"
#include "UI/UIJoypad.h"
#include "UI/UITextField.h"
#include "UI/UISlider.h"
#include "UI/UIScrollBar.h"
#include "UI/UIJoypad.h"
#include "UI/UI3DView.h"
#include "UI/UIHierarchy.h"
#include "UI/UIHierarchyCell.h"
#include "UI/UIFileSystemDialog.h"
#include "UI/UIWebView.h"
#include "UI/UIScrollView.h"
#include "UI/UI3DView.h"
#include "UI/UISpinner.h"
#include "UI/VectorSpinnerAdapter.h"
#include "UI/UISwitch.h"
#include "UI/UIParticles.h"
#include "UI/UIMovieView.h"

#include "UI/UIYamlLoader.h"

#include "UI/UIScreenTransition.h"
#include "UI/UIMoveInTransition.h"
#include "UI/UIFadeTransition.h"
#include "UI/UIHoleTransition.h"

#include "UI/UIScreenManager.h"

#include "UI/TheoraPlayer.h"
#include "UI/UIAggregatorControl.h"

#include "UI/UIScrollViewContainer.h"
#include "UI/UIControlHelpers.h"

// Game object manager / 2D Scene
#include "Scene2D/GameObject.h"
#include "Scene2D/GameObjectManager.h"
#include "Collision/CollisionObject2.h"

// Sound & Music
#include "Sound/SoundEvent.h"
#include "Sound/SoundSystem.h"

// Particle System
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLayer.h"
#include "Particles/Particle.h"

// 3D core classes
#include "Scene3D/SceneFile.h"
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/SceneFile/SerializationContext.h"

#include "Render/3D/StaticMesh.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/3D/EdgeAdjacency.h"
#include "Render/3D/MeshUtils.h"

// Material compiler
#include "Render/Material/MaterialCompiler.h"
#include "Render/Material/MaterialGraph.h"
#include "Render/Material/MaterialGraphNode.h"
#include "Render/Material/RenderTechnique.h"
#include "Render/Material/NMaterialNames.h"

// 3D scene management
#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SpriteNode.h"
#include "Scene3D/MeshInstanceNode.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/Heightmap.h"
#include "Render/Highlevel/Light.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/Highlevel/SkinnedMesh.h"
#include "Render/Highlevel/SpriteObject.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/RenderFastNames.h"
#include "Render/Highlevel/LandscapeChunk.h"
#include "Render/Highlevel/SkyboxRenderObject.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Render/Highlevel/Vegetation/TextureSheet.h"
#include "Render/Highlevel/Vegetation/VegetationRenderObject.h"

#include "Scene3D/AnimationData.h"
#include "Scene3D/ShadowVolumeNode.h"
#include "Scene3D/LodNode.h"
#include "Scene3D/ImposterNode.h"
#include "Scene3D/ParticleEmitterNode.h"
#include "Scene3D/ParticleEffectNode.h"
#include "Scene3D/SwitchNode.h"
#include "Scene3D/UserNode.h"
#include "Scene3D/Systems/LodSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/BillboardNode.h"
#include "Scene3D/BoneNode.h"
#include "Scene3D/SkeletonNode.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/SpeedTreeUpdateSystem.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Scene3D/Systems/FoliageSystem.h"
#include "Scene3D/Systems/ParticleEffectSystem.h"

//Components
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/BulletComponent.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/UpdatableComponent.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Components/UserComponent.h"
#include "Scene3D/Components/SoundComponent.h"
#include "Scene3D/Components/ActionComponent.h"
#include "Scene3D/Components/StaticOcclusionComponent.h"
#include "Scene3D/Components/QualitySettingsComponent.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Scene3D/Components/WindComponent.h"
#include "Scene3D/Components/WaveComponent.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"
#include "Scene3D/Components/Controller/RotationControllerComponent.h"
#include "Scene3D/Components/Controller/SnapToLandscapeControllerComponent.h"

// Application core 
#include "Core/Core.h"
#include "Core/ApplicationCore.h"



// Notifications
#include "Notification/LocalNotification.h"
#include "Notification/LocalNotificationText.h"
#include "Notification/LocalNotificationProgress.h"
#include "Notification/LocalNotificationDelayed.h"
#include "Notification/LocalNotificationController.h"

#endif // __DAVAENGINE_H__

