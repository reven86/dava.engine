import os
import shutil
import build_utils

def get_supported_targets_for_build_platform(platform):
    if platform == 'win32':
        return ['win32']
    else:
        return ['macos']

def get_dependencies_for_target(target):
    if target =='win32':
        return ['zlib']
    else:
        return []

def get_supported_build_platforms():
    return ['win32', 'darwin']

def build_for_target(target, working_directory_path, root_project_path):
    if target == 'win32':
        return _build_win32(working_directory_path, root_project_path)
    elif target == 'macos':
        return _build_macos(working_directory_path, root_project_path)

def get_download_url():
    return 'https://sourceforge.net/projects/libpsd/files/libpsd/0.9/libpsd-0.9.zip'

def _download_and_extract(working_directory_path):
    source_folder_path = os.path.join(working_directory_path, 'libpsd_source')
    url = get_download_url()
    build_utils.download_and_extract(url, working_directory_path, source_folder_path, build_utils.get_url_file_name_no_ext(url))    
    return source_folder_path

def _patch_sources(source_folder_path, working_directory_path):
    try:
        if _patch_sources.did:
            return
    except AttributeError:
        pass

    build_utils.apply_patch(os.path.abspath('patch.diff'), working_directory_path)

    shutil.copyfile('CMakeLists.txt', os.path.join(source_folder_path, 'CMakeLists.txt'))

    _patch_sources.did = True

def _build_macos(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    _patch_sources(source_folder_path, working_directory_path)

    build_utils.build_and_copy_libraries_macos_cmake(
        os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
        'psd.xcodeproj', 'psd',
        'libpsd.a',
        'libpsd.a')

    _copy_headers(source_folder_path, root_project_path)

    return True

def _build_win32(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    _patch_sources(source_folder_path, working_directory_path)
    
    build_utils.build_and_copy_libraries_win32_cmake(
        os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
        'psd.sln', 'psd',
        'psd.lib', 'psd.lib',
        'libpsd.lib', 'libpsd.lib',
        'libpsd.lib', 'libpsd.lib')

    _copy_headers(source_folder_path, root_project_path)

    return True

def _copy_headers(source_folder_path, root_project_path):
    include_path = os.path.join(root_project_path, 'Libs/include/libpsd')
    build_utils.copy_files(os.path.join(source_folder_path, 'include'), include_path, '*.h')