#!/bin/bash
set -e

SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
PYTHON3=$(which python3 || true)

if [ ! -x "$PYTHON3" ]; then
    echo "Cannot find 'python3' in your PATH. Install Python before you continue."
    echo "Installation instructions: https://www.python.org/downloads/"
    exit 1
fi

echo "Checking Python dependencies..."

HAS_PYSERIAL=$(pip3 list | grep -F pyserial || true)
HAS_ESPTOOL=$(pip3 list | grep -F esptool || true)

if [ -z "$HAS_PYSERIAL" ]; then
    pip3 install pyserial>=3.4
fi

if [ -z "$HAS_ESPTOOL" ]; then
    pip3 install esptool>=3.3
fi

echo "Checking Python dependencies OK"
echo ""

echo "Flashing firmware and model... "
echo ""

# Flash do firmware principal
python3 -m esptool -b 460800 write_flash 0x10000 "${SCRIPTPATH}"/firmware_esp32.bin

# Flash do modelo na partição model
python3 -m esptool -b 460800 write_flash 0x110000 "${SCRIPTPATH}"/firmware_esp32.bin

echo ""
echo "Flashed your ESP32 development board with firmware and model."
echo "To set up your development with Edge Impulse, run 'edge-impulse-daemon'"
echo "To run your impulse on your development board, run 'edge-impulse-run-impulse'"