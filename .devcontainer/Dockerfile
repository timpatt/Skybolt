FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

# Install build tools
RUN apt-get update  && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends \
    build-essential cmake cppcheck git git-lfs fontconfig pkg-config valgrind clang lldb llvm gdb \
	&& apt-get autoremove -y && apt-get clean -y && rm -rf /var/lib/apt/lists/*

# Install uv
COPY --from=ghcr.io/astral-sh/uv:0.11.32 /uv /uvx /bin/

# gdb and potentially some other packages might also install python3, but I'd prefer to use a "fixed" and local version of python
# This ensures that uv ignores global instances of python
ENV UV_PYTHON_PREFERENCE=only-managed

# Install skybolt dependencies
RUN apt-get update  && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends \
    libgl-dev `# opengl/system` \
    xkb-data `# xkeyboard-config/system` \
    libx11-xcb-dev libfontenc-dev libice-dev libsm-dev libxaw7-dev libxcomposite-dev libxcursor-dev libxdamage-dev libxext-dev libxfixes-dev libxi-dev libxinerama-dev libxkbfile-dev libxmu-dev libxmuu-dev libxpm-dev libxrandr-dev libxrender-dev libxres-dev libxss-dev libxt-dev libxtst-dev libxv-dev libxxf86vm-dev libxcb-glx0-dev libxcb-render0-dev libxcb-render-util0-dev libxcb-xkb-dev libxcb-icccm4-dev libxcb-image0-dev libxcb-keysyms1-dev libxcb-randr0-dev libxcb-shape0-dev libxcb-sync-dev libxcb-xfixes0-dev libxcb-xinerama0-dev libxcb-dri3-dev uuid-dev libxcb-cursor-dev libxcb-dri2-0-dev libxcb-dri3-dev libxcb-present-dev libxcb-composite0-dev libxcb-ewmh-dev libxcb-res0-dev libxcb-util-dev `# xorg/system` \
	&& apt-get autoremove -y && apt-get clean -y && rm -rf /var/lib/apt/lists/*


# Disable downloading debug info globally
# gdb can appear hung while attempting/failing to download debug symbols on launch (https://github.com/microsoft/MIEngine/issues/1569).
# See also .vscode/launch.json 
COPY zz-disable-debuginfod.sh /etc/profile.d/

#USER $USERNAME

# Make sure directory that uv uses to install python and tools to is available on the path
#ENV PATH="/home/$USERNAME/.local/bin:$PATH"

# Configure default user environment
# Install python
#RUN uv python install --default 3.11
#WORKDIR /home/$USERNAME
#COPY Tools/BuildScripts/requirements.txt /tmp/requirements.txt

#COPY --chmod=755 init.sh /tmp/init.sh
#COPY Tools/BuildScripts/requirements.txt /tmp/requirements.txt
#COPY Tools/BuildScripts/docs/requirements.txt /tmp/docs/requirements.txt

#RUN /tmp/init.sh


# Default command to run
CMD ["/bin/bash"]


