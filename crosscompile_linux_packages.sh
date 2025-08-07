#!/bin/bash

# Check args
if [ $# -lt 2 ]; then
    echo 'crosscompile_linux_packages.sh %preset% {debian,fedora}'
    exit 1
fi

# Build toolchain images
bash ./toolchain_images.sh build "$2"

# Execute cmake in workflow mode
docker run --rm -v ".:/tmp/russian-doom" "toolchain-russian-doom-$2" /bin/bash -c "
    git config --global --add safe.directory '/tmp/russian-doom' &&
    cd /tmp/russian-doom &&
    export MAKEFLAGS=--keep-going &&
    export CI=true &&
    cmake --workflow --preset '$1'"
