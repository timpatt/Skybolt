import logging
import os
from pathlib import Path
from typing import Optional

from invoke import call, task


logging.basicConfig(level=logging.INFO)

SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_SOURCE_DIR = SCRIPT_DIR.parent.parent

@task
def build_only(context, source_dir: str = str(DEFAULT_SOURCE_DIR), build_config_name: str = "Release", profile_filepath: Optional[str] = None):
    """
    Build Skybolt using conan. This will also build dependencies if required.
    """
    source_dir_path = Path(source_dir).resolve()
    profile_path = Path(profile_filepath).resolve() if profile_filepath else source_dir_path / "Conan/Profiles/BuildEverythingWithPlugins.ini"

    context.run(f"conan remote add skybolt-deps {source_dir_path / 'Conan'} --force")

    build_config_args = "-s skybolt/*:build_type=RelWithDebInfo -s skybolt-widgets/*:build_type=RelWithDebInfo" if build_config_name == "RelWithDebInfo" else ""

    context.run(f"conan build {source_dir_path} -pr:a=default -pr:a={profile_path} --build=missing {build_config_args}")


@task
def docs_only(context, build_dir: str = str(DEFAULT_SOURCE_DIR / "build"), build_config_name: str = "Release"):
    """
    Generates the documentation without running any pre tasks (build etc).
    """
    build_dir_path = Path(build_dir).resolve()

    if os.sys.platform.startswith('linux'):
        # FIXME: build dir is under config folder on Linux. We should update Windows to do this as well.
        build_dir_path = build_dir_path / build_config_name

    # Construct the CMake build command
    cmake_cmd = f"cmake --build {build_dir_path} --config={build_config_name} --target Docs"

    # Detect the operating system and chain the appropriate environment script
    generators_dir_path = build_dir_path / "generators"
    if os.sys.platform.startswith('win32'):
        setup_script = generators_dir_path / "conanrun.bat"
        # Windows cmd.exe uses '&' to chain commands
        full_command = f'"{setup_script}" & {cmake_cmd}'
    else:
        setup_script = generators_dir_path / "conanrun.sh"
        # Linux/Unix uses '.' or 'source' and chains with '&&'
        full_command = f'. "{setup_script}" && {cmake_cmd}'

    context.run(full_command)


@task
def install_only(context, build_dir: str = str(DEFAULT_SOURCE_DIR / "build"), build_config_name: str = "Release"):
    """
    Copy a stand-alone runtime build to an install folder, without running any pre tasks (build etc).
    """
    build_dir_path = Path(build_dir).resolve()

    if os.sys.platform.startswith('linux'):
        # FIXME: build dir is under config folder on Linux. We should update Windows to do this as well.
        build_dir_path = build_dir_path / build_config_name

    components = ["Assets", "Docs", "OsgPlugins", "QtPlugins", "Runtime", "SkyboltDependencies"]
    for component in components:
        context.run(f"cmake --install {build_dir_path} --prefix {build_dir_path}/install --config={build_config_name} --component {component}")


@task
def install(context, source_dir = str(DEFAULT_SOURCE_DIR), build_dir: str = str(DEFAULT_SOURCE_DIR / "build"), build_config_name: str = "Release", profile_filepath: Optional[str] = None):
    """
    Copy a stand-alone runtime build to an install folder.
    """
    build_only(context, source_dir, build_config_name, profile_filepath)
    docs_only(context, build_dir, build_config_name)
    install_only(context, build_dir, build_config_name)