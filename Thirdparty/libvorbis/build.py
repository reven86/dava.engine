import os
import shutil
import build_utils
import build_config


def get_supported_targets(platform):
    if platform == 'win32':
        return ['win32', 'win10']
    elif platform == 'darwin':
        return ['macos']
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
    elif target == 'linux':
        _build_linux(working_directory_path, root_project_path)


def get_download_info():
    return 'http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.5.tar.gz'


def _download_and_extract(working_directory_path, source_folder_prefix=''):
    source_folder_name = 'libvorbis_source' + source_folder_prefix
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

    ogg_include_path = os.path.abspath(
        os.path.join(
            working_directory_path,
            '../libogg/libogg_source/include'))
    vc_solution_file_path = os.path.join(
        source_folder_path, 'win32/VS2010/vorbis_static.sln')

    env = build_utils.get_win32_vs_x86_env()
    env['INCLUDE'] = ogg_include_path + ';' + env['INCLUDE']
    build_utils.build_vs(
        vc_solution_file_path, 'Debug', 'Win32',
        'libvorbis_static', build_config.get_msvc_toolset_ver_win32(), env=env)
    build_utils.build_vs(
        vc_solution_file_path, 'Release', 'Win32',
        'libvorbis_static', build_config.get_msvc_toolset_ver_win32(), env=env)
    build_utils.build_vs(
        vc_solution_file_path, 'Debug', 'Win32',
        'libvorbisfile', build_config.get_msvc_toolset_ver_win32(), env=env)
    build_utils.build_vs(
        vc_solution_file_path, 'Release', 'Win32',
        'libvorbisfile', build_config.get_msvc_toolset_ver_win32(), env=env)

    env = build_utils.get_win32_vs_x64_env()
    env['INCLUDE'] = ogg_include_path + ';' + env['INCLUDE']
    build_utils.build_vs(
        vc_solution_file_path, 'Debug', 'x64',
        'libvorbis_static', build_config.get_msvc_toolset_ver_win32(), env=env)
    build_utils.build_vs(
        vc_solution_file_path, 'Release', 'x64',
        'libvorbis_static', build_config.get_msvc_toolset_ver_win32(), env=env)
    build_utils.build_vs(
        vc_solution_file_path, 'Debug', 'x64',
        'libvorbisfile', build_config.get_msvc_toolset_ver_win32(), env=env)
    build_utils.build_vs(
        vc_solution_file_path, 'Release', 'x64',
        'libvorbisfile', build_config.get_msvc_toolset_ver_win32(), env=env)

    libs_win_root = os.path.join(root_project_path, 'Libs/lib_CMake/win')

    libvorbis_path_x86_debug = os.path.join(
        source_folder_path, 'win32/VS2010/Win32/Debug/libvorbis_static.lib')
    libvorbis_path_x86_release = os.path.join(
        source_folder_path, 'win32/VS2010/Win32/Release/libvorbis_static.lib')
    shutil.copyfile(
        libvorbis_path_x86_debug,
        os.path.join(libs_win_root, 'x86/Debug/libvorbis_static_d.lib'))
    shutil.copyfile(
        libvorbis_path_x86_release,
        os.path.join(libs_win_root, 'x86/Release/libvorbis_static.lib'))

    libvorbisfile_path_x86_debug = os.path.join(
        source_folder_path,
        'win32/VS2010/Win32/Debug/libvorbisfile_static.lib')
    libvorbisfile_path_x86_release = os.path.join(
        source_folder_path,
        'win32/VS2010/Win32/Release/libvorbisfile_static.lib')
    shutil.copyfile(
        libvorbisfile_path_x86_debug,
        os.path.join(
            libs_win_root,
            'x86/Debug/libvorbisfile_static_d.lib'))
    shutil.copyfile(
        libvorbisfile_path_x86_release,
        os.path.join(
            libs_win_root,
            'x86/Release/libvorbisfile_static.lib'))

    libvorbis_path_x64_debug = os.path.join(
        source_folder_path, 'win32/VS2010/x64/Debug/libvorbis_static.lib')
    libvorbis_path_x64_release = os.path.join(
        source_folder_path, 'win32/VS2010/x64/Release/libvorbis_static.lib')
    shutil.copyfile(
        libvorbis_path_x64_debug,
        os.path.join(libs_win_root, 'x64/Debug/libvorbis_static_d.lib'))
    shutil.copyfile(
        libvorbis_path_x64_release,
        os.path.join(libs_win_root, 'x64/Release/libvorbis_static.lib'))

    libvorbisfile_path_x64_debug = os.path.join(
        source_folder_path, 'win32/VS2010/x64/Debug/libvorbisfile_static.lib')
    libvorbisfile_path_x64_release = os.path.join(
        source_folder_path, 'win32/VS2010/x64/Release/libvorbisfile_static.lib')
    shutil.copyfile(
        libvorbisfile_path_x64_debug,
        os.path.join(libs_win_root, 'x64/Debug/libvorbisfile_static_d.lib'))
    shutil.copyfile(
        libvorbisfile_path_x64_release,
        os.path.join(libs_win_root, 'x64/Release/libvorbisfile_static.lib'))

    _copy_headers(source_folder_path, root_project_path)


def _build_win10(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path, '_win10')
    _patch_sources(source_folder_path, working_directory_path, '_win10')

    ogg_include_path = os.path.abspath(
        os.path.join(
            working_directory_path,
            '../libogg/libogg_source_win10/include'))
    vc_solution_file_path = os.path.join(
        source_folder_path, 'win32/VS2010/vorbis_static.sln')

    env = build_utils.get_win10_vs_x86_env()
    env['INCLUDE'] = ogg_include_path + ';' + env['INCLUDE']
    build_utils.build_vs(
        vc_solution_file_path, 'Debug', 'Win32', 'libvorbis_static', env=env)
    build_utils.build_vs(
        vc_solution_file_path, 'Release', 'Win32', 'libvorbis_static', env=env)

    env = build_utils.get_win10_vs_x64_env()
    env['INCLUDE'] = ogg_include_path + ';' + env['INCLUDE']
    build_utils.build_vs(
        vc_solution_file_path, 'Debug', 'x64', 'libvorbis_static', env=env)
    build_utils.build_vs(
        vc_solution_file_path, 'Release', 'x64', 'libvorbis_static', env=env)

    env = build_utils.get_win10_vs_arm_env()
    env['INCLUDE'] = ogg_include_path + ';' + env['INCLUDE']
    build_utils.build_vs(
        vc_solution_file_path, 'Debug', 'ARM', 'libvorbis_static', env=env)
    build_utils.build_vs(
        vc_solution_file_path, 'Release', 'ARM', 'libvorbis_static', env=env)

    libs_win10_root = os.path.join(root_project_path, 'Libs/lib_CMake/win10')

    libvorbis_path_x86_debug = os.path.join(
        source_folder_path, 'win32/VS2010/Win32/Debug/libvorbis_static.lib')
    libvorbis_path_x86_release = os.path.join(
        source_folder_path, 'win32/VS2010/Win32/Release/libvorbis_static.lib')
    shutil.copyfile(
        libvorbis_path_x86_debug,
        os.path.join(libs_win10_root, 'Win32/Debug/libvorbis_static.lib'))
    shutil.copyfile(
        libvorbis_path_x86_release,
        os.path.join(libs_win10_root, 'Win32/Release/libvorbis_static.lib'))

    libvorbis_path_x64_debug = os.path.join(
        source_folder_path, 'win32/VS2010/x64/Debug/libvorbis_static.lib')
    libvorbis_path_x64_release = os.path.join(
        source_folder_path, 'win32/VS2010/x64/Release/libvorbis_static.lib')
    shutil.copyfile(
        libvorbis_path_x64_debug,
        os.path.join(libs_win10_root, 'x64/Debug/libvorbis_static.lib'))
    shutil.copyfile(
        libvorbis_path_x64_release,
        os.path.join(libs_win10_root, 'x64/Release/libvorbis_static.lib'))

    libvorbis_path_arm_debug = os.path.join(
        source_folder_path, 'win32/VS2010/ARM/Debug/libvorbis_static.lib')
    libvorbis_path_arm_release = os.path.join(
        source_folder_path, 'win32/VS2010/ARM/Release/libvorbis_static.lib')
    shutil.copyfile(
        libvorbis_path_arm_debug,
        os.path.join(
            libs_win10_root, 'arm/Debug/libvorbis_static.lib'))
    shutil.copyfile(
        libvorbis_path_arm_release,
        os.path.join(libs_win10_root, 'arm/Release/libvorbis_static.lib'))

    _copy_headers(source_folder_path, root_project_path)


def _build_macos(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    ogg_install_dir_macos = os.path.join(
        working_directory_path, '../libogg/gen/install_macos')
    install_dir_macos = os.path.join(
        working_directory_path, 'gen/install_macos')
    build_utils.build_with_autotools(
        source_folder_path,
        ['--with-ogg=' + ogg_install_dir_macos,
         '--disable-examples',
         '--host=x86_64-apple-darwin',
         '--disable-shared',
         '--enable-static'],
        install_dir_macos,
        env=build_utils.get_autotools_macos_env(),
        postclean=False)

    shutil.copyfile(
        os.path.join(install_dir_macos, 'lib/libvorbis.a'),
        os.path.join(
            root_project_path,
            os.path.join('Libs/lib_CMake/mac/libvorbis_macos.a')))
    shutil.copyfile(
        os.path.join(install_dir_macos, 'lib/libvorbisfile.a'),
        os.path.join(
            root_project_path,
            os.path.join('Libs/lib_CMake/mac/libvorbisfile_macos.a')))

    _copy_headers(install_dir_macos, root_project_path)

def _build_linux(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    env = build_utils.get_autotools_linux_env()
    install_dir = os.path.join(working_directory_path, 'gen/install_linux')
    ogg_install_dir = os.path.join(working_directory_path, '../libogg/gen/install_linux')

    build_utils.build_with_autotools(
        source_folder_path,
        ['--with-ogg=' + ogg_install_dir,
         '--disable-examples',
         '--disable-shared',
         '--enable-static'],
        install_dir,
        env=env,
        postclean=False)

    shutil.copyfile(os.path.join(install_dir, 'lib/libvorbis.a'),
                    os.path.join(root_project_path, 'Libs/lib_CMake/linux/libvorbis.a'))
    shutil.copyfile(os.path.join(install_dir, 'lib/libvorbisfile.a'),
                    os.path.join(root_project_path, 'Libs/lib_CMake/linux/libvorbisfile.a'))

    _copy_headers(install_dir, root_project_path)


def _copy_headers(source_folder_path, root_project_path):
    include_path = os.path.join(root_project_path, 'Sources/External/vorbis')
    build_utils.clean_copy_includes(
        os.path.join(source_folder_path, 'include/vorbis'), include_path)
