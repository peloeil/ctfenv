# syntax=docker/dockerfile:1

ARG VERSION='24.04'

FROM ubuntu:${VERSION}
ENV DEBIAN_FRONTEND='noninteractive'
ENV TZ='Asia/Tokyo'
RUN sed -i 's@archive.ubuntu.com@ftp.jaist.ac.jp/pub/Linux@g' /etc/apt/sources.list

## prerequisites
RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    apt-get update && apt-get install -y \
    build-essential \
    sudo \
    tzdata \
    wget \
    curl \
    git \
    patchelf \
    elfutils

## non-root user
RUN id ubuntu && userdel ubuntu || true; \
    groupadd pwn -g 1000; \
    useradd -m pwn -s /bin/fish -u 1000 -g 1000 -G sudo; \
    echo "pwn ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers; \
    touch /home/pwn/.sudo_as_admin_successful

## bata24/gef
RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    apt-get update && apt-get install -y \
    binutils \
    python3-dev \
    ruby-dev \
    file \
    colordiff \
    imagemagick \
    linux-tools-common \
    && wget -q https://raw.githubusercontent.com/bata24/gef/dev/install-uv.sh -O- | sh

## gef without root previlege
RUN cp /root/.gef/.venv-gef/bin/rp-lin /root/.gef/.venv-gef/bin/rp++ \
    && cp -r /root/.gef /home/pwn/.gef \
    && cp /root/.gdbinit /home/pwn/.gdbinit \
    && mkdir -p /home/pwn/.local \
    && cp -r /root/.local/bin /home/pwn/.local/bin \
    && sed -i -e "s|/root/.gef|/home/pwn/.gef|" /home/pwn/.gdbinit \
    && find /home/pwn/.gef -type f -exec sed -i "s|/root/\.gef|/home/pwn/.gef|g" {} + \
    && chown -R pwn:pwn /home/pwn/.gef \
    && chown pwn:pwn /home/pwn/.gdbinit \
    && chown -R pwn:pwn /home/pwn/.local

## exploit tools
RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    apt-get update && apt-get install -y \
    fish \
    tmux \
    netcat-openbsd \
    gdb

## switch user
USER pwn
WORKDIR /home/pwn

## mise
ENV PATH="/home/pwn/.local/bin:$PATH"
RUN curl https://mise.run | sh
RUN git clone https://github.com/peloeil/dotfiles.git \
    && mkdir -p ~/.config \
    && cp -r dotfiles/dot_config/mise ~/.config/mise
RUN mise install uv ripgrep node deno neovim ruff "npm:pyright"
RUN cp -r dotfiles/dot_config/nvim ~/.config/nvim \
    && cp -r dotfiles/dot_config/tmux ~/.config/tmux \
    && cp -r dotfiles/dot_config/private_fish ~/.config/fish \
    && rm -rf dotfiles

## fish
RUN echo "fish_add_path /home/pwn/.gef/.venv-gef/bin" >> .config/fish/config.fish

## neovim
RUN fish -c "nvim /home/pwn/.config/nvim/init.lua -c 'sleep 5' -c 'w' -c 'sleep 5' -c 'q'"
RUN fish -c "nvim -c \"call dpp#sync_ext_action('installer', 'install')\" -c 'q'"

ARG PROBLEM_PATH
ARG BINARY_NAME

USER root

COPY ../../${PROBLEM_PATH}/libc.so.6 /lib/x86_64-linux-gnu/libc.so.6
COPY ../../${PROBLEM_PATH}/ld-linux-x86-64.so.2 /lib64/ld-linux-x86-64.so.2
COPY --chown=pwn:pwn --chmod=744 ../../${PROBLEM_PATH}/${BINARY_NAME} /home/pwn/${BINARY_NAME}
COPY --chown=pwn:pwn --chmod=744 ../../${PROBLEM_PATH}/attach.sh /home/pwn/attach.sh
COPY --chown=pwn:pwn ../../${PROBLEM_PATH}/gdb.py /home/pwn/gdb.py

RUN ldconfig

USER pwn
WORKDIR /home/pwn
CMD ["/bin/fish"]
