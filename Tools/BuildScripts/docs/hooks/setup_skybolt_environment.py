import os
import sys
from pathlib import Path

"""
Sets up DLL search paths to allow the python skybolt python module to be imported in mkdocs.
"""

def setup_skybolt_environment():
	# PATH not automatically added since Python 3.8 and must be added manually
    if sys.platform == "win32":
        print(os.environ["PATH"])
        for path in os.environ["PATH"].split(os.pathsep):
            try:
                if path:
                    path = path.strip('\'"')
                    os.add_dll_directory(path)
            except FileNotFoundError:
                # Skip invalid directories
                pass


def on_config(config, **kwargs):
    setup_skybolt_environment()
    return config