FROM debian:12-slim

ARG TERM=vt100

ARG USERNAME=rddev
ARG USER_UID=1000
ARG USER_GID=1001

# 1) Apt dependencies and Pillow (PIL)
# 2) Container user setup
# Add the user to the audio group, to have ALSA sound access
# TODO: pulseaudio, so this isn't necessary?
RUN apt -y update && apt -y upgrade && \
    apt -y install wget procps python3-pillow \
                   gcc make cmake pkg-config git python3 gzip dpkg \
                   libsdl2-dev libsdl2-mixer-dev libsdl2-net-dev libsamplerate-dev && \
    apt -y autoremove && \
    \
    groupadd --gid $USER_GID $USERNAME && \
    useradd --uid $USER_UID --gid $USER_GID -m $USERNAME && \
    adduser $USERNAME audio && \
    chsh $USERNAME -s /bin/bash && \
    mkdir -p /home/$USERNAME/.local/share && \
    chown -R $USERNAME:$USERNAME /home/$USERNAME/.local

USER $USERNAME
