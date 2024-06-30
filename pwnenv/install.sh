#!/bin/bash

# update and install packages
sudo apt-get update && sudo apt-get install -y \
    gcc \
    gdb \
    vim \
    curl \
    unzip \
    git \
    netcat \
    patchelf

# editor
## neovim@0.10.0
curl -sL https://github.com/neovim/neovim/releases/download/v0.10.0/nvim-linux64.tar.gz -o /tmp/nvim-linux64.tar.gz
tar -xvf /tmp/nvim-linux64.tar.gz --directory=/tmp
sudo cp /tmp/nvim-linux64/bin/nvim /usr/bin
sudo cp -r /tmp/nvim-linux64/share/nvim /usr/share
rm /tmp/nvim-linux64.tar.gz
rm -r /tmp/nvim-linux64

## astronvim
git clone --depth 1 https://github.com/AstroNvim/template ${HOME}/.config/nvim
rm -rf ${HOME}/.config/nvim/.git

# exploit tools
## rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
source ${HOME}/.cargo/env
## ropr@0.2.25
cargo install ropr@0.2.25

## python@3.12.0
curl -sSf https://rye.astral.sh/get | RYE_INSTALL_OPTION="--yes" bash
source ${HOME}/.rye/env
rye toolchain fetch cpython@3.12.0
## pwntools@4.12.0, ptrlib@2.3.3
rye install pwntools==4.12.0
rye install ptrlib==2.3.3

## rp++@2.1.3
curl -sL https://github.com/0vercl0k/rp/releases/download/v2.1.3/rp-lin-gcc.zip -o /tmp/rp++.zip
unzip /tmp/rp++.zip -d /tmp
sudo cp /tmp/rp-lin /usr/local/bin/rp++
sudo chmod +x /usr/local/bin/rp++
rm /tmp/rp++.zip
rm /tmp/rp-lin

## one_gadget (requires ruby)


# debug tools
TOOLS_DIR="${HOME}/tools"
## radare2@5.9.2
git clone --depth 1 https://github.com/radareorg/radare2 --branch 5.9.2 ${TOOLS_DIR}/radare2
cd ${TOOLS_DIR}/radare2 && ./sys/install.sh

## pwndbg@2024.02.14
git clone --depth 1 https://github.com/pwndbg/pwndbg --branch 2024.02.14 ${TOOLS_DIR}/pwndbg
cd ${TOOLS_DIR}/pwndbg && DEBIAN_FRONTEND=noninteractive ./setup.sh --update

## gef@2024.06
curl -sL https://raw.githubusercontent.com/hugsy/gef/2024.06/gef.py -o ${TOOLS_DIR}/.gdbinit-gef.py

# clean up
sudo apt-get clean
sudo rm -rf /var/lib/apt/lists/*
