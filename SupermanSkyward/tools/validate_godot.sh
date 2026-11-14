#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
GODOT="${GODOT_BIN:-$HOME/.local/bin/godot}"
if ! test -x "$GODOT"; then
  echo "Godot not installed. Run: bash tools/install_godot.sh" >&2
  exit 2
fi
echo "Godot version:"
"$GODOT" --headless --version
echo "Importing assets and scripts..."
"$GODOT" --headless --editor --path . --import --quit
echo "Smoke test (120 physics frames):"
"$GODOT" --headless --path . --quit-after 120
echo "Validation commands completed. Review output for ERROR lines."
