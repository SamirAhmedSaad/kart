# Kartwheel variant of the Cartoon skin, made with the same method as Cartoon Ocean:

for i in *.png ; do magick "$i" -colorspace gray -fill "#8b5cf6" -tint 100 "$i" ; done
for i in *focus.png *focused.png gauge_fill.png spinner_fill.png select.png table_header_down.png ; do magick "$i" -colorspace gray -fill "#ff2e88" -tint 100 "$i" ; done
magick bottom_bar.png -colorspace gray -fill "#3b1d6e" -tint 100 -modulate 55 bottom_bar.png
magick ../cartoon/background.jpg -modulate 100,40 ( -size 1920x1080 gradient:"#ffc38a"-"#8b5cf6" ) -compose multiply -composite -modulate 125,130 background.jpg
# Frosted band behind the main menu labels, for contrast:
magick background.jpg ( -size 1920x1080 xc:none -fill "rgba(255,240,250,0.55)" -draw "rectangle 0,660 1920,930" -blur 0x40 ) -compose over -composite background.jpg
# Brighter background (replaces the two background commands above):
magick ../cartoon/background.jpg -modulate 100,118 ( -size 1920x1080 gradient:"rgba(124,58,237,0.35)"-"rgba(124,58,237,0)" ) -compose over -composite ( -size 1920x1080 xc:none -fill "rgba(255,240,250,0.30)" -draw "rectangle 0,700 1920,930" -blur 0x40 ) -compose over -composite -quality 92 background.jpg
# Solid panels and the gold selection frame:
magick -size 64x64 xc:none -fill "rgba(255,255,255,0.86)" -stroke "#3b1d6e" -strokewidth 5 -draw "roundrectangle 3,3 60,60 12,12" PNG32:section_solid.png
magick -size 64x32 xc:none -fill none -stroke "#ffc21a" -strokewidth 8 -draw "roundrectangle 4,4 59,27 6,6" -stroke white -strokewidth 2 -draw "roundrectangle 9,9 54,22 3,3" PNG32:glass_square_focused_gold.png
# Gold focus glow behind icon buttons:
magick -size 256x256 radial-gradient:"#ffffff"-"#ffb300" col.png ; magick -size 256x256 radial-gradient:white-black -evaluate pow 0.7 -level 0%,85% alpha.png ; magick col.png alpha.png -alpha off -compose CopyOpacity -composite PNG32:glass_iconhighlight_focus.png
# Lighter bottom bar so the black button labels are readable (made from the earlier dark bar):
magick bottom_bar.png -colorspace gray -auto-level +level 55%,85% -fill "#c9b8ff" -tint 100 -modulate 110,160 PNG32:bottom_bar.png
# Gold ring behind the selected difficulty and other combo choices:
magick -size 256x256 xc:none -fill "rgba(255,214,90,0.45)" -stroke "#ffb300" -strokewidth 14 -draw "circle 128,128 128,16" -fill none -stroke white -strokewidth 4 -draw "circle 128,128 128,30" PNG32:bubble.png
# Dialog frame:
magick -size 256x256 xc:none -fill "#f7f3ff" -stroke "#3b1d6e" -strokewidth 14 -draw "roundrectangle 8,8 247,247 34,34" -fill none -stroke "#8b5cf6" -strokewidth 5 -draw "roundrectangle 21,21 234,234 24,24" PNG32:dialog_solid.png
# Tile icons for the pause menu and yes/no dialogs, for each icon:
magick -size 256x256 xc:none ( -size 256x256 gradient:"#b197fc"-"#6d28d9" ( -size 256x256 xc:black -fill white -draw "roundrectangle 8,8 247,247 44,44" ) -alpha off -compose CopyOpacity -composite ) -compose over -composite -fill none -stroke "#2e1065" -strokewidth 12 -draw "roundrectangle 8,8 247,247 44,44" -stroke "rgba(255,255,255,0.55)" -strokewidth 4 -draw "roundrectangle 22,22 233,233 32,32" ( ICON.png -resize 170x170 ) -gravity center -compose over -composite PNG32:data/gui/icons/tile_NAME.png
# Home screen tiles (orange for the main race button, gold frame when focused):
# tile ICON OUT TOP BOTTOM FRAME FRAME_WIDTH ICON_SIZE is the tile command above with the gradient, frame and icon size as parameters
#   tile menu_race.png  tile_menu_race.png        "#ffd166" "#f97316" "#7c2d12" 12 180
#   tile menu_race.png  tile_menu_race_focus.png  "#ffe08a" "#fb923c" "#ffd23f" 20 190
#   tile menu_story.png tile_menu_story.png       "#b197fc" "#6d28d9" "#2e1065" 12 180
#   tile menu_story.png tile_menu_story_focus.png "#c4b5fd" "#7c3aed" "#ffd23f" 20 190
#   bottom row (main_options, main_help, tutorial, crown, main_about, main_quit): "#b197fc" "#6d28d9" "#2e1065" 12 170

# Dark version for readable white text (replaces the light panels, bar and background above):
for i in button.png tab.png tab_vert.png spinner.png ; do magick $i -modulate 50,70,98 $i ; done
for i in spinner_fill.png gauge_fill.png ; do magick $i -modulate 72,105 $i ; done
magick spinner_deactivated.png -modulate 45,60 spinner_deactivated.png
for i in button_focused.png tab_focus.png tab_vert_focus.png spinner_focus.png ; do magick $i -modulate 82,110 $i ; done
for i in generic.png achievement.png friend.png ; do magick $i -modulate 50,110 $i ; done
magick ../cartoon/tooltip.png -fill "#1e1440" -colorize 70 tooltip.png
magick ../cartoon/table_header.png -fill "#3b1d6e" -colorize 75 table_header.png
magick ../cartoon/error.png -modulate 55,110 error.png
magick ../cartoon/rsection.png -fill "#1e1440" -colorize 70 rsection.png
magick -size 64x64 xc:none -fill "rgba(24,14,56,0.90)" -stroke "#8b5cf6" -strokewidth 5 -draw "roundrectangle 3,3 60,60 12,12" PNG32:section_solid.png
magick -size 256x256 xc:none -fill "#1e1440" -stroke "#8b5cf6" -strokewidth 14 -draw "roundrectangle 8,8 247,247 34,34" -fill none -stroke "#c4b5fd" -strokewidth 4 -draw "roundrectangle 21,21 234,234 24,24" PNG32:dialog_solid.png
magick bottom_bar.png -colorspace gray -auto-level +level 8%,22% -fill "#2a1760" -tint 100 PNG32:bottom_bar.png
magick ../cartoon/background.jpg -modulate 72,115 ( -size 1920x1080 gradient:"rgba(76,29,149,0.45)"-"rgba(30,20,64,0.25)" ) -compose over -composite ( -size 1920x1080 xc:none -fill "rgba(15,8,35,0.35)" -draw "rectangle 0,560 1920,1080" -blur 0x60 ) -compose over -composite -quality 92 background.jpg
#   tile options_players.png tile_players.png "#b197fc" "#6d28d9" "#2e1065" 12 170
#   tile green_check.png     tile_start.png   "#ffd166" "#f97316" "#7c2d12" 12 180  (Start Race and Continue)
