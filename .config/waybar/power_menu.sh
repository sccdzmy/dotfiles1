#!/bin/bash
# 电源下拉菜单 — 自编译 GTK layer-shell 窗口（~/.local/bin/power-dropdown）
# 源码与编译命令见 ~/.config/waybar/power_dropdown.c
# 旧 fuzzel 方案（fuzzel 居中弹窗，替代前先删上面的 pkill/exec）：
# CHOICE="$(printf '⏾  Suspend\n↻  Reboot\n⏻  Shutdown\n' | fuzzel --dmenu --prompt 'Power: ' --lines 3 2>/dev/null)"
# case "$CHOICE" in
#   *Suspend*) systemctl suspend ;;
#   *Reboot*) systemctl reboot ;;
#   *Shutdown*) systemctl poweroff ;;
# esac
# 旧 waybar 内置 GTK menu 方案见 config.jsonc 里注释掉的 menu/menu-file/menu-actions

# 已打开则先关掉（避免叠两个菜单）
pkill -x power-dropdown 2>/dev/null
exec "$HOME/.local/bin/power-dropdown"
