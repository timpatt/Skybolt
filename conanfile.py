import os

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.cmake.utils import is_multi_configuration
from conan.tools.env import Environment
from pathlib import Path

class SkyboltConan(ConanFile):
    name = "skybolt"
    version = "1.8.0"
    settings = "os", "compiler", "arch", "build_type"
    package_type = "library"

    options = {
        "enable_boost_log": [True, False],
        "enable_bullet": [True, False],
        "enable_docs": [True, False],
        "enable_examples": [True, False],
        "enable_fft_ocean": [True, False],
        "enable_jsbsim": [True, False],
        "enable_map_features_converter": [True, False],
        "enable_osg": [True, False],
        "enable_osg_curl_plugin": [True, False], # Whether to link the OSG curl plugin into the static build. Only relavent if shared = False.
        "enable_python": [True, False],
        "enable_qt": [True, False],
        "python_executable": [None, "ANY"], # system default python installation is used, if None
        "python_version": [None, "ANY"], # major.minor; computed automatically, if None
        "shared": [True, False],
        "shared_plugins": [True, False], # Build plugins as shared libraries.
        "fPIC": [True, False]
    }
    default_options = {
        "enable_boost_log": True,
        "enable_bullet": False,
        "enable_docs": False,
        "enable_examples": True,
        "enable_fft_ocean": True,
        "enable_jsbsim": False,
        "enable_map_features_converter": True,
        "enable_osg": True,
        "enable_osg_curl_plugin": True,
        "enable_python": True,
        "enable_qt": True,
        "python_executable": None,
        "python_version": None,
        "shared": False,
        "shared_plugins": True,
        "fPIC": True
    }
    exports_sources = (
        "CMake/*",
        "src/*",
        ".python-version",
        "CMakeLists.txt",
        "conanfile.py",
        "pyproject.toml",
        "uv.lock"
    )
    no_copy_source = True

    implements = ["auto_shared_fpic"]

    test_requires = "catch2/3.16.1"

    def configure(self):
        if self.options.enable_osg:
            self.options["openscenegraph-mr"].with_curl = True # Required for loading terrain tiles from http sources
        self.options["bullet3"].double_precision = True

        if self.options.get_safe("shared"):
            self.options.rm_safe("fPIC")

    def requirements(self):
        self.requires("boost/1.85.0", transitive_headers=True)
        self.requires("cpp-httplib/0.10.1")
        self.requires("earcut/2.2.3")
        self.requires("glm/0.9.9.8", transitive_headers=True)
        self.requires("nlohmann_json/3.10.5", transitive_headers=True)
        self.requires("fontconfig/2.17.1", override=True) # Transitive dependency to resolve conflict between qt and openscenegraph
		
        self.requires("cxxtimer/1.0.0")
        self.requires("px_sched/1.0.0", transitive_headers=True)
        if self.options.enable_osg:
            self.requires("openscenegraph-mr/3.7.0", transitive_headers=True)
        self.requires("skybolt-reflect/1.1.0", transitive_headers=True)

        if self.options.enable_bullet:
            self.requires("bullet3/3.22a")

        if self.options.enable_fft_ocean:
            self.requires("mufft/1.0.0")
            self.requires("xsimd/7.4.10", transitive_headers=True)

        if self.options.enable_jsbsim:
            self.requires("jsbsim/1.1.13")

        if self.options.enable_map_features_converter:
            self.requires("readosm/1.1.0a")

        if self.options.enable_python:
            python_options = {}
            if self.options.python_executable:
                python_options["python_executable"] = self.options.python_executable
            if self.options.python_version:
                python_options["python_version"] = self.options.python_version
            self.requires("system-cpython/1.0.0", options=python_options)
            self.requires("pybind11/2.13.6")
            
        if self.options.enable_qt:
            self.requires("qt/6.10.1", transitive_headers=True)
            self.requires("skybolt-widgets/1.1.0", transitive_headers=True)

    @staticmethod
    def _is_unset_option(value):
        return value is None or str(value).strip().lower() in ("", "none")

    def validate(self):
        if not self.options.enable_python:
            return

        dependency_options = self.dependencies["system-cpython"].options
        for option_name in ("python_executable", "python_version"):
            skybolt_value = self.options.get_safe(option_name)
            system_python_value = dependency_options.get_safe(option_name)
            if self._is_unset_option(skybolt_value) or self._is_unset_option(system_python_value):
                continue

            if option_name == "python_executable":
                skybolt_value = str(Path(str(skybolt_value)).expanduser().resolve()).casefold()
                system_python_value = str(Path(str(system_python_value)).expanduser().resolve()).casefold()
            else:
                skybolt_value = str(skybolt_value)
                system_python_value = str(system_python_value)

            if skybolt_value != system_python_value:
                raise ConanInvalidConfiguration(
                    f"Conflicting {option_name} values were selected for skybolt "
                    f"('{skybolt_value}') and system-cpython ('{system_python_value}')."
                )

    def package_id(self):
        del self.info.options.python_executable  # PATH to the interpreter is not important, only version matters
        if not self.info.options.enable_python:
            del self.info.options.python_version

    def _is_multi_configuration(self) -> bool:
        if self.conf.get("tools.cmake.cmaketoolchain:generator") is None:
            compiler = self.settings.get_safe("compiler")
            if compiler == "msvc":
                return True
            else:
                return False

        return is_multi_configuration(self.conf.get("tools.cmake.cmaketoolchain:generator"))

    def _build_type_path_if_multi_generator(self) -> bool:
        return f"{self.settings.build_type}" if self._is_multi_configuration() else ""

    def layout(self):
        cmake_layout(self)
        self.cpp.source.includedirs = ["src"]
        self.cpp.source.builddirs = ["CMake"]
        
        _extra_path = self._build_type_path_if_multi_generator()
        self.cpp.build.libdirs = [f"lib/{_extra_path}", f"lib/{_extra_path}/plugins"]
        self.cpp.build.bindirs = [f"bin/{_extra_path}", f"bin/{_extra_path}/plugins"]
        self.cpp.build.set_property("cmake_build_modules", [f"conan-skybolt-init-buildtree.cmake"])

    def generate(self):
        env = Environment()

        # Make sure the SkyboltEnginePlugins can find the Skybolt plugins at runtime on Windows, 
        # even if they are built as shared libraries
        _extra_path = self._build_type_path_if_multi_generator()
        env.prepend_path("PATH", os.path.join(self.build_folder, "bin", _extra_path, "plugins"))

        tc = CMakeToolchain(self)
        # FIXME: We shouldn't be configuring flags for dependencies here; that should be done by the dependency itself
        tc.variables["Boost_STATIC_LIBS"] = bool(not self.dependencies["boost"].options.shared)
        if self.options.enable_osg:
            tc.variables["OSG_STATIC_LIBS"] = bool(not self.dependencies["openscenegraph-mr"].options.shared)
        tc.variables["SKYBOLT_PLUGINS_STATIC_BUILD"] = bool(not self.options.shared_plugins)
        tc.variables["Skybolt_VERSION"] = self.version

        # FIXME: These variables should be prefixed with "SKYBOLT_"
        tc.variables["BUILD_JSBSIM_PLUGIN"] = bool(self.options.enable_jsbsim)
        tc.variables["BUILD_BULLET_PLUGIN"] = bool(self.options.enable_bullet)
        tc.variables["BUILD_FFT_OCEAN_PLUGIN"] = bool(self.options.enable_fft_ocean)
        tc.variables["BUILD_MAP_FEATURES_CONVERTER"] = bool(self.options.enable_map_features_converter)
        tc.variables["BUILD_PYTHON_BINDINGS"] = bool(self.options.enable_python)
        tc.variables["BUILD_PYTHON_PLUGIN"] = bool(self.options.enable_python)
        tc.variables["BUILD_WITH_BOOST_LOG"] = bool(self.options.enable_boost_log)
        tc.variables["BUILD_WITH_QT"] = bool(self.options.enable_qt)
        tc.variables["BUILD_WITH_OSG"] = bool(self.options.enable_osg)
        tc.variables["BUILD_WITH_OSG_CURL_PLUGIN"] = bool(self.options.enable_osg_curl_plugin)
        
        if self.options.enable_python:
            python_info = self.dependencies["system-cpython"].cpp_info
            tc.variables["Python3_EXECUTABLE"] = python_info.get_property("system_cpython_executable")
            tc.variables["Python3_INCLUDE_DIR"] = python_info.includedirs[0]
            tc.variables["Python3_LIBRARY"] = python_info.get_property("system_cpython_library")

            # Make sure we can find the Python interpreter and libraries at runtime, 
            # even if they are not installed system-wide
            env.prepend_path("PATH", python_info.get_property("system_cpython_runtime_dir"))
            env.define("SKYBOLT_PYTHON_HOME", python_info.get_property("system_cpython_python_home"))

        tc.variables["SKYBOLT_BUILD_DOCS"] = bool(self.options.enable_docs)
        tc.variables["SKYBOLT_BUILD_EXAMPLES"] = bool(self.options.enable_examples)

        # catch_discover_tests should run just prior to tests as part of the host context to 
        # ensure the runtime dependencies are available to it.
        tc.variables["CMAKE_CATCH_DISCOVER_TESTS_DISCOVERY_MODE"] = "PRE_TEST"

        tc.generate()

        env.vars(self, scope="run").save_script("skybolt_extra_paths")

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        # Use `tools.build:skip_test=True` configuration option to skip tests
        cmake.ctest()
        
    def package(self):
        cmake = CMake(self)
        # Install Skybolt binaries (excluding dependencies)
        cmake.install()
        # Install development files (headers, etc)
        cmake.install(component="Development")

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        
        # When building static libraries, the order of the libs matters; SkyboltEngineQt depends on SkyboltVis
        # The linker walks left to right, and collates items that are required and expects them to be satisfied later on
        if self.options.enable_fft_ocean:
            self.cpp_info.libs.append("FftOcean")

        if self.options.enable_bullet:
            self.cpp_info.libs.append("SkyboltBullet")

        if self.options.enable_qt:
            self.cpp_info.libs.append("SkyboltEngineQt")

        if self.options.enable_osg:
            self.cpp_info.libs.append("SkyboltVisOsg")

        self.cpp_info.libs.extend(["SkyboltEngine", "AircraftHud", "SkyboltVis", "SkyboltSim", "SkyboltCommon"])

        self.cpp_info.builddirs = ["CMake"]
        self.cpp_info.set_property("cmake_build_modules", ["CMake/conan-skybolt-init-installtree.cmake"])