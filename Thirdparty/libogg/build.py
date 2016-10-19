import os
import shutil
import build_utils

def get_supported_targets_for_build_platform(platform):
    if platform == 'win32':
        return ['win32', 'win10']
    else:
        return ['macos', 'ios']

def get_dependencies_for_target(target):
    return []

def get_supported_build_platforms():
    return ['win32', 'darwin']

def build_for_target(target, working_directory_path, root_project_path):
    if target == 'win32':
        return _build_win32(working_directory_path, root_project_path)
    elif target == 'win10':
        return _build_win10(working_directory_path, root_project_path)
    elif target == 'macos':
        return _build_macos(working_directory_path, root_project_path)
    elif target == 'ios':
        return _build_ios(working_directory_path, root_project_path)

def get_download_url():
    return 'http://downloads.xiph.org/releases/ogg/libogg-1.3.2.tar.gz'

def _download_and_extract(working_directory_path, source_folder_path_prefix=''):
    source_folder_path = os.path.join(working_directory_path, 'libogg_source' + source_folder_path_prefix)
    url = get_download_url()
    build_utils.download_and_extract(url, working_directory_path, source_folder_path, build_utils.get_url_file_name_no_ext(url))    
    return source_folder_path

def _patch_sources(source_folder_path, working_directory_path, patch_postifx):
    try:
        if source_folder_path in _patch_sources.cache:
            return
    except AttributeError:
        _patch_sources.cache = []
        pass

    # Apply fixes
    build_utils.apply_patch(os.path.abspath('patch' + patch_postifx + '.diff'), working_directory_path)

    _patch_sources.cache.append(source_folder_path)

def _build_win32(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    vc_solution_file_path = os.path.join(source_folder_path, 'win32/VS2010/libogg_static.sln')
    build_utils.build_vs(vc_solution_file_path, 'Debug', 'Win32', 'libogg_static', 'v120')
    build_utils.build_vs(vc_solution_file_path, 'Release', 'Win32', 'libogg_static', 'v120')
    build_utils.build_vs(vc_solution_file_path, 'Debug', 'x64', 'libogg_static', 'v120')
    build_utils.build_vs(vc_solution_file_path, 'Release', 'x64', 'libogg_static', 'v120')

    return True

def _build_win10(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path, '_win10')
    _patch_sources(source_folder_path, working_directory_path, '_win10')

    vc_solution_file_path = os.path.join(source_folder_path, 'win32/VS2010/libogg_static.sln')
    build_utils.build_vs(vc_solution_file_path, 'Debug', 'Win32', 'libogg_static')
    build_utils.build_vs(vc_solution_file_path, 'Release', 'Win32', 'libogg_static')
    build_utils.build_vs(vc_solution_file_path, 'Debug', 'x64', 'libogg_static')
    build_utils.build_vs(vc_solution_file_path, 'Release', 'x64', 'libogg_static')
    build_utils.build_vs(vc_solution_file_path, 'Debug', 'ARM', 'libogg_static')
    build_utils.build_vs(vc_solution_file_path, 'Release', 'ARM', 'libogg_static')

    return True

def _build_macos(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    install_dir_macos = os.path.join(working_directory_path, 'gen/install_macos')
    build_utils.build_with_autotools(source_folder_path, ['--host=x86_64-apple-darwin', '--disable-shared', '--enable-static'], install_dir_macos, env=build_utils.get_autotools_macos_env(), postclean=False)

    return True

def _build_ios(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    install_dir_ios = os.path.join(working_directory_path, 'gen/install_ios')
    build_utils.build_with_autotools(source_folder_path, ['--host=armv7-apple-darwin', '--disable-shared', '--enable-static'], install_dir_ios, env=build_utils.get_autotools_ios_env())
    
    return True

# TODO: Add copying headers & libraries when switching to new directories structure