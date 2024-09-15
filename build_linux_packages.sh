#!/bin/bash

# Build toolchain images
bash ./build_toolchain_images.sh

# Execute cmake in workflow mode
docker run --rm -v ".:/tmp/russian-doom" toolchain-russian-doom-debian \
       /bin/bash -c "cd /tmp/russian-doom && cmake --workflow --preset 'debian-linux-release'"
