import os

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.build import can_run


class SystemCPythonTestPackageConan(ConanFile):
    settings = "os", "arch", "compiler", "build_type"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("system-cpython/1.0.0") 

    def generate(self):
        python_info = self.dependencies["system-cpython"].cpp_info
        toolchain = CMakeToolchain(self)
        toolchain.variables["Python3_EXECUTABLE"] = python_info.get_property("system_cpython_executable")
        toolchain.variables["Python3_INCLUDE_DIR"] = python_info.includedirs[0]
        toolchain.variables["Python3_LIBRARY"] = python_info.get_property("system_cpython_library")
        toolchain.generate()
        CMakeDeps(self).generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        if not can_run(self):
            return

        python_info = self.dependencies["system-cpython"].cpp_info
        executable = python_info.get_property("system_cpython_executable")
        self.run(f'"{executable}" -c "import sys; print(sys.version)"', env="conanrun")
        executable_name = "system_cpython_embedding.exe" if self.settings.os == "Windows" else "system_cpython_embedding"
        binary = os.path.join(self.build_folder, str(self.settings.build_type), executable_name)
        self.run(f'"{binary}"', env="conanrun")
