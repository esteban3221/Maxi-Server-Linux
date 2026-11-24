#!/usr/bin/env sh
set -eu

OUT_DIR="assets/fonts"
mkdir -p "$OUT_DIR"

FONT_REG="/usr/share/fonts/ubuntu/Ubuntu-R.ttf"
FONT_BOLD="/usr/share/fonts/ubuntu/Ubuntu-B.ttf"
FONT_MONO="/usr/share/fonts/ubuntu/UbuntuMono-R.ttf"
FONT_MONO_BOLD="/usr/share/fonts/ubuntu/UbuntuMono-B.ttf"

RANGE="0x20-0x7E,0xA0-0xFF,0x2022"

gen_font() {
    name="${3%.c}"
    npx --yes lv_font_conv \
        --bpp 4 \
        --format lvgl \
        --no-compress \
        --lv-include "lvgl.h" \
        --size "$2" \
        --font "$1" \
        --range "$RANGE" \
        -o "$OUT_DIR/$3"

    cat <<EOF > "$OUT_DIR/${name}.h"
#pragma once
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif
LV_FONT_DECLARE(${name});
#ifdef __cplusplus
}
#endif
EOF
}

# Familia Ubuntu
gen_font "$FONT_REG"       14 "font_adw_14.c"
gen_font "$FONT_REG"       16 "font_adw_16.c"
gen_font "$FONT_BOLD"      16 "font_adw_16_bold.c"
gen_font "$FONT_BOLD"      22 "font_adw_22_bold.c"
gen_font "$FONT_BOLD"      28 "font_adw_28_bold.c"

# Familia Ubuntu Mono
gen_font "$FONT_MONO"      16 "font_adw_mono_16.c"
gen_font "$FONT_MONO_BOLD" 22 "font_adw_mono_22.c"

ls -lh "$OUT_DIR"