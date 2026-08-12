
# Skybolt Engine
A C++/Python-based engine for simulating and visualizing dynamic objects such as aircraft, ships and spacecraft, in a 3D planetary environment.

![Skybolt UI](https://prograda.com/wp-content/uploads/2024/10/MainUi-small.jpg)

![Shuttle in space](https://prograda.com/wp-content/uploads/2024/10/Shuttle5-300x170-1.jpg) !["Ship on ocean"](https://prograda.com/wp-content/uploads/2020/11/ShipHeloShot1-300x169.jpg)

## Features
* **Dynamic Simulation:** Real-time 3D object simulation within a geospatial coordinate system.
* **Modular Architecture**: Model dynamic objects using a composition-based entity-component system (ECS) with a configurable property framework.
* **Photoreal Rendering**: Visualize photorealistic environments, including terrain, ocean, urban infrastructure, and atmospheric conditions.
* **Extensible Framework**: Add custom functionality through C++ and Python-based plugins. 
* **Scenario Viewer App**: Interactive 3D application for visualizing scenarios.

## Getting Started
Please refer to the [Getting Started](https://prograda.github.io/Skybolt/getting_started.html) guide in the [Skybolt documentation](https://prograda.github.io/Skybolt/index.html).

## Contact
To submit a bug report, please [raise an issue on the GitHub repository](https://github.com/Prograda/Skybolt/issues).
For general discussion, please [post on our GitHub discussions page](https://github.com/Prograda/Skybolt/discussions).
For business enquiries, please use our [contact form](https://prograda.com/contact).

## License
This project is licensed under the Mozilla Public License Version 2.0 - see the [License.txt](License.txt) file for details.


Tim's notes:
Add export DEBUGINFOD_URLS='' into .bashrc

# Add local repositories
conan remote add skybolt-conan ./Conan # In Skybolt directory
conan remote add archon-conan ./Conan # In Archon directory
uv run conan create . -s build_type=RelWithDebInfo # First in skybolt directory, then in Archon directory
# or
uv run conan build . -s build_type=RelWithDebInfo --build=missing # First in skybolt directory, then in Archon directory

# Install runtime deps
cmake --install build/RelWithDebInfo --config RelWithDebInfo --prefix=$(pwd)/install
cmake --install build/RelWithDebInfo --config RelWithDebInfo --prefix=$(pwd)/install --component QtPlugins
cmake --install build/RelWithDebInfo --config RelWithDebInfo --prefix=$(pwd)/install --component OsgPlugins
cmake --install build/RelWithDebInfo --config RelWithDebInfo --prefix=$(pwd)/install --component SkyboltDependencies

# To run installed app from "bin"
export LD_LIBRARY_PATH=$(pwd):$(pwd)/qtPlugins:$(pwd)/../lib
export SKYBOLT_PLUGINS_PATH="$(pwd)/../lib"
export SKYBOLT_ASSETS_PATH="$(pwd)/../../Assets"



uv run conan install . --deployer=runtime_deploy --deployer-folder=deploy -of=deploy/build --envs-generation=false



## Build unreal plugin on Linux
cd UnrealEngine/Engine/Build/BatchFiles
/RunUAT.sh BuildPlugin -plugin=/workspaces/arkeus/SkyboltUnreal/Plugins/SkyboltUnreal/SkyboltUnreal.uplugin -package=~/SkyboltUnrealPlugin

uv run python3 Tools/BuildScripts/build.py --skybolt-source-dir=$(pwd) --output-dir=$(pwd)/package --stage package

# uv run conan build . --lockfile=conan-shared.lock --build=missing -s build_type=RelWithDebInfo

# Make it accessible as an editable package (removed CMAKE_INSTALL_PREFIX stuff)
conan editable add -of /workspaces/Skybolt/packages/Build .

# Build Archon from archon directory
uv run python3 Tools/BuildScripts/build.py --skybolt-source-dir=$(pwd)/../Skybolt --archon-source-dir=$(pwd) --output-dir=$(pwd)/package --stage package


export SKYBOLT_ASSETS_PATH=/workspaces/Skybolt/Assets

Run ArchonApp

	
export SKYBOLT_PLUGINS_PATH='/workspaces/Skybolt/package/Build/lib:/workspaces/Archon/package/Package/lib'
export SKYBOLT_ASSETS_PATH='/workspaces/Skybolt/Assets:/workspaces/Archon/Assets'
# etc...
. /workspaces/Archon/package/Build/conanrunenv-relwithdebinfo-x86_64.sh



# TODO
* Tests don't work because build context doesn't contain paths to conan dependencies
  * Add DISCOVERY_MODE PRE_TEST to catch_discover_tests to run it in host context just before tests are run.
	* See https://github.com/catchorg/Catch2/issues/2493
* Backup conan dependency sources somewhere (see https://docs.conan.io/2/devops/backup_sources/sources_backup.html)
* Remove `include_package` functionality from conanfile and replace with https://docs.conan.io/2/devops/devops_local_recipes_index.html (potentially)
* Add top-level submodule to capture child packages
* Allow assets to sit in sibling directories
* .gitattributes in SkyboltUnreal is broken.  Need to update and rewrite history
* *.ttf files checked into skybolt need to be added to lfs.  Also remove fonts that we don't have a license for
* "package" now fails in Skybolt; fix it up
* Build UnrealEngine and SkyboltUnrealEngine (with RTTI enabled) and fix any issues
* Assets/Core/Shaders are referred to from SkyboltVisTests/PrincipledBrdfTests.cpp.  Anything required for build should be in the source (?)
* The `MinimalApp` example is way too large!!  It even requires an extra library "ExamplesCommon" to build!?!?
  * I want to be able to do "MinimalApp myApp"; it should be configured with sane defaults, and still be able to configure it as required