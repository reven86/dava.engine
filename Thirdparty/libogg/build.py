import os
import build_utils
import build_config


def get_supported_targets(platform):
    if platform == 'win32':
        return ['win32', 'win10']
    elif platform == 'darwin':
        return ['macos', 'ios']
    elif platform == 'linux':
        return ['linux']
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
    elif target == 'linux':
        _build_linux(working_directory_path, root_project_path)


def get_download_info():
    return 'http://downloads.xiph.org/releases/ogg/libogg-1.3.2.tar.gz'


def _download_and_extract(working_directory_path, source_folder_postfix=''):
    source_folder_name = 'libogg_source' + source_folder_postfix
    source_folder_path = os.path.join(
        working_directory_path, source_folder_name)

    url = get_download_info()
    build_utils.download_and_extract(
        url,
        working_directory_path,
        source_folder_path,
        build_utils.get_url_file_name_no_ext(url))

    return source_folder_path


def _patch_sources(source_folder_path, working_directory_path, patch_postifx):
    try:
        if source_folder_path in _patch_sources.cache:
            return
    except AttributeError:
        _patch_sources.cache = []
        pass

    # Apply fixes
    build_utils.apply_patch(
        os.path.abspath('patch' + patch_postifx + '.diff'),
        working_directory_path)

    _patch_sources.cache.append(source_folder_path)


def _build_win32(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    vc_solution_file_path = os.path.join(
        source_folder_path, 'win32/VS2010/libogg_static.sln')
    build_utils.build_vs(
        vc_solution_file_path,
        'Debug', 'Win32', 'libogg_static',
        build_config.get_msvc_toolset_ver_win32())
    build_utils.build_vs(
        vc_solution_file_path,
        'Release', 'Win32', 'libogg_static',
        build_config.get_msvc_toolset_ver_win32())
    build_utils.build_vs(
        vc_solution_file_path,
        'Debug', 'x64', 'libogg_static',
        build_config.get_msvc_toolset_ver_win32())
    build_utils.build_vs(
        vc_solution_file_path,
        'Release', 'x64', 'libogg_static',
        build_config.get_msvc_toolset_ver_win32())


def _build_win10(working_directory_path, root_project_path):
    prefix = '_win10'
    source_folder_path = _download_and_extract(working_directory_path, prefix)
    _patch_sources(source_folder_path, working_directory_path, prefix)

    vc_solution_file_path = os.path.join(
        source_folder_path, 'win32/VS2010/libogg_static.sln')
    build_utils.build_vs(
        vc_solution_file_path, 'Debug', 'Win32', 'libogg_static')
    build_utils.build_vs(
        vc_solution_file_path, 'Release', 'Win32', 'libogg_static')
    build_utils.build_vs(
        vc_solution_file_path, 'Debug', 'x64', 'libogg_static')
    build_utils.build_vs(
        vc_solution_file_path, 'Release', 'x64', 'libogg_static')
    build_utils.build_vs(
        vc_solution_file_path, 'Debug', 'ARM', 'libogg_static')
    build_utils.build_vs(
        vc_solution_file_path, 'Release', 'ARM', 'libogg_static')


def _build_macos(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    install_dir_macos = os.path.join(
        working_directory_path, 'gen/install_macos')
    build_utils.build_with_autotools(
        source_folder_path,
        ['--host=x86_64-apple-darwin', '--disable-shared', '--enable-static'],
        install_dir_macos,
        env=build_utils.get_autotools_macos_env(),
        postclean=False)


def _build_ios(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    install_dir_ios = os.path.join(working_directory_path, 'gen/install_ios')
    build_utils.build_with_autotools(
        source_folder_path,
        ['--host=armv7-apple-darwin', '--disable-shared', '--enable-static'],
        install_dir_ios,
        env=build_utils.get_autotools_ios_env())

# TODO: Add copying headers & libraries when switching to new directories structure

def _build_linux(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    env = build_utils.get_autotools_linux_env()

    install_dir = os.path.join(working_directory_path, 'gen/install_linux')

    build_utils.build_with_autotools(
        source_folder_path,
        ['--disable-shared', '--enable-static'],
        install_dir,
        env=env)
