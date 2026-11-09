import json
import os
import platform
import shutil
import sys
from io import StringIO
from pathlib import Path
from typing import Any, Union

from conan import ConanFile
from conan.errors import ConanException, ConanInvalidConfiguration
from conan.tools.build import cross_building


class SystemCPythonConan(ConanFile):
    name = "system-cpython"
    version = "1.0.0"
    package_type = "library"
    settings = "os", "arch"

    options = {
        "python_executable": [None, "ANY"],
        "python_version": [None, "ANY"],
        "abi_tag": [None, "ANY"],
        "debug_abi": [None, True, False],
        "shared": [True, False],
    }
    default_options = {
        "python_executable": None,
        "python_version": None,
        "abi_tag": None,
        "debug_abi": None,
        "shared": True,
    }

    _architecture_aliases = {
        "x86_64": ("x86_64", "amd64", "x64"),
        "x86": ("x86", "i386", "i486", "i586", "i686"),
        "armv8": ("arm64", "aarch64"),
        "armv7": ("armv7", "armv7l"),
    }

    def init(self):
        self._python_info = None

    @staticmethod
    def _normalized_path(path: Union[Path, str]):
        return str(Path(path).resolve()).replace("\\", "/")

    @property
    def python_executable(self):
        executable = self.options.python_executable or sys.executable
        executable = str(executable)
        resolved = shutil.which(executable) or executable
        return self._normalized_path(Path(resolved).expanduser())

    def _probe_python(self):
        if self._python_info is not None:
            return self._python_info

        script = (
            "import json, sys, sysconfig; "
            "import platform; "
            "print(json.dumps({"
            "'version': [sys.version_info[0], sys.version_info[1]], "
            "'prefix': sys.prefix, "
            "'base_prefix': sys.base_prefix, "
            "'machine': platform.machine(), "
            "'paths': sysconfig.get_paths(), "
            "'config_vars': {key: sysconfig.get_config_var(key) for key in "
            "('LIBDIR', 'LDLIBRARY', 'LIBRARY', 'INSTSONAME', 'Py_ENABLE_SHARED', "
            "'BINDIR', 'ABIFLAGS', 'LDVERSION', 'MULTIARCH', 'Py_DEBUG', 'SOABI')}"
            "}))"
        )
        output = StringIO()
        try:
            self.run(f'"{self.python_executable}" -c "{script}"', output, scope="run")
        except ConanException as exc:
            raise ConanInvalidConfiguration(
                f"Unable to run the selected Python executable '{self.python_executable}': {exc}"
            ) from exc

        lines = [line.strip() for line in output.getvalue().splitlines() if line.strip()]
        try:
            self._python_info = json.loads(lines[-1])
            version = self._python_info["version"]
            self._python_info["version"] = f"{version[0]}.{version[1]}"
        except (IndexError, KeyError, TypeError, ValueError, json.JSONDecodeError) as exc:
            raise ConanInvalidConfiguration(
                f"Python executable '{self.python_executable}' returned invalid sysconfig data."
            ) from exc

        self._validate_python_info(self._python_info)
        return self._python_info

    def _validate_python_info(self, python_info):
        executable = Path(self.python_executable)
        if not executable.is_file():
            raise ConanInvalidConfiguration(
                f"Python executable does not exist: '{self.python_executable}'."
            )
        if self.settings.os == "Linux" and not os.access(executable, os.X_OK):
            raise ConanInvalidConfiguration(
                f"Python executable is not executable: '{self.python_executable}'."
            )

        expected_arch = str(self.settings.arch)
        machine = str(python_info.get("machine", "")).lower()
        aliases = self._architecture_aliases.get(expected_arch)
        if aliases is None or machine not in aliases:
            raise ConanInvalidConfiguration(
                f"Python at '{self.python_executable}' reports architecture '{machine}', "
                f"which is incompatible with Conan architecture '{expected_arch}'."
            )

        paths = python_info.get("paths", {})
        include_candidates = []
        for path_key in ("include", "platinclude"):
            include_dir = paths.get(path_key)
            if include_dir:
                include_candidates.append(Path(include_dir) / "Python.h")
        python_header = next((path for path in include_candidates if path.is_file()), None)
        if python_header is None:
            raise ConanInvalidConfiguration(
                f"Python {python_info['version']} at '{self.python_executable}' does not have "
                "the required development header Python.h."
            )

        base_prefix = Path(python_info["base_prefix"])
        config_vars = python_info.get("config_vars", {})
        library_candidates = []
        if self.settings.os == "Windows":
            version = python_info["version"].replace(".", "")
            debug_build = bool(config_vars.get("Py_DEBUG"))
            library_suffix = "_d" if debug_build else ""
            library_names = [f"python{version}{library_suffix}.lib"]
            for directory in (config_vars.get("LIBDIR"), str(base_prefix / "libs")):
                if directory:
                    library_candidates.extend(Path(directory) / name for name in library_names)
            runtime_dll = base_prefix / f"python{version}{library_suffix}.dll"
        else:
            library_dir = config_vars.get("LIBDIR")
            names = [
                config_vars.get("LDLIBRARY"),
                config_vars.get("INSTSONAME"),
                config_vars.get("LIBRARY"),
            ]
            if library_dir:
                library_candidates.extend(
                    Path(library_dir) / name for name in names if name
                )
            runtime_dll = None

        python_library = next((path for path in library_candidates if path.is_file()), None)
        if python_library is None:
            candidates = ", ".join(str(path) for path in library_candidates) or "none reported"
            raise ConanInvalidConfiguration(
                f"Python {python_info['version']} at '{self.python_executable}' does not have "
                f"a usable embedding library (checked: {candidates})."
            )

        python_info["abi_tag"] = config_vars.get("SOABI") or config_vars.get("ABIFLAGS") or "default"
        python_info["debug_abi"] = bool(config_vars.get("Py_DEBUG"))
        python_info["include_dir"] = self._normalized_path(python_header.parent)
        python_info["library"] = self._normalized_path(python_library)
        python_info["library_dir"] = self._normalized_path(python_library.parent)

        # Windows doesn't currently support static libraries for Python embedding, so we can 
        # assume that if the library is a .a file, it is static.
        python_info["shared"] = not python_library.name.lower().endswith(".a")
        binary_dirs = [executable.parent]
        if config_vars.get("BINDIR"):
            binary_dirs.append(Path(config_vars["BINDIR"]))
        python_info["prefix_dir"] = self._normalized_path(python_info["prefix"])
        python_info["binary_dirs"] = list(dict.fromkeys(self._normalized_path(path) for path in binary_dirs))
        python_info["runtime_dir"] = self._normalized_path(runtime_dll.parent if runtime_dll else executable.parent)
        python_info["runtime_library"] = self._normalized_path(runtime_dll) if runtime_dll else None
        if runtime_dll and not runtime_dll.is_file():
            raise ConanInvalidConfiguration(
                f"Python runtime DLL was not found at '{self._normalized_path(runtime_dll)}'."
            )

    def configure(self):
        if str(self.settings.os) not in ("Windows", "Linux"):
            raise ConanInvalidConfiguration(
                f"system-python supports Windows and Linux, not '{self.settings.os}'."
            )
        if cross_building(self):
            raise ConanInvalidConfiguration(
                "system-python cannot use a local interpreter when cross-building."
            )

        python_info = self._probe_python()
        self._apply_detected_option("python_version", python_info["version"])
        self._apply_detected_option("abi_tag", python_info["abi_tag"])
        self._apply_detected_option("debug_abi", python_info["debug_abi"])
        self._apply_detected_option("shared", python_info["shared"])

    def _apply_detected_option(self, option_name: str, detected_value: Any):
        requested = getattr(self.options, option_name)
        if requested.value is not None and str(requested) != str(detected_value):
            raise ConanInvalidConfiguration(
                f"Python at '{self.python_executable}' has {option_name} '{detected_value}', "
                f"which does not match the requested {option_name} '{requested}'."
            )
        setattr(self.options, option_name, detected_value)

    def package_id(self):
        # The interpreter location does not affect binary compatibility; the version, ABI and
        # linkage are captured by the python_version, abi_tag, debug_abi and shared options.
        del self.info.options.python_executable

    def package_info(self):
        python_info = self._probe_python()
        library_filename = Path(python_info["library"]).name
        library_name = library_filename.split(".so", 1)[0] if ".so" in library_filename else Path(library_filename).stem
        if library_name.startswith("lib"):
            library_name = library_name[3:]

        self.cpp_info.includedirs = [python_info["include_dir"]]
        self.cpp_info.libdirs = [python_info["library_dir"]]
        self.cpp_info.bindirs = python_info["binary_dirs"]
        self.cpp_info.libs = [library_name]

        self.cpp_info.set_property("system_cpython_executable", self.python_executable)
        self.cpp_info.set_property("system_cpython_version", python_info["version"])
        self.cpp_info.set_property("system_cpython_library", python_info["library"])
        self.cpp_info.set_property("system_cpython_runtime_dir", python_info["runtime_dir"])
        self.cpp_info.set_property("system_cpython_python_home", python_info["prefix_dir"])

        print("system_cpython_bin_dirs" + ', '.join(python_info["binary_dirs"]))
        print("system_cpython_runtime_dir" + python_info["runtime_dir"])

        for env_info in (self.buildenv_info, self.runenv_info):
            for binary_dir in python_info["binary_dirs"]:
                env_info.prepend_path("PATH", binary_dir)
            if self.settings.os == "Windows":
                env_info.prepend_path("PATH", python_info["runtime_dir"])
            else:
                env_info.prepend_path("LD_LIBRARY_PATH", python_info["library_dir"])
