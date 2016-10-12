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

    build_utils.build_and_copy_libraries_win32_cmake(os.path.join(source_folder_path, '_build'), source_folder_path,
                                                     root_project_path, 'googletest-distribution.sln', 'gmock', 'gmock.lib', 'gmock.lib',
                                                     'gmock.lib', 'gmock.lib', 'gmock.lib', 'gmock.lib', target_lib_subdir='googlemock')

    _copy_headers(source_folder_path, root_project_path)

    return True

def _build_macos(working_directory_path, root_project_path):
    source_folder_path = _download(working_directory_path)
    _patch_sources(source_folder_path)
    build_utils.build_and_copy_libraries_macos_cmake(os.path.join(source_folder_path, '_build'), source_folder_path,
                                                     root_project_path, 'googletest-distribution.xcodeproj',  'gmock',
                                                     'libgmock.a', 'libgmock.a', target_lib_subdir='googlemock')

    _copy_headers(source_folder_path, root_project_path)

    return True

def _copy_headers(source_folder_path, root_project_path):
    os.path.join(root_project_path, 'Libs/include/libpng')
    gmock_from_dir = os.path.join(source_folder_path, 'googlemock/include/gmock')
    gmock_to_dir = os.path.join(root_project_path, 'Libs/include/googlemock/gmock')
    gtest_from_dir = os.path.join(source_folder_path, 'googletest/include/gtest')
    gtest_to_dir = os.path.join(root_project_path, 'Libs/include/googlemock/gtest')
    build_utils.clean_copy_includes(gmock_from_dir, gmock_to_dir)
    build_utils.clean_copy_includes(gtest_from_dir, gtest_to_dir)

