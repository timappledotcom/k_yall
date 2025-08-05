#!/bin/bash

# K, Y'all Installation Script

set -e

echo "Installing K, Y'all dependencies..."

# Detect the distribution
if command -v apt &> /dev/null; then
    # Ubuntu/Debian
    echo "Detected Debian/Ubuntu system"
    sudo apt update
    sudo apt install -y cmake build-essential \
        qtbase6-dev qt6-websockets-dev \
        libkf6i18n-dev libkf6coreaddons-dev libkf6config-dev \
        libkf6configwidgets-dev libkf6statusnotifieritem-dev \
        libkf6notifications-dev libkf6kio-dev \
        extra-cmake-modules
elif command -v dnf &> /dev/null; then
    # Fedora
    echo "Detected Fedora system"
    sudo dnf install -y cmake gcc-c++ \
        qt6-qtbase-devel qt6-qtwebsockets-devel \
        kf6-ki18n-devel kf6-kcoreaddons-devel kf6-kconfig-devel \
        kf6-kconfigwidgets-devel kf6-kstatusnotifieritem-devel \
        kf6-knotifications-devel kf6-kio-devel \
        extra-cmake-modules
elif command -v pacman &> /dev/null; then
    # Arch Linux
    echo "Detected Arch Linux system"
    sudo pacman -S --needed cmake base-devel \
        qt6-base qt6-websockets \
        ki18n6 kcoreaddons6 kconfig6 kconfigwidgets6 \
        kstatusnotifieritem6 knotifications6 kio6 \
        extra-cmake-modules
elif command -v zypper &> /dev/null; then
    # openSUSE
    echo "Detected openSUSE system"
    sudo zypper install -y cmake gcc-c++ \
        qt6-base-devel qt6-websockets-devel \
        kf6-ki18n-devel kf6-kcoreaddons-devel kf6-kconfig-devel \
        kf6-kconfigwidgets-devel kf6-kstatusnotifieritem-devel \
        kf6-knotifications-devel kf6-kio-devel \
        extra-cmake-modules
else
    echo "Unsupported distribution. Please install dependencies manually."
    echo "See README.md for the list of required packages."
    exit 1
fi

echo ""
echo "Dependencies installed successfully!"
echo ""
echo "To build and install K, Y'all:"
echo "  ./build.sh"
echo "  cd build"
echo "  sudo make install"
