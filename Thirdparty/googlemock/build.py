#!/usr/bin/env python

import sys
import os
import shutil
import build_utils
import build_config
import subprocess

def get_supported_targets_for_build_platform(platform):
    if platform == 'win32':
        return ['win32']
    else:
        return ['macos']

def get_dependencies_for_target(target):
    return []

def get_supported_build_platforms():
    return ['win32', 'darwin']

def build_for_target(target, working_directory_path, root_project_path):
    if target == 'win32':
        return _build_win32(working_directory_path, root_project_path)
    elif target == 'macos':
        return _build_macos(working_directory_path, root_project_path)

def get_download_url():
    return 'https://github.com/google/googletest.git'

def _copyLib(src, dst):
    if not os.path.isdir(dst):
        os.makedirs(dst)
    shutil.copy2(src, dst)

def _download(working_directory_path):
    source_folder_path = os.path.join(working_directory_path, 'googletest')
    build_utils.run_process('git clone ' + get_download_url(), process_cwd=working_directory_path, shell=True)
    return source_folder_path

def _patch_sources(working_directory_path):
    # Skip if we've already did the job once
    try:
        if _patch_sources.did:
            return
    except AttributeError:
        pass

    # Apply fixes
    build_utils.apply_patch(os.path.abspath('patch.diff'), working_directory_path)
    _patch_sources.did = True

def _build_win32(working_directory_path, root_project_path):
    source_folder_path = _download(working_directory_path)
    _patch_sources(source_folder_path)

    build_folder86 = os.path.join(source_folder_path, '_build86')
    build_folder64 = os.path.join(source_folder_path, '_build64')

    build_utils.cmake_generate(build_folder86, source_folder_path, build_config.win32_x86_cmake_generator)
    build_utils.cmake_build(build_folder86, 'Debug')
    build_utils.cmake_build(build_folder86, 'Release')

    build_utils.cmake_generate(build_folder64, source_folder_path, build_config.win32_x64_cmake_generator)
    build_utils.cmake_build(build_folder64, 'Debug')
    build_utils.cmake_build(build_folder64, 'Release')

    win_libs_folder = os.path.join(root_project_path, 'Modules/TArc/Libs/Win32')
    if os.path.exists(win_libs_folder) and os.path.isdir(win_libs_folder):
        shutil.rmtree(win_libs_folder)

    lib_path_x86_debug = os.path.join(build_folder86, 'googlemock', 'Debug', 'gmock.lib')
    lib_path_x86_release = os.path.join(build_folder86, 'googlemock', 'Release', 'gmock.lib')
    lib_path_x64_debug = os.path.join(build_folder64, 'googlemock', 'Debug', 'gmock.lib')
    lib_path_x64_release = os.path.join(build_folder64, 'googlemock', 'Release', 'gmock.lib')

    _copyLib(lib_path_x86_debug, os.path.join(win_libs_folder, 'x86/Debug/'))
    _copyLib(lib_path_x86_release, os.path.join(win_libs_folder, 'x86/Release/'))
    _copyLib(lib_path_x64_debug, os.path.join(win_libs_folder, 'x64/Debug/'))
    _copyLib(lib_path_x64_release, os.path.join(win_libs_folder, 'x64/Release/'))

    _copy_headers(source_folder_path, root_project_path)

    return True

def _build_macos(working_directory_path, root_project_path):
    source_folder_path = _download(working_directory_path)
    _patch_sources(source_folder_path)

    build_folder = os.path.join(source_folder_path, '_build')

    build_utils.cmake_generate(build_folder, source_folder_path, build_config.macos_cmake_generator)
    build_utils.cmake_build(build_folder, 'Debug')
    build_utils.cmake_build(build_folder, 'Release')

    lib_path_debug = os.path.join(build_folder, 'googlemock', 'Debug', 'libgmock.a')
    lib_path_release = os.path.join(build_folder,'googlemock', 'Release', 'libgmock.a')

    mac_libs_folder = os.path.join(root_project_path, 'Modules/TArc/Libs/Mac/')
    build_utils.clear_files(mac_libs_folder, '*/*.a')
    _copyLib(lib_path_debug, os.path.join(mac_libs_folder, 'Debug/'))
    _copyLib(lib_path_release, os.path.join(mac_libs_folder, 'Release/'))

    _copy_headers(source_folder_path, root_project_path)

    return True

def _copy_headers(source_folder_path, root_project_path):
    gmock_from_dir = os.path.join(source_folder_path, 'googlemock/include/gmock')
    gmock_to_dir = os.path.join(root_project_path, 'Modules/TArc/Libs/Include/gmock')
    gtest_from_dir = os.path.join(source_folder_path, 'googletest/include/gtest')
    gtest_to_dir = os.path.join(root_project_path, 'Modules/TArc/Libs/Include/gtest')
    build_utils.clean_copy_includes(gmock_from_dir, gmock_to_dir)
    build_utils.clean_copy_includes(gtest_from_dir, gtest_to_dir)

