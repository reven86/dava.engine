import os
import shutil
import build_utils


def get_supported_targets(platform):
    if platform == 'win32':
        return ['win32', 'win10', 'android']
    else:
        return ['macos', 'ios', 'android']


def get_dependencies_for_target(target):
    return []


def build_for_target(target, working_directory_path, root_project_path):
    if target == 'win32':
        _build_win32(working_directory_path, root_project_path)
   # elif target == 'win10':
   #     _build_win10(working_directory_path, root_project_path)
    #elif target == 'macos':
   #     _build_macos(working_directory_path, root_project_path)
   # elif target == 'ios':
   #     _build_ios(working_directory_path, root_project_path)
   # elif target == 'android':
   #     _build_android(working_directory_path, root_project_path)


def get_download_info():
    return "https://share.wargaming.net/f/9f184d56e0/?raw=1"


def _create_folder_if_not_exists(path):
    if not os.path.isdir(path):
        os.makedirs(path)


def _copy_libs(src, dst, files_ext):
    _create_folder_if_not_exists(dst)
    for fileName in os.listdir(src):
        if fileName.endswith(files_ext):
            shutil.copyfile(os.path.join(src, fileName), os.path.join(dst, fileName))


def _download_and_extract(working_directory_path):
    source_archive_filepath = os.path.join(working_directory_path, 'physx.zip')

    url = get_download_info()
    build_utils.download(url, source_archive_filepath)
    build_utils.unzip_inplace(source_archive_filepath)
    return os.path.join(working_directory_path, 'PhysX-3.4-master')


@build_utils.run_once
def _patch_sources(patch_name, working_directory_path):
    build_utils.apply_patch(
        os.path.abspath(patch_name), working_directory_path)


def _build_win32(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    _patch_sources('patch_win32.diff', working_directory_path)
    project_x86_path = os.path.join(source_folder_path, 'PhysX_3.4', 'Source', 'compiler', 'vc12win32', 'PhysX.sln')
    project_x64_path = os.path.join(source_folder_path, 'PhysX_3.4', 'Source', 'compiler', 'vc12win64', 'PhysX.sln')

    binary_dst_path = os.path.join(root_project_path, 'Modules', 'Physics', 'Libs', 'Win32')
    x86_binary_dst_path = os.path.join(binary_dst_path, 'x86')
    x64_binary_dst_path = os.path.join(binary_dst_path, 'x64')

    #build_utils.build_vs(project_x86_path, 'debug')
    build_utils.build_vs(project_x86_path, 'checked')
    #build_utils.build_vs(project_x86_path, 'profile')
    build_utils.build_vs(project_x86_path, 'release')
    _copy_libs(os.path.join(source_folder_path, 'PhysX_3.4', 'Bin', 'vc12win32'), x86_binary_dst_path,  '.dll')
    _copy_libs(os.path.join(source_folder_path, 'PhysX_3.4', 'Lib', 'vc12win32'), x86_binary_dst_path,  '.lib')

    #build_utils.build_vs(project_x64_path, 'debug', 'x64')
    build_utils.build_vs(project_x64_path, 'checked', 'x64')
    #build_utils.build_vs(project_x64_path, 'profile', 'x64')
    build_utils.build_vs(project_x64_path, 'release', 'x64')
    _copy_libs(os.path.join(source_folder_path, 'PhysX_3.4', 'Bin', 'vc12win64'), x64_binary_dst_path,  '.dll')
    _copy_libs(os.path.join(source_folder_path, 'PhysX_3.4', 'Lib', 'vc12win64'), x64_binary_dst_path,  '.lib')

    _copy_headers(source_folder_path, root_project_path)


# def _build_macos(working_directory_path, root_project_path):
#     source_folder_path = _download(working_directory_path)
#     _patch_sources(source_folder_path)
#     build_utils.build_and_copy_libraries_macos_cmake(
#         os.path.join(source_folder_path, '_build'),
#         source_folder_path,
#         root_project_path,
#         'googletest-distribution.xcodeproj', 'gmock',
#         'libgmock.a', 'libgmock.a',
#         target_lib_subdir='googlemock')
#
#     _copy_headers(source_folder_path, root_project_path)
#
#
def _copy_headers(source_folder_path, root_project_path):
    copy_to_folder = os.path.join(root_project_path, 'Modules', 'Physics', 'Libs', 'Include')
    copy_from_folder = os.path.join(source_folder_path, 'PhysX_3.4', 'Include')
    _create_folder_if_not_exists(copy_to_folder)
    build_utils.clean_copy_includes(copy_from_folder, copy_to_folder)
