#!/bin/sh
# Converts theme/*.png into theme_out/ (copy that folder's contents to sdmc:/loader/theme/).
set -e
cd "$(dirname "$0")/.."
mkdir -p theme_out
python3 tools/png2fb.py theme/top.png    theme_out/top.bin    --top
python3 tools/png2fb.py theme/bottom.png theme_out/bottom.bin --bottom
python3 tools/png2fb.py theme/logo.png   theme_out/logo.spr   --sprite
echo "Copy theme_out/* to sdmc:/loader/theme/"
