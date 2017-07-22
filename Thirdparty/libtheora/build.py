import os
import shutil
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
    return ['libogg']


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
    return 'http://downloads.xiph.org/releases/theora/libtheora-1.1.1.tar.bz2'


def _download_and_extract(working_directory_path, source_folder_prefix=''):
    source_folder_name = 'libtheora_source' + source_folder_prefix
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
    _patch_sources(source_folder_path, working_directory_path, '_win32')

    vc_solution_file_path = os.path.join(
        source_folder_path, 'win32/VS2008/libtheora_static.sln')

    libogg_include_path = os.path.abspath(
        os.path.join(
            working_directory_path,
            '../libogg/libogg_source/include'))

    env = build_utils.get_win32_vs_x86_env()
    env['INCLUDE'] = libogg_include_path + ';' + env['INCLUDE']
    build_utils.build_vs(
        vc_solution_file_path,
        'Debug', 'Win32', 'libtheora_static',
        build_config.get_msvc_toolset_ver_win32(),
        env=env)
    build_utils.build_vs(
        vc_solution_file_path,
        'Release', 'Win32', 'libtheora_static',
        build_config.get_msvc_toolset_ver_win32(),
        env=env)

    env = build_utils.get_win32_vs_x64_env()
    env['INCLUDE'] = libogg_include_path + ';' + env['INCLUDE']
    build_utils.build_vs(
        vc_solution_file_path,
        'Debug', 'x64', 'libtheora_static',
        build_config.get_msvc_toolset_ver_win32(),
        env=env)
    build_utils.build_vs(
        vc_solution_file_path,
        'Release', 'x64', 'libtheora_static',
        build_config.get_msvc_toolset_ver_win32(),
        env=env)

    libraries_win_root = os.path.join(root_project_path, 'Libs/lib_CMake/win')

    lib_path_x86_debug = os.path.join(
        source_folder_path, 'win32/VS2008/Win32/Debug/libtheora_static.lib')
    lib_path_x86_release = os.path.join(
        source_folder_path, 'win32/VS2008/Win32/Release/libtheora_static.lib')
    shutil.copyfile(
        lib_path_x86_debug,
        os.path.join(libraries_win_root, 'x86/Debug/libtheora_win.lib'))
    shutil.copyfile(
        lib_path_x86_release,
        os.path.join(libraries_win_root, 'x86/Release/libtheora_win.lib'))

    lib_path_x64_debug = os.path.join(
        source_folder_path, 'win32/VS2008/x64/Debug/libtheora_static.lib')
    lib_path_x64_release = os.path.join(
        source_folder_path, 'win32/VS2008/x64/Release/libtheora_static.lib')
    shutil.copyfile(
        lib_path_x64_debug,
        os.path.join(libraries_win_root, 'x64/Debug/theora_static_d.lib'))
    shutil.copyfile(
        lib_path_x64_release,
        os.path.join(libraries_win_root, 'x64/Release/theora_static.lib'))

    _copy_headers(source_folder_path, root_project_path)


def _build_win10(working_directory_path, root_project_path):
    prefix = '_win10'
    source_folder_path = _download_and_extract(working_directory_path, prefix)
    _patch_sources(source_folder_path, working_directory_path, prefix)

    vc_solution_file_path = os.path.join(
        source_folder_path, 'win32/VS2008/libtheora_static.sln')

    libogg_include_path = os.path.abspath(os.path.join(
        working_directory_path,
        '../libogg/libogg_source_win10/include'))

    env = build_utils.get_win10_vs_x86_env()
    env['INCLUDE'] = libogg_include_path + ';' + env['INCLUDE']
    build_utils.build_vs(
        vc_solution_file_path,
        'Debug', 'Win32', 'libtheora_static', env=env)
    build_utils.build_vs(
        vc_solution_file_path,
        'Release', 'Win32', 'libtheora_static', env=env)

    env = build_utils.get_win10_vs_x64_env()
    env['INCLUDE'] = libogg_include_path + ';' + env['INCLUDE']
    build_utils.build_vs(
        vc_solution_file_path,
        'Debug', 'x64', 'libtheora_static', env=env)
    build_utils.build_vs(
        vc_solution_file_path,
        'Release', 'x64', 'libtheora_static', env=env)

    env = build_utils.get_win10_vs_arm_env()
    env['INCLUDE'] = libogg_include_path + ';' + env['INCLUDE']
    build_utils.build_vs(
        vc_solution_file_path,
        'Debug', 'ARM', 'libtheora_static', env=env)
    build_utils.build_vs(
        vc_solution_file_path,
        'Release', 'ARM', 'libtheora_static', env=env)

    libraries_win10_root = os.path.join(
        root_project_path, 'Libs/lib_CMake/win10')

    lib_path_x86_debug = os.path.join(
        source_folder_path, 'win32/VS2008/Win32/Debug/libtheora_static.lib')
    lib_path_x86_release = os.path.join(
        source_folder_path, 'win32/VS2008/Win32/Release/libtheora_static.lib')
    shutil.copyfile(
        lib_path_x86_debug,
        os.path.join(
            libraries_win10_root,
            'Win32/Debug/libtheora10_static.lib'))
    shutil.copyfile(
        lib_path_x86_release,
        os.path.join(
            libraries_win10_root,
            'Win32/Release/libtheora10_static.lib'))

    lib_path_x64_debug = os.path.join(
        source_folder_path, 'win32/VS2008/x64/Debug/libtheora_static.lib')
    lib_path_x64_release = os.path.join(
        source_folder_path, 'win32/VS2008/x64/Release/libtheora_static.lib')
    shutil.copyfile(
        lib_path_x64_debug,
        os.path.join(libraries_win10_root, 'x64/Debug/libtheora10_static.lib'))
    shutil.copyfile(
        lib_path_x64_release,
        os.path.join(
            libraries_win10_root,
            'x64/Release/libtheora10_static.lib'))

    lib_path_x64_debug = os.path.join(
        source_folder_path, 'win32/VS2008/ARM/Debug/libtheora_static.lib')
    lib_path_x64_release = os.path.join(
        source_folder_path, 'win32/VS2008/ARM/Release/libtheora_static.lib')
    shutil.copyfile(
        lib_path_x64_debug,
        os.path.join(libraries_win10_root, 'arm/Debug/libtheora10_static.lib'))
    shutil.copyfile(
        lib_path_x64_release,
        os.path.join(
            libraries_win10_root,
            'arm/Release/libtheora10_static.lib'))

    _copy_headers(source_folder_path, root_project_path)


def _build_macos(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    ogg_install_dir_macos = os.path.join(
        working_directory_path, '../libogg/gen/install_macos')
    install_dir_macos = os.path.join(
        working_directory_path, 'gen/install_macos')

    # --disable-asm since it won't compile for x64 otherwise
    # do not postclean since binaries may be used for cross compiling
    build_utils.build_with_autotools(
        source_folder_path,
        ['--with-ogg=' + ogg_install_dir_macos,
         '--disable-asm', '--disable-examples',
         '--host=x86_64-apple-darwin',
         '--disable-shared',
         '--enable-static'],
        install_dir_macos,
        env=build_utils.get_autotools_macos_env(),
        postclean=False)

    shutil.copyfile(
        os.path.join(install_dir_macos, 'lib/libtheora.a'),
        os.path.join(
            root_project_path,
            os.path.join('Libs/lib_CMake/mac/libtheora_macos.a')))

    _copy_headers_from_installation(install_dir_macos, root_project_path)


def _build_ios(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    ogg_install_dir_ios = os.path.join(
        working_directory_path,
        '../libogg/gen/install_ios')
    install_dir_ios = os.path.join(
        working_directory_path,
        'gen/install_ios')
    build_utils.build_with_autotools(
        source_folder_path,
        ['--with-ogg=' + ogg_install_dir_ios,
         '--disable-examples',
         '--host=armv7-apple-darwin',
         '--disable-shared',
         '--enable-static'],
        install_dir_ios,
        env=build_utils.get_autotools_ios_env())

    shutil.copyfile(
        os.path.join(install_dir_ios, 'lib/libtheora.a'),
        os.path.join(
            root_project_path,
            os.path.join('Libs/lib_CMake/ios/libtheora_ios.a')))

    _copy_headers_from_installation(install_dir_ios, root_project_path)

def _build_linux(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    env = build_utils.get_autotools_linux_env()
    ogg_install_dir = os.path.join(working_directory_path, '../libogg/gen/install_linux')
    install_dir = os.path.join(working_directory_path, 'gen/install_linux')

    # --disable-asm since it won't compile for x64 otherwise
    # do not postclean since binaries may be used for cross compiling
    build_utils.build_with_autotools(
        source_folder_path,
        ['--with-ogg=' + ogg_install_dir,
         '--disable-asm',
         '--disable-examples',
         '--disable-shared',
         '--enable-static'],
        install_dir,
        env=env,
        postclean=False)

    shutil.copyfile(os.path.join(install_dir, 'lib/libtheora.a'),
                    os.path.join(root_project_path, 'Libs/lib_CMake/linux/libtheora.a'))

    _copy_headers_from_installation(install_dir, root_project_path)

def _copy_headers(source_folder_path, root_project_path):
    include_path = os.path.join(root_project_path, 'Libs/include/theora')
    build_utils.clean_copy_includes(
        os.path.join(source_folder_path, 'include/theora'), include_path)


def _copy_headers_from_installation(install_dir_path, root_project_path):
    include_path = os.path.join(root_project_path, 'Libs/include/theora')
    build_utils.clean_copy_includes(
        os.path.join(install_dir_path, 'include/theora'),
        include_path)
