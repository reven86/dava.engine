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

set(WIN_UAP_STATIC_DEBUG_LIBS "${LibsPath}/Win10/$(Platform)/LowLevelDEBUG.lib"
                              "${LibsPath}/Win10/$(Platform)/LowLevelAABBDEBUG.lib"
                              "${LibsPath}/Win10/$(Platform)/LowLevelClothDEBUG.lib"
                              "${LibsPath}/Win10/$(Platform)/LowLevelDynamicsDEBUG.lib"
                              "${LibsPath}/Win10/$(Platform)/LowLevelParticlesDEBUG.lib"
                              "${LibsPath}/Win10/$(Platform)/PhysX3DEBUG_$(PlatformShortName).lib"
                              "${LibsPath}/Win10/$(Platform)/PhysX3CharacterKinematicDEBUG_$(PlatformShortName).lib"
                              "${LibsPath}/Win10/$(Platform)/PhysX3CommonDEBUG_$(PlatformShortName).lib"
                              "${LibsPath}/Win10/$(Platform)/PhysX3CookingDEBUG_$(PlatformShortName).lib"
                              "${LibsPath}/Win10/$(Platform)/PhysX3ExtensionsDEBUG.lib"
                              "${LibsPath}/Win10/$(Platform)/PhysX3VehicleDEBUG.lib"
                              "${LibsPath}/Win10/$(Platform)/PsFastXmlDEBUG_$(PlatformShortName).lib"
                              "${LibsPath}/Win10/$(Platform)/PxFoundationDEBUG_$(PlatformShortName).lib"
                              "${LibsPath}/Win10/$(Platform)/PxPvdSDKDEBUG_$(PlatformShortName).lib"
                              "${LibsPath}/Win10/$(Platform)/PxTaskDEBUG_$(PlatformShortName).lib"
                              "${LibsPath}/Win10/$(Platform)/SceneQueryDEBUG.lib"
                              "${LibsPath}/Win10/$(Platform)/SimulationControllerDEBUG.lib")

set(WIN_UAP_DYNAMIC_DEBUG_LIBS "${LibsPath}/Win10/Win32/nvToolsExt32_1.dll"
                               "${LibsPath}/Win10/Win32/PhysX3DEBUG_x86.dll"
                               "${LibsPath}/Win10/Win32/PhysX3CharacterKinematicDEBUG_x86.dll"
                               "${LibsPath}/Win10/Win32/PhysX3CommonDEBUG_x86.dll"
                               "${LibsPath}/Win10/Win32/PhysX3CookingDEBUG_x86.dll"
                               "${LibsPath}/Win10/Win32/PxFoundationDEBUG_x86.dll"
                               "${LibsPath}/Win10/Win32/PxPvdSDKDEBUG_x86.dll"
                               "${LibsPath}/Win10/x64/nvToolsExt64_1.dll"
                               "${LibsPath}/Win10/x64/PhysX3DEBUG_x64.dll"
                               "${LibsPath}/Win10/x64/PhysX3CharacterKinematicDEBUG_x64.dll"
                               "${LibsPath}/Win10/x64/PhysX3CommonDEBUG_x64.dll"
                               "${LibsPath}/Win10/x64/PhysX3CookingDEBUG_x64.dll"
                               "${LibsPath}/Win10/x64/PxFoundationDEBUG_x64.dll"
                               "${LibsPath}/Win10/x64/PxPvdSDKDEBUG_x64.dll")

set(WIN_UAP_STATIC_CHECKED_LIBS "${LibsPath}/Win10/$(Platform)/LowLevelCHECKED.lib"
                                "${LibsPath}/Win10/$(Platform)/LowLevelAABBCHECKED.lib"
                                "${LibsPath}/Win10/$(Platform)/LowLevelClothCHECKED.lib"
                                "${LibsPath}/Win10/$(Platform)/LowLevelDynamicsCHECKED.lib"
                                "${LibsPath}/Win10/$(Platform)/LowLevelParticlesCHECKED.lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3CHECKED_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3CharacterKinematicCHECKED_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3CommonCHECKED_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3CookingCHECKED_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3ExtensionsCHECKED.lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3VehicleCHECKED.lib"
                                "${LibsPath}/Win10/$(Platform)/PsFastXmlCHECKED_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PxFoundationCHECKED_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PxPvdSDKCHECKED_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PxTaskCHECKED_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/SceneQueryCHECKED.lib"
                                "${LibsPath}/Win10/$(Platform)/SimulationControllerCHECKED.lib")

set(WIN_UAP_DYNAMIC_CHECKED_LIBS "${LibsPath}/Win10/Win32/nvToolsExt32_1.dll"
                                 "${LibsPath}/Win10/Win32/PhysX3CHECKED_x86.dll"
                                 "${LibsPath}/Win10/Win32/PhysX3CharacterKinematicCHECKED_x86.dll"
                                 "${LibsPath}/Win10/Win32/PhysX3CommonCHECKED_x86.dll"
                                 "${LibsPath}/Win10/Win32/PhysX3CookingCHECKED_x86.dll"
                                 "${LibsPath}/Win10/Win32/PxFoundationCHECKED_x86.dll"
                                 "${LibsPath}/Win10/Win32/PxPvdSDKCHECKED_x86.dll"
                                 "${LibsPath}/Win10/x64/nvToolsExt64_1.dll"
                                 "${LibsPath}/Win10/x64/PhysX3CHECKED_x64.dll"
                                 "${LibsPath}/Win10/x64/PhysX3CharacterKinematicCHECKED_x64.dll"
                                 "${LibsPath}/Win10/x64/PhysX3CommonCHECKED_x64.dll"
                                 "${LibsPath}/Win10/x64/PhysX3CookingCHECKED_x64.dll"
                                 "${LibsPath}/Win10/x64/PxFoundationCHECKED_x64.dll"
                                 "${LibsPath}/Win10/x64/PxPvdSDKCHECKED_x64.dll")

set(WIN_UAP_STATIC_PROFILE_LIBS "${LibsPath}/Win10/$(Platform)/LowLevelPROFILE.lib"
                                "${LibsPath}/Win10/$(Platform)/LowLevelAABBPROFILE.lib"
                                "${LibsPath}/Win10/$(Platform)/LowLevelClothPROFILE.lib"
                                "${LibsPath}/Win10/$(Platform)/LowLevelDynamicsPROFILE.lib"
                                "${LibsPath}/Win10/$(Platform)/LowLevelParticlesPROFILE.lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3PROFILE_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3CharacterKinematicPROFILE_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3CommonPROFILE_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3CookingPROFILE_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3ExtensionsPROFILE.lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3VehiclePROFILE.lib"
                                "${LibsPath}/Win10/$(Platform)/PsFastXmlPROFILE_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PxFoundationPROFILE_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PxPvdSDKPROFILE_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PxTaskPROFILE_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/SceneQueryPROFILE.lib"
                                "${LibsPath}/Win10/$(Platform)/SimulationControllerPROFILE.lib")

set(WIN_UAP_DYNAMIC_PROFILE_LIBS "${LibsPath}/Win10/Win32/nvToolsExt32_1.dll"
                                 "${LibsPath}/Win10/Win32/PhysX3PROFILE_x86.dll"
                                 "${LibsPath}/Win10/Win32/PhysX3CharacterKinematicPROFILE_x86.dll"
                                 "${LibsPath}/Win10/Win32/PhysX3CommonPROFILE_x86.dll"
                                 "${LibsPath}/Win10/Win32/PhysX3CookingPROFILE_x86.dll"
                                 "${LibsPath}/Win10/Win32/PxFoundationPROFILE_x86.dll"
                                 "${LibsPath}/Win10/Win32/PxPvdSDKPROFILE_x86.dll"
                                 "${LibsPath}/Win10/x64/nvToolsExt64_1.dll"
                                 "${LibsPath}/Win10/x64/PhysX3PROFILE_x64.dll"
                                 "${LibsPath}/Win10/x64/PhysX3CharacterKinematicPROFILE_x64.dll"
                                 "${LibsPath}/Win10/x64/PhysX3CommonPROFILE_x64.dll"
                                 "${LibsPath}/Win10/x64/PhysX3CookingPROFILE_x64.dll"
                                 "${LibsPath}/Win10/x64/PxFoundationPROFILE_x64.dll"
                                 "${LibsPath}/Win10/x64/PxPvdSDKPROFILE_x64.dll")

set(WIN_UAP_STATIC_RELEASE_LIBS "${LibsPath}/Win10/$(Platform)/LowLevel.lib"
                                "${LibsPath}/Win10/$(Platform)/LowLevelAABB.lib"
                                "${LibsPath}/Win10/$(Platform)/LowLevelCloth.lib"
                                "${LibsPath}/Win10/$(Platform)/LowLevelDynamics.lib"
                                "${LibsPath}/Win10/$(Platform)/LowLevelParticles.lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3CharacterKinematic_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3Common_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3Cooking_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3Extensions.lib"
                                "${LibsPath}/Win10/$(Platform)/PhysX3Vehicle.lib"
                                "${LibsPath}/Win10/$(Platform)/PsFastXml_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PxFoundation_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/PxTask_$(PlatformShortName).lib"
                                "${LibsPath}/Win10/$(Platform)/SceneQuery.lib"
                                "${LibsPath}/Win10/$(Platform)/SimulationController.lib")

set(WIN_UAP_DYNAMIC_RELEASE_LIBS "${LibsPath}/Win10/Win32/PhysX3_x86.dll"
                                 "${LibsPath}/Win10/Win32/PhysX3CharacterKinematic_x86.dll"
                                 "${LibsPath}/Win10/Win32/PhysX3Common_x86.dll"
                                 "${LibsPath}/Win10/Win32/PhysX3Cooking_x86.dll"
                                 "${LibsPath}/Win10/Win32/PxFoundation_x86.dll"
                                 "${LibsPath}/Win10/x64/PhysX3_x64.dll"
                                 "${LibsPath}/Win10/x64/PhysX3CharacterKinematic_x64.dll"
                                 "${LibsPath}/Win10/x64/PhysX3Common_x64.dll"
                                 "${LibsPath}/Win10/x64/PhysX3Cooking_x64.dll"
                                 "${LibsPath}/Win10/x64/PxFoundation_x64.dll")

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

set(IOS_STATIC_DEBUG_LIBS "${LibsPath}/iOS/libLowLevelDEBUG.a"
                          "${LibsPath}/iOS/libLowLevelAABBDEBUG.a"
                          "${LibsPath}/iOS/libLowLevelClothDEBUG.a"
                          "${LibsPath}/iOS/libLowLevelDynamicsDEBUG.a"
                          "${LibsPath}/iOS/libLowLevelParticlesDEBUG.a"
                          "${LibsPath}/iOS/libPhysX3DEBUG.a"
                          "${LibsPath}/iOS/libPhysX3CharacterKinematicDEBUG.a"
                          "${LibsPath}/iOS/libPhysX3CommonDEBUG.a"
                          "${LibsPath}/iOS/libPhysX3CookingDEBUG.a"
                          "${LibsPath}/iOS/libPhysX3ExtensionsDEBUG.a"
                          "${LibsPath}/iOS/libPhysX3VehicleDEBUG.a"
                          "${LibsPath}/iOS/libPsFastXmlDEBUG.a"
                          "${LibsPath}/iOS/libPxFoundationDEBUG.a"
                          "${LibsPath}/iOS/libPxPvdSDKDEBUG.a"
                          "${LibsPath}/iOS/libPxTaskDEBUG.a"
                          "${LibsPath}/iOS/libSceneQueryDEBUG.a"
                          "${LibsPath}/iOS/libSimulationControllerDEBUG.a")

set(IOS_STATIC_CHECKED_LIBS "${LibsPath}/iOS/libLowLevelCHECKED.a"
                            "${LibsPath}/iOS/libLowLevelAABBCHECKED.a"
                            "${LibsPath}/iOS/libLowLevelClothCHECKED.a"
                            "${LibsPath}/iOS/libLowLevelDynamicsCHECKED.a"
                            "${LibsPath}/iOS/libLowLevelParticlesCHECKED.a"
                            "${LibsPath}/iOS/libPhysX3CHECKED.a"
                            "${LibsPath}/iOS/libPhysX3CharacterKinematicCHECKED.a"
                            "${LibsPath}/iOS/libPhysX3CommonCHECKED.a"
                            "${LibsPath}/iOS/libPhysX3CookingCHECKED.a"
                            "${LibsPath}/iOS/libPhysX3ExtensionsCHECKED.a"
                            "${LibsPath}/iOS/libPhysX3VehicleCHECKED.a"
                            "${LibsPath}/iOS/libPsFastXmlCHECKED.a"
                            "${LibsPath}/iOS/libPxFoundationCHECKED.a"
                            "${LibsPath}/iOS/libPxPvdSDKCHECKED.a"
                            "${LibsPath}/iOS/libPxTaskCHECKED.a"
                            "${LibsPath}/iOS/libSceneQueryCHECKED.a"
                            "${LibsPath}/iOS/libSimulationControllerCHECKED.a")

set(IOS_STATIC_PROFILE_LIBS "${LibsPath}/iOS/libLowLevelPROFILE.a"
                            "${LibsPath}/iOS/libLowLevelAABBPROFILE.a"
                            "${LibsPath}/iOS/libLowLevelClothPROFILE.a"
                            "${LibsPath}/iOS/libLowLevelDynamicsPROFILE.a"
                            "${LibsPath}/iOS/libLowLevelParticlesPROFILE.a"
                            "${LibsPath}/iOS/libPhysX3PROFILE.a"
                            "${LibsPath}/iOS/libPhysX3CharacterKinematicPROFILE.a"
                            "${LibsPath}/iOS/libPhysX3CommonPROFILE.a"
                            "${LibsPath}/iOS/libPhysX3CookingPROFILE.a"
                            "${LibsPath}/iOS/libPhysX3ExtensionsPROFILE.a"
                            "${LibsPath}/iOS/libPhysX3VehiclePROFILE.a"
                            "${LibsPath}/iOS/libPsFastXmlPROFILE.a"
                            "${LibsPath}/iOS/libPxFoundationPROFILE.a"
                            "${LibsPath}/iOS/libPxPvdSDKPROFILE.a"
                            "${LibsPath}/iOS/libPxTaskPROFILE.a"
                            "${LibsPath}/iOS/libSceneQueryPROFILE.a"
                            "${LibsPath}/iOS/libSimulationControllerPROFILE.a")

set(IOS_STATIC_RELEASE_LIBS "${LibsPath}/iOS/libLowLevel.a"
                            "${LibsPath}/iOS/libLowLevelAABB.a"
                            "${LibsPath}/iOS/libLowLevelCloth.a"
                            "${LibsPath}/iOS/libLowLevelDynamics.a"
                            "${LibsPath}/iOS/libLowLevelParticles.a"
                            "${LibsPath}/iOS/libPhysX3.a"
                            "${LibsPath}/iOS/libPhysX3CharacterKinematic.a"
                            "${LibsPath}/iOS/libPhysX3Common.a"
                            "${LibsPath}/iOS/libPhysX3Cooking.a"
                            "${LibsPath}/iOS/libPhysX3Extensions.a"
                            "${LibsPath}/iOS/libPhysX3Vehicle.a"
                            "${LibsPath}/iOS/libPsFastXml.a"
                            "${LibsPath}/iOS/libPxFoundation.a"
                            "${LibsPath}/iOS/libPxPvdSDK.a"
                            "${LibsPath}/iOS/libPxTask.a"
                            "${LibsPath}/iOS/libSceneQuery.a"
                            "${LibsPath}/iOS/libSimulationController.a")

set(STATIC_LIBRARIES_WIN32_RELEASE "${WIN32_STATIC_RELEASE_LIBS}")
set(DYNAMIC_LIBRARIES_WIN32_RELEASE "${WIN32_DYNAMIC_RELEASE_LIBS}")

set(STATIC_LIBRARIES_WIN32_DEBUG "${WIN32_STATIC_DEBUG_LIBS}")
set(DYNAMIC_LIBRARIES_WIN32_DEBUG "${WIN32_DYNAMIC_DEBUG_LIBS}")

set(STATIC_LIBRARIES_WIN64_RELEASE "${WIN64_STATIC_RELEASE_LIBS}")
set(DYNAMIC_LIBRARIES_WIN64_RELEASE "${WIN64_DYNAMIC_RELEASE_LIBS}")

set(STATIC_LIBRARIES_WIN64_DEBUG "${WIN64_STATIC_DEBUG_LIBS}")
set(DYNAMIC_LIBRARIES_WIN64_DEBUG "${WIN64_DYNAMIC_DEBUG_LIBS}")

set(STATIC_LIBRARIES_WINUAP_RELEASE "${WIN_UAP_STATIC_RELEASE_LIBS}")
set(DYNAMIC_LIBRARIES_WINUAP_RELEASE "${WIN_UAP_DYNAMIC_RELEASE_LIBS}")

set(STATIC_LIBRARIES_WINUAP_DEBUG "${WIN_UAP_STATIC_DEBUG_LIBS}")
set(DYNAMIC_LIBRARIES_WINUAP_DEBUG "${WIN_UAP_DYNAMIC_DEBUG_LIBS}")

set(STATIC_LIBRARIES_MACOS_RELEASE "${MACOS_STATIC_RELEASE_LIBS}")
set(STATIC_LIBRARIES_MACOS_DEBUG "${MACOS_STATIC_DEBUG_LIBS}")

set(STATIC_LIBRARIES_IOS_RELEASE "${IOS_STATIC_RELEASE_LIBS}")
set(STATIC_LIBRARIES_IOS_DEBUG "${IOS_STATIC_DEBUG_LIBS}")

set(BINARY_WIN32_DIR_RELEASE "${LibsPath}/Win32/x86")
set(BINARY_WIN32_DIR_RELWITHDEB "${LibsPath}/Win32/x86")
set(BINARY_WIN32_DIR_DEBUG "${LibsPath}/Win32/x86")
set(BINARY_WIN64_DIR_RELEASE "${LibsPath}/Win32/x64")
set(BINARY_WIN64_DIR_RELWITHDEB "${LibsPath}/Win32/x64")
set(BINARY_WIN64_DIR_DEBUG "${LibsPath}/Win32/x64")

setup_main_module()
