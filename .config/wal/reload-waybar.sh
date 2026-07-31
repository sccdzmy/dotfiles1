#!/bin/bash
# Reload waybar + niri focus ring with pywal colors

CSS="$HOME/.cache/wal/colors-waybar.css"
COLORS_JSON="$HOME/.cache/wal/colors.json"
NIRI_CFG="$HOME/.config/niri/config.kdl"

# ── Waybar: smart text color ──

BG=$(grep -oP 'rgba\(\K[\d,]+' "$CSS" | head -1)
R=$(echo "$BG" | cut -d, -f1)
G=$(echo "$BG" | cut -d, -f2)
B=$(echo "$BG" | cut -d, -f3)
L=$(python3 -c "print(0.2126*$R/255 + 0.7152*$G/255 + 0.0722*$B/255)")

if python3 -c "exit(0 if $L > 0.5 else 1)" 2>/dev/null; then
    FG="#000000"
else
    FG="#ffffff"
fi

# In-place overwrite (cat >), NOT mv: mv replaces the inode and kills
# waybar's file monitor (reload_style_on_change) for subsequent updates.
sed "s/@define-color fg .*/@define-color fg $FG;/" "$CSS" > "$CSS.tmp"
cat "$CSS.tmp" > "$CSS"
rm -f "$CSS.tmp"

# ── Niri: lighter accent for focus ring ──

FOCUS=$(python3 -c "import json;c=json.load(open('$COLORS_JSON'))['colors'];print(c['color6'])")
if [ -n "$FOCUS" ]; then
    awk -v color="$FOCUS" '
        /pywal-focus/ { gsub(/active-color "[^"]*"/, "active-color \"" color "\"") }
        { print }
    ' "$NIRI_CFG" > "$NIRI_CFG.tmp" && mv "$NIRI_CFG.tmp" "$NIRI_CFG"
    niri msg action load-config-file 2>/dev/null
fi

# Waybar auto-reloads its CSS (and @imports, incl. colors-waybar.css) via
# CssReloadHelper — no SIGUSR2 needed. SIGUSR2 would rebuild the bar window
# (exclusive zone flicker + windows jumping), which is why we don't use it.
# touch style.css below if the helper misses an update:
# touch "$HOME/.config/waybar/style.css"

# ── Mako: reload notification theme ──
# This mako rejects rgba()/opacity; convert bg to 8-digit hex (#RRGGBBD9 ≈ 85%).
MAKO="$HOME/.cache/wal/mako"
BG=$(grep -oP '^background-color=#\K[0-9A-Fa-f]{6}' "$MAKO")
[ -n "$BG" ] && sed -i "s/^background-color=#.*/background-color=#${BG}d9/" "$MAKO"
makoctl reload 2>/dev/null
