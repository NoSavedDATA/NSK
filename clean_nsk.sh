#!/usr/bin/env bash

set -e



if [ -n "$SUDO_USER" ]; then
    PREFIX="/usr/bin/nsk"
    USER_HOME=$(getent passwd "$SUDO_USER" | cut -d: -f6)
else
    PREFIX="$HOME/.local/nsk"
    USER_HOME="$HOME"
fi

rm -r "$PREFIX"


BASHRC="$USER_HOME/.bashrc"

# remove nsk PATH entry
sed -i "\|^export PATH=\"$PREFIX/bin:\$PATH\"$|d" "$BASHRC"
# remove NSK_LIBS export
sed -i "\|^export NSK_LIBS=$PREFIX/lib$|d" "$BASHRC"

echo "✅ nsk environment entries removed"
echo "ℹ️  Run: source ~/.bashrc (or open a new shell)"
