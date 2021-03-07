#!/bin/sh

# Make this script exit with error if any command fails
set -e

# Make sure an argument is provided to this script
if [ $# -ne 1 ]
then
	echo "Usage: $0 build-distro-name"
	echo "'build-distro-name' is the Linux distribution name your are building radvd on."
	echo "Currently supported distributions are:"
	echo "  ubuntu"
fi

# Install the right dependencies according to build host
echo "Installing required dependencies for distribution '${1}'..."
case $1 in
	ubuntu)
		sudo apt update
		sudo apt install autoconf automake check gettext libtool pkg-config
		;;
	*)
		echo "Unsupported distribution, build will be tried without installing any dependencies."
		;;
esac

# Build radvd
./autogen.sh
./configure --with-check
make -j
make check
make dist-xz
sudo make install
