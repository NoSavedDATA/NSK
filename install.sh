#!/usr/bin/env bash

set -e

wget https://github.com/NoSavedDATA/NSK/releases/download/nsk-bin/nsk
wget https://github.com/NoSavedDATA/NSK/releases/download/nsk-bin/sys.tar.bz2
wget https://github.com/NoSavedDATA/nsm/releases/download/latest/nsm

PREFIX="$HOME/.local/nsk"
USER_HOME="$HOME"


mkdir -p "$PREFIX/bin"
mkdir -p "$PREFIX/lib"
mkdir -p "$PREFIX/sys_lib"

mv ./nsk "$PREFIX/bin"
mv ./nsm "$PREFIX/bin"

chmod +x "$PREFIX/bin/nsk"
chmod +x "$PREFIX/bin/nsm"

tar -xjf sys.tar.bz2 -C "$PREFIX"
rm sys.tar.bz2


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
