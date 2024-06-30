FROM --platform=linux/amd64 ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Tokyo

ARG USERNAME
ARG GROUPNAME
ARG UID
ARG GID
RUN useradd -m -s /bin/bash -G sudo -u ${UID} ${USERNAME}
RUN apt-get update && apt-get install sudo tzdata -y
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

COPY ./install.sh /tmp/install.sh
RUN chmod +x /tmp/install.sh

RUN chown -R ${UID}:${GID} /home/${USERNAME}
USER ${USERNAME}

WORKDIR /tmp
RUN ./install.sh

COPY ./config/.gdbinit /home/${USERNAME}/.gdbinit
COPY ./config/.bashrc /home/${USERNAME}/.bashrc
COPY ./config/.python-version /home/${USERNAME}/.python-version

RUN sudo chown -R ${UID}:${GID} /home/${USERNAME}
WORKDIR /home/${USERNAME}
CMD ["/bin/bash"]
