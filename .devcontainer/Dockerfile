FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

# Install build tools
RUN apt-get update  && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends \
    build-essential cmake cppcheck fontconfig pkg-config valgrind clang lldb llvm gdb \
	&& apt-get autoremove -y && apt-get clean -y && rm -rf /var/lib/apt/lists/*

# Install uv
COPY --from=ghcr.io/astral-sh/uv:0.11.32 /uv /uvx /bin/

# gdb and potentially some other packages might also install python3, but I'd prefer to use a "fixed" and local version of python
# This ensures that uv ignores global instances of python
ENV UV_PYTHON_PREFERENCE=only-managed

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


