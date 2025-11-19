# syntax=docker/dockerfile:1
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
RUN sed -i 's@archive.ubuntu.com@ftp.jaist.ac.jp/pub/Linux@g' /etc/apt/sources.list
RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    apt-get update && apt-get install -y \
    wget \
    patchelf \
    elfutils

WORKDIR /work

ARG PROBLEM_PATH
ARG BINARY_NAME

COPY --chmod=744 ../../${PROBLEM_PATH}/${BINARY_NAME} /work/chall
COPY ../../${PROBLEM_PATH}/libc.so.6 /work/libc.so.6

RUN wget https://github.com/io12/pwninit/releases/download/3.3.1/pwninit \
    && chmod +x ./pwninit
RUN ./pwninit --bin ./chall

FROM scratch
COPY --from=builder /work/libc.so.6 /libc.so.6
COPY --from=builder /work/ld-* /
