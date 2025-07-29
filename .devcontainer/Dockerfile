FROM ubuntu:22.04

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies and ARM toolchain
RUN apt-get update && apt-get install -y \
    gcc-arm-none-eabi \
    cmake \
    make \
    openocd \
    python3 \
    python3-pip \
    doxygen \
    graphviz \
    clang-format \
    clang-tidy \
    cppcheck \
    git \
    curl \
    wget \
    libcanberra-gtk-module \
    ruby-full \
    gdb-multiarch \
    i2c-tools \
    minicom \
    screen \
    netcat-openbsd \
    stlink-tools \
    build-essential \
    libnewlib-arm-none-eabi \
    jq \
    xxd \
    && pip3 install python-can bandit cantools pyserial sphinx breathe

# TODO: See .instructions/stm32h753zi.md for STM32CubeMX integration
# TODO: See .instructions/l6470-driver.md for X-NUCLEO-IHM02A1 SPI configuration  
# TODO: See .instructions/as5600-encoder.md for I2C encoder setup
# TODO: See .instructions/can-fd.md for FDCAN peripheral configuration
# TODO: See .instructions/ethernet-lwip.md for LwIP TCP/IP stack setup
# TODO: See .instructions/ssot-config.md for Single Source of Truth setup

# Install Unity testing framework
RUN git clone https://github.com/ThrowTheSwitch/Unity.git /opt/Unity

# Install GoogleTest
RUN git clone https://github.com/google/googletest.git /opt/googletest && \
    cd /opt/googletest && \
    cmake . && make && make install

# Install Ceedling (Ruby gem for C testing)
RUN gem install ceedling

# Install additional SSOT and documentation tools
RUN pip3 install \
    pre-commit \
    gitpython \
    jinja2 \
    pyyaml \
    jsonschema

# Create a non-root user
RUN useradd -m -s /bin/bash vscode && \
    usermod -aG sudo vscode && \
    echo 'vscode ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

USER vscode
WORKDIR /workspaces/code

# Set environment variables for ARM toolchain
ENV PATH="/usr/bin:${PATH}"
ENV ARM_NONE_EABI_GCC_PATH="/usr/bin"

# TODO: See .instructions/development-setup.md for IDE configuration
# TODO: See .instructions/debugging-setup.md for OpenOCD and GDB configuration
# TODO: See .instructions/ssot-toolchain.md for configuration management tools
