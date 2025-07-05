#!/bin/bash

# Check args
case $1 in
  ci)
   preset='ci-%s-linux-dev'
   ci='export CI=true &&'
   ;;
 release)
   preset='%s-linux-release'
   ;;
 *)
   echo 'crosscompile_linux_packages.sh {ci,release} [debian,fedora]...'
   exit 1
   ;;
esac

# Default distros
distros=("${@:2}")
if [ $# -eq 1 ]; then
    distros=('debian' 'fedora')
fi

# Build toolchain images
bash ./toolchain_images.sh build "${distros[@]}"

# Execute cmake in workflow mode for each distro
for distro in "${distros[@]}"; do
    # shellcheck disable=SC2059
    docker run --rm -v ".:/tmp/russian-doom" "toolchain-russian-doom-${distro}" /bin/bash -c "
        git config --global --add safe.directory '/tmp/russian-doom' &&
        cd /tmp/russian-doom &&
        export MAKEFLAGS=--keep-going &&
        ${ci}
        cmake --workflow --preset '$(printf "${preset}" "${distro}")'"
done
