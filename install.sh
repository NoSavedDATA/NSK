#!/usr/bin/env bash

set -e

wget https://github.com/NoSavedDATA/NSK/releases/download/nsk-bin/nsk


if [ -n "$SUDO_USER" ]; then
    PREFIX="/usr/bin/nsk"
    USER_HOME=$(getent passwd "$SUDO_USER" | cut -d: -f6)
else
    PREFIX="$HOME/.local/nsk"
    USER_HOME="$HOME"
fi


mkdir -p "$PREFIX/bin"
mkdir -p "$PREFIX/lib"

mv ./nsk "$PREFIX/bin"

chmod +x "$PREFIX/bin/nsk"


# Set up bashrc
BASHRC="$USER_HOME/.bashrc"


# add nsk binary
if ! grep -q 'export PATH=.*/nsk/bin' "$BASHRC"; then
    echo "export PATH=\"$PREFIX/bin:\$PATH\"" >> "$BASHRC"
fi
# track nsk .so libs
if ! grep -q 'export NSK_LIBS=.*/lib' "$BASHRC"; then
    echo "export NSK_LIBS=$PREFIX/lib" >> "$BASHRC"
fi

echo "✅ nsk installed"
echo "run ~/.bashrc"
