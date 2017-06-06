set(NAME_MODULE Physics )

set(MODULE_TYPE STATIC)#INLINE STATIC DYNAMIC 

set(HPP_FILES_RECURSE Sources/*.h Sources/*.hpp)
set(CPP_FILES_RECURSE Sources/*.cpp)

set(PhysicsRootDir "${CMAKE_CURRENT_SOURCE_DIR}")

set(INCLUDES "${PhysicsRootDir}/Sources" "${PhysicsRootDir}/Libs/include/")
set(INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR} "${PhysicsRootDir}/Libs/include/physx" "${PhysicsRootDir}/Libs/include/PxShared")
set( MODULE_INITIALIZATION_CODE Sources/Physics/PhysicsModule.h )

set (LibsPath "${PhysicsRootDir}/Libs")

set(WIN32_STATIC_DEBUG_LIBS "${LibsPath}/Win32/x86/LowLevelDEBUG.lib"
                                                              "${LibsPath}/Win32/x86/LowLevelAABBDEBUG.lib"
                                                              "${LibsPath}/Win32/x86/LowLevelClothDEBUG.lib"
                                                              "${LibsPath}/Win32/x86/LowLevelDynamicsDEBUG.lib"
                                                              "${LibsPath}/Win32/x86/LowLevelParticlesDEBUG.lib"
                                                              "${LibsPath}/Win32/x86/PhysX3DEBUG_x86.lib"
                                                              "${LibsPath}/Win32/x86/PhysX3CharacterKinematicDEBUG_x86.lib"
                                                              "${LibsPath}/Win32/x86/PhysX3CommonDEBUG_x86.lib"
                                                              "${LibsPath}/Win32/x86/PhysX3CookingDEBUG_x86.lib"
                                                              "${LibsPath}/Win32/x86/PhysX3ExtensionsDEBUG.lib"
                                                              "${LibsPath}/Win32/x86/PhysX3VehicleDEBUG.lib"
                                                              "${LibsPath}/Win32/x86/PsFastXmlDEBUG_x86.lib"
                                                              "${LibsPath}/Win32/x86/PxFoundationDEBUG_x86.lib"
                                                              "${LibsPath}/Win32/x86/PxPvdSDKDEBUG_x86.lib"
                                                              "${LibsPath}/Win32/x86/PxTaskDEBUG_x86.lib"
                                                              "${LibsPath}/Win32/x86/SceneQueryDEBUG.lib"
                                                              "${LibsPath}/Win32/x86/SimulationControllerDEBUG.lib")

set(WIN32_DYNAMIC_DEBUG_LIBS "${LibsPath}/Win32/x86/nvToolsExt32_1.dll"
                                                                  "${LibsPath}/Win32/x86/PhysX3DEBUG_x86.dll"
                                                                  "${LibsPath}/Win32/x86/PhysX3CharacterKinematicDEBUG_x86.dll"
                                                                  "${LibsPath}/Win32/x86/PhysX3CommonDEBUG_x86.dll"
                                                                  "${LibsPath}/Win32/x86/PhysX3CookingDEBUG_x86.dll"
                                                                  "${LibsPath}/Win32/x86/PxFoundationDEBUG_x86.dll"
                                                                  "${LibsPath}/Win32/x86/PxPvdSDKDEBUG_x86.dll")

set(WIN32_STATIC_CHECKED_LIBS "${LibsPath}/Win32/x86/LowLevelCHECKED.lib"
                                                                   "${LibsPath}/Win32/x86/LowLevelAABBCHECKED.lib"
                                                                   "${LibsPath}/Win32/x86/LowLevelClothCHECKED.lib"
                                                                   "${LibsPath}/Win32/x86/LowLevelDynamicsCHECKED.lib"
                                                                   "${LibsPath}/Win32/x86/LowLevelParticlesCHECKED.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3CHECKED_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3CharacterKinematicCHECKED_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3CommonCHECKED_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3CookingCHECKED_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3ExtensionsCHECKED.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3VehicleCHECKED.lib"
                                                                   "${LibsPath}/Win32/x86/PsFastXmlCHECKED_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PxFoundationCHECKED_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PxPvdSDKCHECKED_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PxTaskCHECKED_x86.lib"
                                                                   "${LibsPath}/Win32/x86/SceneQueryCHECKED.lib"
                                                                   "${LibsPath}/Win32/x86/SimulationControllerCHECKED.lib")

set(WIN32_DYNAMIC_CHECKED_LIBS "${LibsPath}/Win32/x86/nvToolsExt32_1.dll"
                                                                      "${LibsPath}/Win32/x86/PhysX3CHECKED_x86.dll"
                                                                      "${LibsPath}/Win32/x86/PhysX3CharacterKinematicCHECKED_x86.dll"
                                                                      "${LibsPath}/Win32/x86/PhysX3CommonCHECKED_x86.dll"
                                                                      "${LibsPath}/Win32/x86/PhysX3CookingCHECKED_x86.dll"
                                                                      "${LibsPath}/Win32/x86/PxFoundationCHECKED_x86.dll"
                                                                      "${LibsPath}/Win32/x86/PxPvdSDKCHECKED_x86.dll")

set(WIN32_STATIC_PROFILE_LIBS "${LibsPath}/Win32/x86/LowLevelPROFILE.lib"
                                                                   "${LibsPath}/Win32/x86/LowLevelAABBPROFILE.lib"
                                                                   "${LibsPath}/Win32/x86/LowLevelClothPROFILE.lib"
                                                                   "${LibsPath}/Win32/x86/LowLevelDynamicsPROFILE.lib"
                                                                   "${LibsPath}/Win32/x86/LowLevelParticlesPROFILE.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3PROFILE_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3CharacterKinematicPROFILE_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3CommonPROFILE_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3CookingPROFILE_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3ExtensionsPROFILE.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3VehiclePROFILE.lib"
                                                                   "${LibsPath}/Win32/x86/PsFastXmlPROFILE_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PxFoundationPROFILE_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PxPvdSDKPROFILE_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PxTaskPROFILE_x86.lib"
                                                                   "${LibsPath}/Win32/x86/SceneQueryPROFILE.lib"
                                                                   "${LibsPath}/Win32/x86/SimulationControllerPROFILE.lib")

set(WIN32_DYNAMIC_PROFILE_LIBS "${LibsPath}/Win32/x86/nvToolsExt32_1.dll"
                                                                      "${LibsPath}/Win32/x86/PhysX3PROFILE_x86.dll"
                                                                      "${LibsPath}/Win32/x86/PhysX3CharacterKinematicPROFILE_x86.dll"
                                                                      "${LibsPath}/Win32/x86/PhysX3CommonPROFILE_x86.dll"
                                                                      "${LibsPath}/Win32/x86/PhysX3CookingPROFILE_x86.dll"
                                                                      "${LibsPath}/Win32/x86/PxFoundationPROFILE_x86.dll"
                                                                      "${LibsPath}/Win32/x86/PxPvdSDKPROFILE_x86.dll")

set(WIN32_STATIC_RELEASE_LIBS "${LibsPath}/Win32/x86/LowLevel.lib"
                                                                   "${LibsPath}/Win32/x86/LowLevelAABB.lib"
                                                                   "${LibsPath}/Win32/x86/LowLevelCloth.lib"
                                                                   "${LibsPath}/Win32/x86/LowLevelDynamics.lib"
                                                                   "${LibsPath}/Win32/x86/LowLevelParticles.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3CharacterKinematic_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3Common_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3Cooking_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3Extensions.lib"
                                                                   "${LibsPath}/Win32/x86/PhysX3Vehicle.lib"
                                                                   "${LibsPath}/Win32/x86/PsFastXml_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PxFoundation_x86.lib"
                                                                   "${LibsPath}/Win32/x86/PxTask_x86.lib"
                                                                   "${LibsPath}/Win32/x86/SceneQuery.lib"
                                                                   "${LibsPath}/Win32/x86/SimulationController.lib")

set(WIN32_DYNAMIC_RELEASE_LIBS "${LibsPath}/Win32/x86/PhysX3_x86.dll"
                                                                      "${LibsPath}/Win32/x86/PhysX3CharacterKinematic_x86.dll"
                                                                      "${LibsPath}/Win32/x86/PhysX3Common_x86.dll"
                                                                      "${LibsPath}/Win32/x86/PhysX3Cooking_x86.dll"
                                                                      "${LibsPath}/Win32/x86/PxFoundation_x86.dll")

set(WIN64_STATIC_DEBUG_LIBS "${LibsPath}/Win32/x64/LowLevelDEBUG.lib"
                                                              "${LibsPath}/Win32/x64/LowLevelAABBDEBUG.lib"
                                                              "${LibsPath}/Win32/x64/LowLevelClothDEBUG.lib"
                                                              "${LibsPath}/Win32/x64/LowLevelDynamicsDEBUG.lib"
                                                              "${LibsPath}/Win32/x64/LowLevelParticlesDEBUG.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3DEBUG_x64.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3CharacterKinematicDEBUG_x64.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3CommonDEBUG_x64.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3CookingDEBUG_x64.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3ExtensionsDEBUG.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3VehicleDEBUG.lib"
                                                              "${LibsPath}/Win32/x64/PsFastXmlDEBUG_x64.lib"
                                                              "${LibsPath}/Win32/x64/PxFoundationDEBUG_x64.lib"
                                                              "${LibsPath}/Win32/x64/PxPvdSDKDEBUG_x64.lib"
                                                              "${LibsPath}/Win32/x64/PxTaskDEBUG_x64.lib"
                                                              "${LibsPath}/Win32/x64/SceneQueryDEBUG.lib"
                                                              "${LibsPath}/Win32/x64/SimulationControllerDEBUG.lib")

set(WIN64_DYNAMIC_DEBUG_LIBS "${LibsPath}/Win32/x64/nvToolsExt64_1.dll"
                                                                  "${LibsPath}/Win32/x64/PhysX3DEBUG_x64.dll"
                                                                  "${LibsPath}/Win32/x64/PhysX3CharacterKinematicDEBUG_x64.dll"
                                                                  "${LibsPath}/Win32/x64/PhysX3CommonDEBUG_x64.dll"
                                                                  "${LibsPath}/Win32/x64/PhysX3CookingDEBUG_x64.dll"
                                                                  "${LibsPath}/Win32/x64/PxFoundationDEBUG_x64.dll"
                                                                  "${LibsPath}/Win32/x64/PxPvdSDKDEBUG_x64.dll")

set(WIN64_STATIC_CHECKED_LIBS "${LibsPath}/Win32/x64/LowLevelCHECKED.lib"
                                                              "${LibsPath}/Win32/x64/LowLevelAABBCHECKED.lib"
                                                              "${LibsPath}/Win32/x64/LowLevelClothCHECKED.lib"
                                                              "${LibsPath}/Win32/x64/LowLevelDynamicsCHECKED.lib"
                                                              "${LibsPath}/Win32/x64/LowLevelParticlesCHECKED.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3CHECKED_x64.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3CharacterKinematicCHECKED_x64.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3CommonCHECKED_x64.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3CookingCHECKED_x64.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3ExtensionsCHECKED.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3VehicleCHECKED.lib"
                                                              "${LibsPath}/Win32/x64/PsFastXmlCHECKED_x64.lib"
                                                              "${LibsPath}/Win32/x64/PxFoundationCHECKED_x64.lib"
                                                              "${LibsPath}/Win32/x64/PxPvdSDKCHECKED_x64.lib"
                                                              "${LibsPath}/Win32/x64/PxTaskCHECKED_x64.lib"
                                                              "${LibsPath}/Win32/x64/SceneQueryCHECKED.lib"
                                                              "${LibsPath}/Win32/x64/SimulationControllerCHECKED.lib")

set(WIN64_DYNAMIC_CHECKED_LIBS "${LibsPath}/Win32/x64/nvToolsExt64_1.dll"
                                                                  "${LibsPath}/Win32/x64/PhysX3CHECKED_x64.dll"
                                                                  "${LibsPath}/Win32/x64/PhysX3CharacterKinematicCHECKED_x64.dll"
                                                                  "${LibsPath}/Win32/x64/PhysX3CommonCHECKED_x64.dll"
                                                                  "${LibsPath}/Win32/x64/PhysX3CookingCHECKED_x64.dll"
                                                                  "${LibsPath}/Win32/x64/PxFoundationCHECKED_x64.dll"
                                                                  "${LibsPath}/Win32/x64/PxPvdSDKCHECKED_x64.dll")

set(WIN64_STATIC_PROFILE_LIBS "${LibsPath}/Win32/x64/LowLevelPROFILE.lib"
                                                              "${LibsPath}/Win32/x64/LowLevelAABBPROFILE.lib"
                                                              "${LibsPath}/Win32/x64/LowLevelClothPROFILE.lib"
                                                              "${LibsPath}/Win32/x64/LowLevelDynamicsPROFILE.lib"
                                                              "${LibsPath}/Win32/x64/LowLevelParticlesPROFILE.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3PROFILE_x64.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3CharacterKinematicPROFILE_x64.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3CommonPROFILE_x64.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3CookingPROFILE_x64.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3ExtensionsPROFILE.lib"
                                                              "${LibsPath}/Win32/x64/PhysX3VehiclePROFILE.lib"
                                                              "${LibsPath}/Win32/x64/PsFastXmlPROFILE_x64.lib"
                                                              "${LibsPath}/Win32/x64/PxFoundationPROFILE_x64.lib"
                                                              "${LibsPath}/Win32/x64/PxPvdSDKPROFILE_x64.lib"
                                                              "${LibsPath}/Win32/x64/PxTaskPROFILE_x64.lib"
                                                              "${LibsPath}/Win32/x64/SceneQueryPROFILE.lib"
                                                              "${LibsPath}/Win32/x64/SimulationControllerPROFILE.lib")

set(WIN64_DYNAMIC_PROFILE_LIBS "${LibsPath}/Win32/x64/nvToolsExt64_1.dll"
                                                                  "${LibsPath}/Win32/x64/PhysX3PROFILE_x64.dll"
                                                                  "${LibsPath}/Win32/x64/PhysX3CharacterKinematicPROFILE_x64.dll"
                                                                  "${LibsPath}/Win32/x64/PhysX3CommonPROFILE_x64.dll"
                                                                  "${LibsPath}/Win32/x64/PhysX3CookingPROFILE_x64.dll"
                                                                  "${LibsPath}/Win32/x64/PxFoundationPROFILE_x64.dll"
                                                                  "${LibsPath}/Win32/x64/PxPvdSDKPROFILE_x64.dll")

set(WIN64_STATIC_RELEASE_LIBS "${LibsPath}/Win32/x64/LowLevel.lib"
                                                                  "${LibsPath}/Win32/x64/LowLevelAABB.lib"
                                                                  "${LibsPath}/Win32/x64/LowLevelCloth.lib"
                                                                  "${LibsPath}/Win32/x64/LowLevelDynamics.lib"
                                                                  "${LibsPath}/Win32/x64/LowLevelParticles.lib"
                                                                  "${LibsPath}/Win32/x64/PhysX3_x64.lib"
                                                                  "${LibsPath}/Win32/x64/PhysX3CharacterKinematic_x64.lib"
                                                                  "${LibsPath}/Win32/x64/PhysX3Common_x64.lib"
                                                                  "${LibsPath}/Win32/x64/PhysX3Cooking_x64.lib"
                                                                  "${LibsPath}/Win32/x64/PhysX3Extensions.lib"
                                                                  "${LibsPath}/Win32/x64/PhysX3Vehicle.lib"
                                                                  "${LibsPath}/Win32/x64/PsFastXml_x64.lib"
                                                                  "${LibsPath}/Win32/x64/PxFoundation_x64.lib"
                                                                  "${LibsPath}/Win32/x64/PxTask_x64.lib"
                                                                  "${LibsPath}/Win32/x64/SceneQuery.lib"
                                                                  "${LibsPath}/Win32/x64/SimulationController.lib")

set(WIN64_DYNAMIC_RELEASE_LIBS "${LibsPath}/Win32/x64/PhysX3_x64.dll"
                                                                      "${LibsPath}/Win32/x64/PhysX3CharacterKinematic_x64.dll"
                                                                      "${LibsPath}/Win32/x64/PhysX3Common_x64.dll"
                                                                      "${LibsPath}/Win32/x64/PhysX3Cooking_x64.dll"
                                                                      "${LibsPath}/Win32/x64/PxFoundation_x64.dll")

set(MACOS_STATIC_DEBUG_LIBS "${LibsPath}/MacOS/libLowLevelDEBUG.a"
                                                              "${LibsPath}/MacOS/libLowLevelAABBDEBUG.a"
                                                              "${LibsPath}/MacOS/libLowLevelClothDEBUG.a"
                                                              "${LibsPath}/MacOS/libLowLevelDynamicsDEBUG.a"
                                                              "${LibsPath}/MacOS/libLowLevelParticlesDEBUG.a"
                                                              "${LibsPath}/MacOS/libPhysX3DEBUG.a"
                                                              "${LibsPath}/MacOS/libPhysX3CharacterKinematicDEBUG.a"
                                                              "${LibsPath}/MacOS/libPhysX3CommonDEBUG.a"
                                                              "${LibsPath}/MacOS/libPhysX3CookingDEBUG.a"
                                                              "${LibsPath}/MacOS/libPhysX3ExtensionsDEBUG.a"
                                                              "${LibsPath}/MacOS/libPhysX3VehicleDEBUG.a"
                                                              "${LibsPath}/MacOS/libPsFastXmlDEBUG.a"
                                                              "${LibsPath}/MacOS/libPxFoundationDEBUG.a"
                                                              "${LibsPath}/MacOS/libPxPvdSDKDEBUG.a"
                                                              "${LibsPath}/MacOS/libPxTaskDEBUG.a"
                                                              "${LibsPath}/MacOS/libSceneQueryDEBUG.a"
                                                              "${LibsPath}/MacOS/libSimulationControllerDEBUG.a")

set(MACOS_STATIC_CHECKED_LIBS "${LibsPath}/MacOS/libLowLevelCHECKED.a"
                                                              "${LibsPath}/MacOS/libLowLevelAABBCHECKED.a"
                                                              "${LibsPath}/MacOS/libLowLevelClothCHECKED.a"
                                                              "${LibsPath}/MacOS/libLowLevelDynamicsCHECKED.a"
                                                              "${LibsPath}/MacOS/libLowLevelParticlesCHECKED.a"
                                                              "${LibsPath}/MacOS/libPhysX3CHECKED.a"
                                                              "${LibsPath}/MacOS/libPhysX3CharacterKinematicCHECKED.a"
                                                              "${LibsPath}/MacOS/libPhysX3CommonCHECKED.a"
                                                              "${LibsPath}/MacOS/libPhysX3CookingCHECKED.a"
                                                              "${LibsPath}/MacOS/libPhysX3ExtensionsCHECKED.a"
                                                              "${LibsPath}/MacOS/libPhysX3VehicleCHECKED.a"
                                                              "${LibsPath}/MacOS/libPsFastXmlCHECKED.a"
                                                              "${LibsPath}/MacOS/libPxFoundationCHECKED.a"
                                                              "${LibsPath}/MacOS/libPxPvdSDKCHECKED.a"
                                                              "${LibsPath}/MacOS/libPxTaskCHECKED.a"
                                                              "${LibsPath}/MacOS/libSceneQueryCHECKED.a"
                                                              "${LibsPath}/MacOS/libSimulationControllerCHECKED.a")

set(MACOS_STATIC_PROFILE_LIBS "${LibsPath}/MacOS/libLowLevelPROFILE.a"
                                                              "${LibsPath}/MacOS/libLowLevelAABBPROFILE.a"
                                                              "${LibsPath}/MacOS/libLowLevelClothPROFILE.a"
                                                              "${LibsPath}/MacOS/libLowLevelDynamicsPROFILE.a"
                                                              "${LibsPath}/MacOS/libLowLevelParticlesPROFILE.a"
                                                              "${LibsPath}/MacOS/libPhysX3PROFILE.a"
                                                              "${LibsPath}/MacOS/libPhysX3CharacterKinematicPROFILE.a"
                                                              "${LibsPath}/MacOS/libPhysX3CommonPROFILE.a"
                                                              "${LibsPath}/MacOS/libPhysX3CookingPROFILE.a"
                                                              "${LibsPath}/MacOS/libPhysX3ExtensionsPROFILE.a"
                                                              "${LibsPath}/MacOS/libPhysX3VehiclePROFILE.a"
                                                              "${LibsPath}/MacOS/libPsFastXmlPROFILE.a"
                                                              "${LibsPath}/MacOS/libPxFoundationPROFILE.a"
                                                              "${LibsPath}/MacOS/libPxPvdSDKPROFILE.a"
                                                              "${LibsPath}/MacOS/libPxTaskPROFILE.a"
                                                              "${LibsPath}/MacOS/libSceneQueryPROFILE.a"
                                                              "${LibsPath}/MacOS/libSimulationControllerPROFILE.a")

set(MACOS_STATIC_RELEASE_LIBS "${LibsPath}/MacOS/libLowLevel.a"
                                                                    "${LibsPath}/MacOS/libLowLevelAABB.a"
                                                                    "${LibsPath}/MacOS/libLowLevelCloth.a"
                                                                    "${LibsPath}/MacOS/libLowLevelDynamics.a"
                                                                    "${LibsPath}/MacOS/libLowLevelParticles.a"
                                                                    "${LibsPath}/MacOS/libPhysX3.a"
                                                                    "${LibsPath}/MacOS/libPhysX3CharacterKinematic.a"
                                                                    "${LibsPath}/MacOS/libPhysX3Common.a"
                                                                    "${LibsPath}/MacOS/libPhysX3Cooking.a"
                                                                    "${LibsPath}/MacOS/libPhysX3Extensions.a"
                                                                    "${LibsPath}/MacOS/libPhysX3Vehicle.a"
                                                                    "${LibsPath}/MacOS/libPsFastXml.a"
                                                                    "${LibsPath}/MacOS/libPxFoundation.a"
                                                                    "${LibsPath}/MacOS/libPxPvdSDK.a"
                                                                    "${LibsPath}/MacOS/libPxTask.a"
                                                                    "${LibsPath}/MacOS/libSceneQuery.a"
                                                                    "${LibsPath}/MacOS/libSimulationController.a")

set(STATIC_LIBRARIES_MACOS_RELEASE "${MACOS_STATIC_RELEASE_LIBS}")
set(STATIC_LIBRARIES_MACOS_DEBUG "${MACOS_STATIC_DEBUG_LIBS}")

set(STATIC_LIBRARIES_WIN32_RELEASE "${WIN32_STATIC_RELEASE_LIBS}")
set(DYNAMIC_LIBRARIES_WIN32_RELEASE "${WIN32_DYNAMIC_RELEASE_LIBS}")

set(STATIC_LIBRARIES_WIN32_DEBUG "${WIN32_STATIC_DEBUG_LIBS}")
set(DYNAMIC_LIBRARIES_WIN32_DEBUG "${WIN32_DYNAMIC_DEBUG_LIBS}")

set(STATIC_LIBRARIES_WIN64_RELEASE  "${WIN64_STATIC_RELEASE_LIBS}")
set(DYNAMIC_LIBRARIES_WIN64_RELEASE "${WIN64_DYNAMIC_RELEASE_LIBS}")

set(STATIC_LIBRARIES_WIN64_DEBUG "${WIN64_STATIC_DEBUG_LIBS}")
set(DYNAMIC_LIBRARIES_WIN64_DEBUG "${WIN64_DYNAMIC_DEBUG_LIBS}")

set(BINARY_WIN32_DIR_RELEASE "${LibsPath}/Win32/x86")
set(BINARY_WIN32_DIR_RELWITHDEB "${LibsPath}/Win32/x86")
set(BINARY_WIN32_DIR_DEBUG "${LibsPath}/Win32/x86")
set(BINARY_WIN64_DIR_RELEASE "${LibsPath}/Win32/x64")
set(BINARY_WIN64_DIR_RELWITHDEB "${LibsPath}/Win32/x64")
set(BINARY_WIN64_DIR_DEBUG "${LibsPath}/Win32/x64")

setup_main_module()
