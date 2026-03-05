#!/usr/bin/env bash
set -e

# --- ARM Toolchain ---
TOOLCHAIN_ZIP="gcc-arm-none-eabi-7-2018-q2-update-win32.zip"
TOOLCHAIN_URL="https://developer.arm.com/-/media/Files/downloads/gnu-rm/7-2018q2/gcc-arm-none-eabi-7-2018-q2-update-win32.zip"
TOOLCHAIN_DIR="/c/tc"
TMP_TOOLCHAIN="/tmp/${TOOLCHAIN_ZIP}"

# --- VESC Tool (hosted on Google Drive) ---
VESC_TOOL_FILE_ID="1prwl3UvWELG_8rTuvLSXxb01_-TOf2uJ"
VESC_TOOL_URL="https://drive.usercontent.google.com/download?id=${VESC_TOOL_FILE_ID}&export=download&authuser=0&confirm=t"
VESC_TOOL_ZIP="/tmp/vesc_tool_free_windows.zip"
# Install next to this script (repo root)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VESC_TOOL_DEST="${SCRIPT_DIR}/vesc_tool_6.06.exe"

echo "=== Refloat Windows Build Environment Installer ==="
echo ""

# --- check curl ---
if ! command -v curl &>/dev/null; then
    echo "[ERROR] curl not found. Please install Git for Windows (includes curl)."
    exit 1
fi

# ================================================================
# Step 1: ARM toolchain
# ================================================================
if [ -f "${TOOLCHAIN_DIR}/bin/arm-none-eabi-gcc" ] || [ -f "${TOOLCHAIN_DIR}/bin/arm-none-eabi-gcc.exe" ]; then
    echo "[SKIP] ARM toolchain already installed at ${TOOLCHAIN_DIR}"
    GCC_VER=$("${TOOLCHAIN_DIR}/bin/arm-none-eabi-gcc" --version 2>&1 | head -1)
    echo "       ${GCC_VER}"
else
    echo "[1/2] Downloading ARM GCC toolchain (~130 MB)..."
    echo "      ${TOOLCHAIN_URL}"
    curl -L --progress-bar \
        -H "User-Agent: Mozilla/5.0" \
        -o "${TMP_TOOLCHAIN}" \
        "${TOOLCHAIN_URL}"

    echo "      Extracting to ${TOOLCHAIN_DIR} ..."
    mkdir -p "${TOOLCHAIN_DIR}"
    tar -xf "${TMP_TOOLCHAIN}" -C "${TOOLCHAIN_DIR}"
    rm -f "${TMP_TOOLCHAIN}"

    GCC_EXE="${TOOLCHAIN_DIR}/bin/arm-none-eabi-gcc"
    if [ ! -f "${GCC_EXE}" ] && [ ! -f "${GCC_EXE}.exe" ]; then
        echo "[ERROR] arm-none-eabi-gcc not found after extraction."
        ls "${TOOLCHAIN_DIR}"
        exit 1
    fi
    GCC_VER=$("${TOOLCHAIN_DIR}/bin/arm-none-eabi-gcc" --version 2>&1 | head -1)
    echo "[OK]  ${GCC_VER}"
fi

echo ""

# ================================================================
# Step 2: VESC Tool
# ================================================================
if [ -f "${VESC_TOOL_DEST}" ]; then
    echo "[SKIP] vesc_tool already present at ${VESC_TOOL_DEST}"
else
    echo "[2/2] Downloading VESC Tool Free for Windows (~93 MB)..."
    echo "      Google Drive file ID: ${VESC_TOOL_FILE_ID}"
    curl -L --progress-bar \
        -H "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) Chrome/120.0.0.0" \
        -o "${VESC_TOOL_ZIP}" \
        "${VESC_TOOL_URL}"

    # Verify it's actually a zip
    MAGIC=$(xxd "${VESC_TOOL_ZIP}" 2>/dev/null | head -1 || od -A x -t x1 "${VESC_TOOL_ZIP}" | head -1)
    if ! echo "$MAGIC" | grep -q '504b'; then
        echo "[ERROR] Downloaded file is not a ZIP (Google Drive may have returned HTML)."
        echo "        First bytes: $MAGIC"
        rm -f "${VESC_TOOL_ZIP}"
        exit 1
    fi

    echo "      Extracting vesc_tool exe ..."
    # Find the .exe inside the zip and extract to repo root
    EXE_NAME=$(unzip -l "${VESC_TOOL_ZIP}" 2>/dev/null | grep -i '\.exe' | awk '{print $NF}' | head -1)
    if [ -z "$EXE_NAME" ]; then
        echo "[ERROR] No .exe found inside downloaded zip."
        unzip -l "${VESC_TOOL_ZIP}" | head -20
        rm -f "${VESC_TOOL_ZIP}"
        exit 1
    fi
    echo "      Found: ${EXE_NAME}"
    unzip -j "${VESC_TOOL_ZIP}" "${EXE_NAME}" -d "${SCRIPT_DIR}"

    # Rename to expected filename if needed
    EXTRACTED=$(find "${SCRIPT_DIR}" -maxdepth 1 -iname 'vesc_tool*.exe' | head -1)
    if [ -n "$EXTRACTED" ] && [ "$EXTRACTED" != "${VESC_TOOL_DEST}" ]; then
        mv "$EXTRACTED" "${VESC_TOOL_DEST}"
        echo "      Renamed to $(basename ${VESC_TOOL_DEST})"
    fi

    rm -f "${VESC_TOOL_ZIP}"
    echo "[OK]  $(basename ${VESC_TOOL_DEST}) installed."
fi

echo ""
echo "=== Installation complete! ==="
echo "Run build.bat to build refloat.vescpkg"
