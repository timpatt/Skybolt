import os

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.files import copy, get


class UvConan(ConanFile):
    name = "uv"
    package_type = "application"
    settings = "os", "arch"

    def build(self):
        os_name = str(self.settings.os)
        arch = str(self.settings.arch)
        try:
            source = self.conan_data["sources"][self.version][os_name][arch]
        except KeyError as exc:
            raise ConanInvalidConfiguration(
                f"uv {self.version} does not provide a binary for {os_name}/{arch}."
            ) from exc

        get(
            self,
            url=(
                f"https://github.com/astral-sh/uv/releases/download/"
                f"{self.version}/{source['filename']}"
            ),
            sha256=source["sha256"],
            strip_root=self.settings.os != "Windows",
        )

    def package(self):
        executables = ("uv.exe", "uvx.exe", "uvw.exe") if self.settings.os == "Windows" else ("uv", "uvx")
        for executable in executables:
            copy(
                self,
                pattern=executable,
                src=self.build_folder,
                dst=os.path.join(self.package_folder, "bin"),
                keep_path=False,
            )

    def package_info(self):
        bin_dir = os.path.join(self.package_folder, "bin")
        self.cpp_info.bindirs = ["bin"]
        self.buildenv_info.prepend_path("PATH", bin_dir)
        self.runenv_info.prepend_path("PATH", bin_dir)
        self.cpp_info.set_property(
            "uv_executable", os.path.join(bin_dir, "uv.exe" if self.settings.os == "Windows" else "uv")
        )
