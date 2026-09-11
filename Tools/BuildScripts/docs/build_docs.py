import argparse
import os
import re
import subprocess as sp
from pathlib import Path
from typing import Sequence


def add_to_path(env, directory: str, prepend: bool = True):
  """Adds a directory to os.environ['PATH'] safely without duplicates."""
  target = os.path.normcase(os.path.abspath(directory))
  paths = [os.path.abspath(p) for p in env.get("PATH", "").split(os.pathsep) if p]
  
  # Filter out the target if it already exists, then insert/append
  filtered = [p for p in paths if os.path.normcase(p) != target]
  updated = [directory] + filtered if prepend else filtered + [directory]
  
  env["PATH"] = os.pathsep.join(updated)

def build(source_dir: Path, skybolt_assets_dir: Path, skybolt_python_module_dirs: Sequence[str], skybolt_binary_dir: str, skybolt_plugin_dirs: Sequence[str], output_dir: Path, deploy_to_github=False):
    python_source_dir = skybolt_assets_dir / "Core/Scripts"

    env = os.environ.copy()
    env["PYTHONPATH"] = os.pathsep.join(
        [str(python_source_dir.resolve())] + [str(Path(d).resolve()) for d in skybolt_python_module_dirs]
    )
    env["SKYBOLT_ASSETS_PATH"] = str(skybolt_assets_dir)
    env["SKYBOLT_PLUGINS_PATH"] = os.pathsep.join(skybolt_plugin_dirs)
    add_to_path(env, str(Path(skybolt_binary_dir).resolve()))

    try:
        command = "gh-deploy" if deploy_to_github else "build"
        commandline = f"mkdocs {command} --site-dir {output_dir} --clean"
        sp.run(commandline, cwd=str(source_dir), env=env, shell=True, check=True)
        #sp.run("python -m griffe dump --no-inspection skybolt", env=env, shell=True, check=True) 
    except RuntimeError as e:
        raise RuntimeError(f"Command '{commandline}' failed", e)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate mkdocs")
    parser.add_argument("--skybolt-source-dir", type=str, help="Directory where skybolt source code is located")
    parser.add_argument("--skybolt-assets-dir", type=str, help="Directory containing skybolt assets")
    parser.add_argument("--skybolt-python-module-dirs", type=str, nargs="+", help="Directories where skybolt python modules are located")
    parser.add_argument("--skybolt-binary-dir", type=str, help="Directory where skybolt binaries are located")
    parser.add_argument("--skybolt-plugin-dirs", type=str, nargs="+", help="Directories where skybolt plugins are located")
    parser.add_argument("--output-dir", type=str, help="Directory to output the docs to")
    parser.add_argument("--deploy-to-github", action="store_true", help="Deploy the docs to github")

    args = parser.parse_args()
    source_dir = Path(args.skybolt_source_dir)
    assets_dir = Path(args.skybolt_assets_dir)
    binary_dir = Path(args.skybolt_binary_dir)
    output_dir = Path(args.output_dir)

    build(
        source_dir=source_dir,
        skybolt_assets_dir=assets_dir,
        skybolt_python_module_dirs=args.skybolt_python_module_dirs,
        skybolt_binary_dir=binary_dir,
        skybolt_plugin_dirs=args.skybolt_plugin_dirs,
        output_dir=output_dir,
        deploy_to_github=args.deploy_to_github)