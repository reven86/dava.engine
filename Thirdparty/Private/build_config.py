_vs_12 = '12'
_vs_15 = '15'


# Current variables to use for building libraries
# Change these when migrating over to new compilers, ides etc.
# Functions below are used by build scripts and should check these variables
# to return appropriate values
_win32_vs = _vs_12
_win10_vs = _vs_15


# ========================================================================


def get_gyp_msvs_version():
    if _win32_vs == _vs_12:
        return '2013'  # forces gyp to use v120
    elif _win32_vs == _vs_15:
        return '2015'


def get_msvc_toolset_ver_win32():
    if _win32_vs == _vs_12:
        return 'v120'
    elif _win32_vs == _vs_15:
        return 'v140'


# Get these from registry?
_vc12_path = 'C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC'
_vc15_path = 'C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC'


def get_vs_vc_path_win32():
    if _win32_vs == _vs_12:
        return _vc12_path
    elif _win32_vs == _vs_15:
        return _vc15_path


def _get_vs_vc_path_win10():
    if _win10_vs == _vs_15:
        return _vc15_path


def get_cmake_generator_win32_x86():
    if _win32_vs == _vs_12:
        return 'Visual Studio 12'
    elif _win32_vs == _vs_15:
        return 'Visual Studio 14 2015'


def get_cmake_generator_win32_x64():
    return '{} Win64'.format(get_cmake_generator_win32_x86())


def get_cmake_generator_win10_x86():
    if _win10_vs == _vs_15:
        return 'Visual Studio 14 2015'


def get_cmake_generator_win10_x64():
    return '{} Win64'.format(get_cmake_generator_win10_x86())


def get_cmake_generator_win10_arm():
    return '{} ARM'.format(get_cmake_generator_win10_x86())


def get_cmake_generator_macos():
    return 'Xcode'

def get_cmake_generator_linux():
    return 'Unix Makefiles'
