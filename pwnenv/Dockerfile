# syntax=docker/dockerfile:1

# configurable variables
ARG VERSION='24.04'
ARG HASH='sha256:3afff29dffbc200d202546dc6c4f614edc3b109691e7ab4aa23d02b42ba86790'
ARG USERNAME='pwn'

# main
FROM ubuntu:${VERSION}@${HASH}
ENV DEBIAN_FRONTEND='noninteractive'
ENV TZ='Asia/Tokyo'
RUN sed -i 's@archive.ubuntu.com@ftp.jaist.ac.jp/pub/Linux@g' /etc/apt/sources.list

## install dependencies
RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    apt-get update && apt-get install -y \
    sudo \
    tzdata \
    make \
    gcc \
    wget \
    tmux \
    git \
    fish \
    netcat-openbsd \
    gdb
RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    wget -q https://raw.githubusercontent.com/bata24/gef/dev/install-uv.sh -O- | sh

## add non-root user
RUN id ubuntu && userdel ubuntu || true
ARG USERNAME
RUN groupadd ${USERNAME} -g 1000 && useradd -m ${USERNAME} -s /bin/fish -u 1000 -g 1000 -G sudo
RUN echo "${USERNAME} ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers \
    && echo "FLAG{REDUCTED}" | tee /flag | tee /flag.txt

USER ${USERNAME}
WORKDIR /home/${USERNAME}
RUN touch .sudo_as_admin_successful \
    && mkdir ctf

CMD ["/bin/fish"]
