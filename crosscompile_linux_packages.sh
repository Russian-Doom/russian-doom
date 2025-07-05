#!/bin/bash

# Build toolchain images
bash ./build_toolchain_images.sh "$@"

# Execute cmake in workflow mode
args=("$@")
if [ $# -eq 0 ]; then
    args=('debian' 'fedora')
fi

for distro in "${args[@]}"; do
    docker run --rm -v ".:/tmp/russian-doom" "toolchain-russian-doom-${distro}" \
           /bin/bash -c "cd /tmp/russian-doom && cmake --workflow --preset '${distro}-linux-release'"
done
