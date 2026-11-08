#!/usr/bin/env python3
"""Night Circuit tile icons: line icons on dark glass or orange tiles, rendered with ImageMagick."""
import subprocess, sys, os

OUT = sys.argv[1]

# 24x24 line icons (path data plus optional circles: (cx, cy, r, filled))
ICONS = {
    "play":     (["M8 5 L19 12 L8 19 Z"], [], True),
    "book":     (["M4 5 Q4 3 6 3 L19 3 L19 19 L6 19 Q4 19 4 21 Z", "M4 21 L4 5", "M8 7 L15 7"], [], False),
    "trophy":   (["M8 4 L16 4 L16 9 Q16 13 12 13 Q8 13 8 9 Z", "M8 6 L5 6 Q5 10 8 10", "M16 6 L19 6 Q19 10 16 10", "M12 13 L12 17", "M8 20 L16 20"], [], False),
    "sliders":  (["M4 7 L14 7", "M18 7 L20 7", "M4 17 L8 17", "M12 17 L20 17"], [(16, 7, 2, False), (10, 17, 2, False)], False),
    "help":     (["M9.5 9.5 Q9.5 7 12 7 Q14.5 7 14.5 9.5 Q14.5 11 12.8 11.8 Q12 12.3 12 13.5 L12 14"], [(12, 12, 9, False), (12, 17, 0.9, True)], False),
    "person":   (["M4 21 Q5 15 12 15 Q19 15 20 21"], [(12, 8, 4, False)], False),
    "info":     (["M12 11 L12 17"], [(12, 12, 9, False), (12, 7.6, 0.9, True)], False),
    "power":    (["M12 3 L12 11", "M6.3 6.3 A8 8 0 1 0 17.7 6.3"], [], False),
    "cap":      (["M2 9 L12 4 L22 9 L12 14 Z", "M6 11 L6 16 Q6 19 12 19 Q18 19 18 16 L18 11", "M22 9 L22 14"], [], False),
    "back":     (["M15 5 L8 12 L15 19"], [], False),
    "restart":  (["M5.5 7.5 A8 8 0 1 1 4 12", "M4 4 L4 9 L9 9"], [], False),
    "check":    (["M5 12.5 L9.5 17 L19 7"], [], False),
    "cross":    (["M6 6 L18 18", "M18 6 L6 18"], [], False),
    "add":      (["M12 5 L12 19", "M5 12 L19 12"], [], False),
    "remove":   (["M5 12 L19 12"], [], False),
    "rename":   (["M4 20 L8 20 L19 9 L15 5 L4 16 Z", "M13 7 L17 11"], [], False),
    "edit":     (["M4 20 L8 20 L18 10 L14 6 L4 16 Z", "M13 20 L20 20"], [], False),
    "wheel":    (["M3.5 10.5 L9.5 10.5", "M14.5 10.5 L20.5 10.5", "M12 14.5 L12 21"], [(12, 12, 9, False), (12, 12, 2.6, False)], False),
    "pedal":    (["M8 3 L16 3 L18 21 L6 21 Z", "M9 8 L15 8", "M8.7 12 L15.3 12", "M8.3 16 L15.7 16"], [], False),
    "tilt":     (["M9 4 L15 4 Q16 4 16 5 L16 19 Q16 20 15 20 L9 20 Q8 20 8 19 L8 5 Q8 4 9 4 Z", "M11 17 L13 17", "M4 8 Q2 12 4 16", "M20 8 Q22 12 20 16"], [], False),
    "home":     (["M3 11 L12 4 L21 11", "M5 10 L5 20 L19 20 L19 10", "M10 20 L10 14 L14 14 L14 20"], [], False),
    "flag":     (["M5 21 L5 4", "M5 4 L16 4 L14 8 L16 12 L5 12"], [], False),
    "watch":    (["M12 9 L12 13 L14.5 15", "M9.5 2.5 L14.5 2.5", "M18.5 6.5 L20 5"], [(12, 13, 8, False)], False),
    "leader":   (["M3 12 L13 12", "M9 8 L13 12 L9 16", "M16 6 L21 12 L16 18"], [], False),
    "shield":   (["M12 3 L20 6 L20 12 Q20 18 12 21 Q4 18 4 12 L4 6 Z", "M9 12 L11 14 L15 10"], [], False),
    "easy":     (["M4 17 A8 8 0 1 1 20 17", "M12 15 L7 11"], [(12, 15, 1.2, True)], False),
    "medium":   (["M4 17 A8 8 0 1 1 20 17", "M12 15 L12 8.5"], [(12, 15, 1.2, True)], False),
    "hard":     (["M4 17 A8 8 0 1 1 20 17", "M12 15 L17 11"], [(12, 15, 1.2, True)], False),
    "legend":   (["M4 18 L4 8 L8.5 12 L12 5 L15.5 12 L20 8 L20 18 Z"], [], False),
    "random":   (["M5 4 L19 4 Q20 4 20 5 L20 19 Q20 20 19 20 L5 20 Q4 20 4 19 L4 5 Q4 4 5 4 Z"],
                 [(8.5, 8.5, 1.2, True), (15.5, 15.5, 1.2, True), (12, 12, 1.2, True), (15.5, 8.5, 1.2, True), (8.5, 15.5, 1.2, True)], False),
    "audio":    (["M11 5 L6 9 L3 9 L3 15 L6 15 L11 19 Z", "M15.5 8.5 Q18 12 15.5 15.5", "M18.5 5.5 Q23 12 18.5 18.5"], [], False),
    "pad":      (["M7 7 L17 7 Q22 7 22 13 Q22 18 18.5 18 Q16.5 18 15 15 L9 15 Q7.5 18 5.5 18 Q2 18 2 13 Q2 7 7 7 Z", "M7 10 L7 13", "M5.5 11.5 L8.5 11.5"],
                 [(16, 10.5, 0.9, True), (18, 12.5, 0.9, True)], False),
    "screen":   (["M3 5 L21 5 L21 16 L3 16 Z", "M8 20 L16 20", "M12 16 L12 20"], [], False),
    "camera":   (["M3 8 L7 8 L9 5 L15 5 L17 8 L21 8 L21 19 L3 19 Z"], [(12, 13, 3.5, False)], False),
    "layout":   (["M3 4 L21 4 L21 20 L3 20 Z", "M3 9 L21 9", "M10 9 L10 20"], [], False),
    "globe":    (["M3 12 L21 12", "M12 3 Q7 12 12 21", "M12 3 Q17 12 12 21"], [(12, 12, 9, False)], False),
    "people":   (["M2 20 Q3 15 9 15 Q15 15 16 20", "M16 15 Q21 15 22 20"], [(9, 8, 3.5, False), (16.5, 9, 2.6, False)], False),
    "kart":     (["M3 15 L6 10 L13 10 L15 13 L21 13 L21 16 L3 16 Z", "M8 10 L9 7 L11 7"], [(7, 17.5, 2.2, False), (17, 17.5, 2.2, False)], False),
    "giveup":   (["M5 21 L5 4", "M5 4 L16 4 L14 8 L16 12 L5 12", "M15 16 L21 22", "M21 16 L15 22"], [], False),
}

STYLES = {
    # fill, stroke, stroke width, icon color
    "glass":         ("#1B2236", "#3A4664", 6, "#3DDCFF"),
    "glass_focus":   ("#25304A", "#FF5A1F", 12, "#FFFFFF"),
    "primary":       ("#FF5A1F", "#FF9268", 6, "#0B0F1A"),
    "primary_focus": ("#FF6E38", "#FFFFFF", 12, "#0B0F1A"),
    "danger":        ("#1B2236", "#3A4664", 6, "#FF7A59"),
}

# output name -> (icon, style)
TILES = {
    "tile_menu_race":        ("play", "primary"),
    "tile_menu_race_focus":  ("play", "primary_focus"),
    "tile_menu_story":       ("book", "glass"),
    "tile_menu_story_focus": ("book", "glass_focus"),
    "tile_crown":            ("trophy", "glass"),
    "tile_main_options":     ("sliders", "glass"),
    "tile_main_help":        ("help", "glass"),
    "tile_main_about":       ("info", "glass"),
    "tile_main_quit":        ("power", "danger"),
    "tile_tutorial":         ("cap", "glass"),
    "tile_players":          ("person", "glass"),
    "tile_start":            ("play", "primary"),
    "tile_check":            ("check", "primary"),
    "tile_cross":            ("cross", "glass"),
    "tile_add":              ("add", "glass"),
    "tile_remove":           ("remove", "danger"),
    "tile_rename":           ("rename", "glass"),
    "tile_edit":             ("edit", "glass"),
    "tile_back":             ("back", "glass"),
    "tile_restart":          ("restart", "glass"),
    "tile_options":          ("sliders", "glass"),
    "tile_quit":             ("home", "glass"),
    "tile_race":             ("flag", "glass"),
    "tile_steering_wheel":   ("wheel", "glass"),
    "tile_accelerator_icon": ("pedal", "glass"),
    "tile_gyroscope_icon":   ("tilt", "glass"),
    "tile_kart":             ("kart", "glass"),
}

# Cartoon icons replaced through the skin (same file names, so no layout changes)
OVERRIDES = {
    "back":                ("back", "glass"),
    "green_check":         ("check", "primary"),
    "red_x":               ("cross", "glass"),
    "restart":             ("restart", "glass"),
    "remove":              ("remove", "danger"),
    "blue_plus":           ("add", "glass"),
    "rename":              ("rename", "glass"),
    "edit":                ("edit", "glass"),
    "main_options":        ("sliders", "glass"),
    "main_help":           ("help", "glass"),
    "main_quit":           ("power", "danger"),
    "race_giveup":         ("giveup", "danger"),
    "story_mode_book":     ("book", "glass"),
    "mode_normal":         ("flag", "glass"),
    "mode_tt":             ("watch", "glass"),
    "mode_ftl":            ("leader", "glass"),
    "mode_weapons":        ("shield", "glass"),
    "difficulty_easy":     ("easy", "glass"),
    "difficulty_medium":   ("medium", "glass"),
    "difficulty_hard":     ("hard", "glass"),
    "difficulty_best":     ("legend", "glass"),
    "track_random":        ("random", "glass"),
    "gp_new":              ("trophy", "glass"),
    "main_race":           ("flag", "glass"),
    "random_kart":         ("random", "glass"),
    "options_audio":       ("audio", "glass"),
    "options_input":       ("pad", "glass"),
    "options_display":     ("camera", "glass"),
    "options_video":       ("screen", "glass"),
    "options_ui":          ("layout", "glass"),
    "options_language":    ("globe", "glass"),
    "options_players":     ("people", "glass"),
}


def scale_path(d, s, off):
    """Scale an absolute M/L/Q/A/Z path; the renderer drops strokes inside transforms."""
    import re
    out, cmd, nums = [], None, []
    tokens = re.findall(r"[MLQAZ]|-?[0-9.]+", d)

    def flush():
        if cmd is None:
            return
        if cmd == "A":
            for i in range(0, len(nums), 7):
                rx, ry, rot, la, sw, x, y = nums[i:i + 7]
                out.append(f"A {rx*s:.1f} {ry*s:.1f} {rot} {int(la)} {int(sw)} {x*s+off:.1f} {y*s+off:.1f}")
        elif cmd == "Z":
            out.append("Z")
        else:
            pts = " ".join(f"{v*s+off:.1f}" for v in nums)
            out.append(f"{cmd} {pts}")

    for t in tokens:
        if t in "MLQAZ":
            flush()
            cmd, nums = t, []
        else:
            nums.append(float(t))
    flush()
    return " ".join(out)


def draw_args(icon, style, k=2):
    """ImageMagick -draw arguments for one tile, drawn at k times 256 px."""
    paths, circles, filled = ICONS[icon]
    fill, stroke, sw, ink = STYLES[style]
    s = 6.4 * k
    off = (256 * k - 24 * s) / 2
    a = ["-size", f"{256*k}x{256*k}", "xc:none",
         "-fill", fill, "-stroke", stroke, "-strokewidth", str(sw * k),
         "-draw", f"roundrectangle {(4+sw/2)*k},{(4+sw/2)*k} {(252-sw/2)*k},{(252-sw/2)*k} {52*k},{52*k}",
         "-stroke", ink, "-strokewidth", f"{2.1*s:.1f}"]
    for d in paths:
        a += ["-fill", ink if filled else "none",
              "-draw", f"stroke-linecap round stroke-linejoin round path '{scale_path(d, s, off)}'"]
    for cx, cy, r, f in circles:
        x, y = cx * s + off, cy * s + off
        if f:
            a += ["-fill", ink, "-stroke", "none", "-draw", f"circle {x:.1f},{y:.1f} {x+r*s:.1f},{y:.1f}", "-stroke", ink]
        else:
            a += ["-fill", "none", "-draw", f"circle {x:.1f},{y:.1f} {x+r*s:.1f},{y:.1f}"]
    return a + ["-resize", "256x256"]


for name, (icon, style) in list(TILES.items()) + list(OVERRIDES.items()):
    if len(sys.argv) > 2 and name not in sys.argv[2:]:
        continue
    subprocess.run(["magick"] + draw_args(icon, style) + ["PNG32:" + os.path.join(OUT, name + ".png")], check=True)
    print(name)
