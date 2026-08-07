#!/bin/bash
# The root of any newly-created named volumes is always owned by root:root.
# Update it to the correct uid:gid.  Only really required after volume creation.
sudo chown -R vscode:vscode /home/vscode/.conan2
# uv caches stuff in here
sudo chown -R vscode:vscode /home/vscode/.cache

# FIXME: These get installed *every* time we start the container which is a bit annoying...
uv tool install conan==2.31.1
uv tool install mkdocs==1.6.1 --with mkdocs-material==9.7.7 --with 'mkdocstrings[python]==1.0.6'

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

mkdir -p $HOME/.conan2/profiles
cp $SCRIPT_DIR/conan/ubuntu $HOME/.conan2/profiles/default
