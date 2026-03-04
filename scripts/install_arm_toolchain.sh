#!/usr/bin/env bash

set -euo pipefail

VERSION="13.3.rel1"
BASE_NAME="arm-gnu-toolchain-${VERSION}-x86_64-arm-none-eabi"
ARCHIVE="${BASE_NAME}.tar.xz"
URL="https://developer.arm.com/-/media/Files/downloads/gnu/${VERSION}/binrel/${ARCHIVE}"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TOOLCHAIN_DIR="${ROOT_DIR}/.toolchains"
INSTALL_DIR="${TOOLCHAIN_DIR}/${BASE_NAME}"
LINK_DIR="${TOOLCHAIN_DIR}/arm-none-eabi"
ARCHIVE_PATH="${TOOLCHAIN_DIR}/${ARCHIVE}"

mkdir -p "${TOOLCHAIN_DIR}"

if [ ! -x "${INSTALL_DIR}/bin/arm-none-eabi-gcc" ]; then
    echo "Downloading ${ARCHIVE} ..."
    if command -v curl >/dev/null 2>&1; then
        curl -L --fail --retry 3 -o "${ARCHIVE_PATH}" "${URL}"
    elif command -v wget >/dev/null 2>&1; then
        wget -O "${ARCHIVE_PATH}" "${URL}"
    else
        echo "Error: neither curl nor wget is available."
        exit 1
    fi

    echo "Extracting toolchain ..."
    tar -xJf "${ARCHIVE_PATH}" -C "${TOOLCHAIN_DIR}"
fi

ln -sfn "${INSTALL_DIR}" "${LINK_DIR}"

echo "Toolchain installed at: ${INSTALL_DIR}"
echo "Use it for this shell with:"
echo "  export PATH=\"${LINK_DIR}/bin:\$PATH\""
echo "Then verify with:"
echo "  arm-none-eabi-gcc --version"