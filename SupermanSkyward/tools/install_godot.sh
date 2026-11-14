#!/usr/bin/env bash
set -euo pipefail
VERSION="${GODOT_VERSION:-4.3}"
DEST="${HOME}/.local/bin"
mkdir -p "$DEST"
case "$(uname -m)" in
  x86_64) ARCH="x86_64";;
  *) echo "Unsupported architecture: $(uname -m). Codespaces x86_64 is expected." >&2; exit 2;;
esac
URL="https://github.com/godotengine/godot/releases/download/${VERSION}-stable/Godot_v${VERSION}-stable_linux.${ARCH}.zip"
TEMP="$(mktemp -d)"
trap 'rm -rf "$TEMP"' EXIT
echo "Downloading official Godot ${VERSION} from $URL"
curl -fL --retry 3 "$URL" -o "$TEMP/godot.zip"
unzip -q "$TEMP/godot.zip" -d "$TEMP"
BIN="$TEMP/Godot_v${VERSION}-stable_linux.${ARCH}"
test -f "$BIN"
install -m 755 "$BIN" "$DEST/godot"
echo "Installed: $DEST/godot"
"$DEST/godot" --headless --version
