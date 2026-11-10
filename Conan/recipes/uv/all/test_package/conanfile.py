from io import StringIO

from conan import ConanFile
from conan.errors import ConanException
from conan.tools.build import can_run
from conan.tools.layout import basic_layout

class UvTestPackageConan(ConanFile):
    settings = "os", "arch", "compiler", "build_type"

    def layout(self):
        basic_layout(self)
        
    def requirements(self):
        self.requires("uv/0.13.7")

    def test(self):
        if not can_run(self):
            return

        output = StringIO()
        self.run("uv --version", output, env="conanrun")
        if "uv 0.13.7" not in output.getvalue():
            raise ConanException(f"Unexpected uv version: {output.getvalue().strip()}")
