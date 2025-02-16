FROM fedora:latest

ARG TERM=vt100

ARG USERNAME=rddev
ARG USER_UID=1000
ARG USER_GID=1001

# 1) Apt dependencies and Pillow (PIL)
# 2) Container user setup
# Add the user to the audio group, to have ALSA sound access
# TODO: pulseaudio, so this isn't necessary?
RUN dnf -y update && dnf -y upgrade && \
    dnf -y install wget procps python3-pillow \
                   gcc make cmake pkg-config git python3 gzip rpmbuild \
                   SDL2-devel SDL2_mixer-devel SDL2_net-devel libsamplerate-devel miniz-devel && \
    dnf -y autoremove && \
    \
    groupadd --gid $USER_GID $USERNAME && \
    useradd --uid $USER_UID --gid $USER_GID -m $USERNAME && \
    usermod -aG audio $USERNAME && \
    chsh $USERNAME -s /bin/bash && \
    mkdir -p /home/$USERNAME/.local/share && \
    chown -R $USERNAME:$USERNAME /home/$USERNAME/.local

USER $USERNAME
