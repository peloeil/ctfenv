# syntax=docker/dockerfile:1

FROM ubuntu:24.04
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
    git

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
RUN rm .config/fish/conf.d/abbr.fish
RUN mkdir -p /home/pwn/.config/fish/conf.d \
    && cat << 'EOF' > /home/pwn/.config/fish/conf.d/auto_tmux.fish
if status is-interactive
    if not set -q TMUX
        tmux new-session -A -s pwn
    end
end
EOF

## neovim
RUN fish -c "nvim /home/pwn/.config/nvim/init.lua -c 'sleep 5' -c 'w' -c 'sleep 5' -c 'q'"
RUN fish -c "nvim -c \"call dpp#sync_ext_action('installer', 'install')\" -c 'q'"

CMD ["/bin/fish"]
