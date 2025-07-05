#!/bin/bash

# Check Docker is installed
if ! which docker > /dev/null 2>&1; then
  echo "Docker is not installed" 1>&2
  exit 1
fi

# Check Docker demon is running
if ! pgrep "[d]ocker" > /dev/null 2>&1; then
  echo "Docker demon is not running" 1>&2
  exit 1
fi

# Check Docker is accessible without root permissions
if !  docker info > /dev/null 2>&1; then
  echo "Docker demon is not accessible without root permissions.
        Add yourself to the 'docker' group and reboot your system" 1>&2
  exit 1
fi

# Check args
case $1 in
  build)
   rebuild=false
   ;;
 rebuild)
   rebuild=true
   ;;
 *)
   echo 'toolchain_images.sh {build,rebuild} [debian,fedora]...'
   exit 1
   ;;
esac

# Default distros
distros=("${@:2}")
if [ $# -eq 1 ]; then
    distros=('debian' 'fedora')
fi

# Build toolchain images
for distro in "${distros[@]}"; do
    # Delete image if rebuild
    if [ "$rebuild" = true ]; then
        docker image rm -f "toolchain-russian-doom-${distro}"
        extra_args='--no-cache'
    fi

    # Only build if doesn't exist
    if [ -z "$(docker images -q "toolchain-russian-doom-${distro}" 2> /dev/null)" ]; then
        # shellcheck disable=SC2086
        docker build "./.devcontainer" \
               -f "./.devcontainer/${distro^}.dockerfile" \
               -t "toolchain-russian-doom-${distro}" \
               $extra_args
    fi
done
