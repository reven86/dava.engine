import os
import shutil
import build_utils
import build_config


def get_supported_targets(platform):
    if platform == 'win32':
        return ['win32', 'win10', 'android']
    elif platform == 'darwin':
        return ['macos', 'ios', 'android']
    elif platform == 'linux':
        return ['android', 'linux']
    else:
        return []


def get_dependencies_for_target(target):
    return []


def build_for_target(target, working_directory_path, root_project_path):
    if target == 'win32':
        _build_win32(working_directory_path, root_project_path)
    elif target == 'win10':
        _build_win10(working_directory_path, root_project_path)
    elif target == 'macos':
        _build_macos(working_directory_path, root_project_path)
    elif target == 'ios':
        _build_ios(working_directory_path, root_project_path)
    elif target == 'android':
        _build_android(working_directory_path, root_project_path)
    elif target == 'linux':
        _build_linux(working_directory_path, root_project_path)


def get_download_info():
    return 'Libs/mongodb'


def _build_win32(working_directory_path, root_project_path):
    source_folder_path=os.path.join(root_project_path, 'Libs/mongodb')

    # TODO: implement win32


def _build_win10(working_directory_path, root_project_path):
    source_folder_path=os.path.join(root_project_path, 'Libs/mongodb')

    # TODO: implement win10


def _build_macos(working_directory_path, root_project_path):
    source_folder_path=os.path.join(root_project_path, 'Libs/mongodb')

    # TODO: implement macos


def _build_ios(working_directory_path, root_project_path):
    source_folder_path=os.path.join(root_project_path, 'Libs/mongodb')

    # TODO: implement ios


def _build_android(working_directory_path, root_project_path):
    source_folder_path=os.path.join(root_project_path, 'Libs/mongodb')

    # TODO: implement android

def _build_linux(working_directory_path, root_project_path):
    source_folder_path=os.path.join(root_project_path, 'Libs/mongodb')

    build_utils.build_and_copy_libraries_linux_cmake(
        os.path.join(working_directory_path, '_build'),
        source_folder_path,
        root_project_path,
        target='all',
        lib_name='libmongodb.a')
    
    _copy_headers(source_folder_path, root_project_path)


def _copy_headers(source_folder_path, root_project_path):
    src_dir = os.path.join(source_folder_path, 'include/mongodb')
    include_path = os.path.join(root_project_path, 'Libs/include/mongodb')

    build_utils.copy_files(src_dir, include_path, '*.h')
