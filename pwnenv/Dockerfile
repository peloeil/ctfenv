FROM --platform=linux/amd64 ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

ARG USERNAME
ARG GROUPNAME
ARG UID
ARG GID
RUN useradd -m -s /bin/bash -G sudo -u $UID $USERNAME \
    && echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
RUN apt update && apt install sudo tzdata -y

COPY ./config/.gdbinit /home/$USERNAME/.gdbinit
COPY ./config/.bashrc /home/$USERNAME/.bashrc
COPY ./install.sh /tmp/install.sh
RUN chmod +x /tmp/install.sh

RUN chown -R $UID:$GID /home/$USERNAME
USER $USERNAME

WORKDIR /tmp
RUN ./install.sh

WORKDIR /home/$USERNAME
CMD ["/bin/bash"]
