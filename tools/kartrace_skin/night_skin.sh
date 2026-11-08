#!/bin/bash
# Regenerates the Kart Race "Night Circuit" skin images (dark navy, racing orange, cyan).
# Usage: tools/kartrace_skin/night_skin.sh   (run from the repository root)
set -e
SKIN=data/skins/cartoon-kartwheel
HERE=$(cd "$(dirname "$0")" && pwd)
cd "$SKIN"

NAVY="#0B0F1A"; PANEL="#151B2B"; GLASS="#1B2236"; EDGE="#34405C"
ORANGE="#F2531A"; ORANGE_HI="#FF8A5C"; ORANGE_LO="#8A2A08"; CYAN="#3DDCFF"

# Buttons, tabs and spinners: 288x96, a 6 px darker lip at the bottom
bar() { # out fill edge lip
    magick -size 288x96 xc:none \
        -fill "$4" -stroke none -draw "roundrectangle 3,9 284,92 18,18" \
        -fill "$2" -stroke "$3" -strokewidth 3 -draw "roundrectangle 3,3 284,86 18,18" \
        -stroke "rgba(255,255,255,0.10)" -strokewidth 2 -draw "line 22,8 266,8" PNG32:"$1"
}
arrows() { # in/out color
    magick "$1" -fill "$2" -stroke none \
        -draw "polygon 40,26 18,45 40,64" -draw "polygon 248,26 270,45 248,64" PNG32:"$1"
}
bar button.png          "$GLASS"  "$EDGE"      "#070A12"
bar button_focused.png  "$ORANGE" "$ORANGE_HI" "$ORANGE_LO"
bar button_inactive.png "#141927" "#232B3F"    "#070A12"
bar tab.png             "$PANEL"  "#2A3348"    "#070A12"
bar tab_focus.png       "$ORANGE" "$ORANGE_HI" "$ORANGE_LO"
bar tab_vert.png        "$PANEL"  "#2A3348"    "#070A12"
bar tab_vert_focus.png  "$ORANGE" "$ORANGE_HI" "$ORANGE_LO"
bar spinner.png             "$GLASS"  "$EDGE"      "#070A12"; arrows spinner.png "$CYAN"
bar spinner_focus.png       "$ORANGE" "$ORANGE_HI" "$ORANGE_LO"; arrows spinner_focus.png "#FFFFFF"
bar spinner_deactivated.png "#141927" "#232B3F"    "#070A12"; arrows spinner_deactivated.png "#4A546B"
# Pressed spinner arrows (only the left and right 110 px are drawn)
magick -size 256x128 xc:none -fill "rgba(255,255,255,0.18)" -draw "roundrectangle 4,4 100,118 18,18" \
    -draw "roundrectangle 156,4 252,118 18,18" PNG32:spinner_down.png

# Fills, gauges and list selection
magick -size 64x64 xc:none -fill "$ORANGE" -draw "roundrectangle 0,0 63,63 8,8" PNG32:gauge_fill.png
magick -size 64x64 xc:none -fill "$ORANGE" -draw "roundrectangle 0,0 63,63 8,8" PNG32:spinner_fill.png
magick -size 64x32 xc:none -fill "$GLASS" -stroke "$EDGE" -strokewidth 3 -draw "roundrectangle 2,2 61,29 8,8" PNG32:progress_bg.png
magick -size 32x64 xc:"rgba(242,83,26,0.90)" PNG32:select.png
magick -size 32x64 xc:"$GLASS" -fill "$EDGE" -draw "rectangle 0,61 31,63" PNG32:table_header.png
magick -size 32x64 xc:"$ORANGE" PNG32:table_header_down.png
magick -size 32x32 xc:none -fill "#46526F" -draw "roundrectangle 4,0 27,31 10,10" PNG32:scrollbar_thumb.png
magick -size 24x24 xc:none -fill "$CYAN" -draw "polygon 4,17 12,7 20,17" PNG32:scrollbar_button.png

# Checkboxes
box() { # out fill edge width check
    magick -size 96x96 xc:none -fill "$2" -stroke "$3" -strokewidth "$4" -draw "roundrectangle 8,8 87,87 20,20" "${@:6}" PNG32:"$1"
}
CHECK=(-fill none -stroke white -strokewidth 10 -draw "stroke-linecap round stroke-linejoin round polyline 28,50 42,64 68,34")
box checkbox.png               "$GLASS"  "$EDGE"  4
box checkbox_focus.png         "$GLASS"  "$CYAN"  7
box checkbox_checked.png       "$ORANGE" "$ORANGE_HI" 4 x "${CHECK[@]}"
box checkbox_checked_focus.png "$ORANGE" "$CYAN"  7 x "${CHECK[@]}"

# Arrows (kart and track lists)
tri() { # out color points
    magick -size 128x128 xc:none -fill "$2" -stroke "rgba(0,0,0,0.55)" -strokewidth 6 \
        -draw "stroke-linejoin round polygon $3" PNG32:"$1"
}
tri left_arrow.png        "$CYAN"   "92,18 24,64 92,110"
tri right_arrow.png       "$CYAN"   "36,18 104,64 36,110"
tri left_arrow_focus.png  "$ORANGE" "92,18 24,64 92,110"
tri right_arrow_focus.png "$ORANGE" "36,18 104,64 36,110"
tri up_arrow.png          "$CYAN"   "18,92 64,24 110,92"
tri down_arrow.png        "$CYAN"   "18,36 64,104 110,36"

# Panels, dialogs and tooltips
magick -size 64x64 xc:none -fill "rgba(21,27,43,0.94)" -stroke "rgba(255,255,255,0.14)" -strokewidth 3 -draw "roundrectangle 2,2 61,61 14,14" PNG32:section_solid.png
magick -size 64x64 xc:none -fill "rgba(21,27,43,0.94)" -stroke "rgba(255,255,255,0.14)" -strokewidth 3 -draw "roundrectangle 2,2 61,61 14,14" PNG32:rsection.png
magick -size 256x256 xc:none -fill "$PANEL" -stroke "#2E3850" -strokewidth 6 -draw "roundrectangle 4,4 251,251 34,34" \
    -stroke "$ORANGE" -strokewidth 4 -draw "line 60,6 196,6" PNG32:dialog_solid.png
magick -size 64x32 xc:none -fill "$PANEL" -stroke "#2E3850" -strokewidth 2 -draw "roundrectangle 1,1 62,30 8,8" PNG32:tooltip.png
magick -size 1024x128 xc:"#0E1322" -fill "#2A3348" -draw "rectangle 0,0 1023,2" PNG32:bottom_bar.png
# Message bars (tutorial hints, achievements, errors): dark bar with a colored badge on the left
msg() { # out badge mark...
    magick -size 256x128 xc:none -fill "$PANEL" -stroke "#2E3850" -strokewidth 3 -draw "roundrectangle 60,14 250,114 18,18" \
        -fill "$2" -stroke "$NAVY" -strokewidth 6 -draw "circle 64,64 64,10" "${@:3}" PNG32:"$1"
}
MARK=(-fill none -stroke "$NAVY" -strokewidth 10 -draw "stroke-linecap round line 64,38 64,70" -fill "$NAVY" -stroke none -draw "circle 64,88 64,82")
msg generic.png     "$ORANGE"
msg achievement.png "#FFD23F" -fill none -stroke "$NAVY" -strokewidth 8 -draw "stroke-linejoin round polygon 46,38 82,38 78,62 64,72 50,62" -draw "line 64,72 64,88" -draw "line 52,90 76,90"
msg friend.png      "$CYAN"   -fill none -stroke "$NAVY" -strokewidth 8 -draw "circle 64,52 64,40" -draw "stroke-linecap round path 'M 42 92 Q 64 64 86 92'"
msg error.png       "#E5484D" "${MARK[@]}"

# Focus and selection highlights
magick -size 64x32 xc:none -fill none -stroke "$ORANGE" -strokewidth 8 -draw "roundrectangle 4,4 59,27 8,8" \
    -stroke "rgba(255,180,150,0.9)" -strokewidth 2 -draw "roundrectangle 9,9 54,22 4,4" PNG32:glass_square_focused_gold.png
# Focus glow: the engine draws it as a 2:1 pulsing light centred on the bottom edge of the focused icon
magick -size 512x256 radial-gradient:"rgba(255,170,130,1)"-"rgba(242,83,26,0.6)" \
    \( -size 512x256 radial-gradient:white-black -evaluate pow 1.2 \) -alpha off -compose CopyOpacity -composite PNG32:glass_iconhighlight_focus.png
magick -size 256x256 xc:none -fill "rgba(242,83,26,0.28)" -stroke "$ORANGE" -strokewidth 12 -draw "circle 128,128 128,16" PNG32:bubble.png

# Background: navy with soft orange and cyan glows
magick -size 1920x1080 xc:"$NAVY" \
    \( -size 1920x1080 xc:none -fill "rgba(242,83,26,0.30)" -draw "ellipse 1500,760 520,360 0,360" -blur 0x160 \) -compose over -composite \
    \( -size 1920x1080 xc:none -fill "rgba(61,220,255,0.14)" -draw "ellipse 260,180 480,320 0,360" -blur 0x160 \) -compose over -composite \
    -quality 92 background.jpg

# Tile icons
python3 "$HERE/make_tiles.py" data/gui/icons
