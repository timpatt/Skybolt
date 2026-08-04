
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

uv run python3 Tools/BuildScripts/build.py --skybolt-source-dir=$(pwd) --output-dir=$(pwd)/package --stage package

# Make it accessible as an editable package (removed CMAKE_INSTALL_PREFIX stuff)
conan editable add -of /workspaces/Skybolt/packages/Build .

# Build Archon from archon directory
uv run python3 Tools/BuildScripts/build.py --skybolt-source-dir=/workspaces/Skybolt --archon-source-dir=/workspaces/Archon --output-dir=/workspaces/Archon/package --stage package


export SKYBOLT_ASSETS_PATH=/workspaces/Skybolt/Assets

Run ArchonApp

* Tests don't work because build context doesn't contain paths to conan dependencies
  * Add DISCOVERY_MODE PRE_TEST to catch_discover_tests to run it in host context just before tests are run.
	* See https://github.com/catchorg/Catch2/issues/2493
	
	
export SKYBOLT_PLUGINS_PATH='/workspaces/Skybolt/package/Build/lib:/workspaces/Archon/package/Package/lib'
export SKYBOLT_ASSETS_PATH='/workspaces/Skybolt/Assets:/workspaces/Archon/Assets'
# etc...
. /workspaces/Archon/package/Build/conanrunenv-relwithdebinfo-x86_64.sh

