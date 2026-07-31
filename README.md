# dotfiles

Arch Linux ricing 配置备份（niri + waybar-git + waypaper + pywal）。

## 结构

| 路径 | 内容 |
|---|---|
| `.config/niri/` | niri 窗口管理器（config.kdl + swayidle.sh） |
| `.config/waybar/` | waybar 状态栏（config.jsonc / style.css / power_dropdown.c 源码 / power_menu.sh） |
| `.config/wal/` | pywal 配色模板（templates/）+ 配色刷新脚本 reload-waybar.sh |
| `.config/alacritty/` | 终端 |
| `.config/fish/` | fish shell（config.fish） |
| `.config/mako/` | 通知 daemon |
| `.config/waypaper/` | 壁纸设置 GUI |
| `.config/ulauncher/` | 应用启动器（含自定义主题） |
| `.local/bin/` | awww（壁纸+自动刷配色 wrapper）、awww-set |
| `.bashrc` 等 | bash 基础配置 |

## 换机恢复

```bash
# 1. 安装依赖
sudo pacman -S niri waypaper python-pywal awww mako alacritty fish ulauncher gtk-layer-shell
# AUR: waybar-git（yay -S waybar-git）

# 2. 拉取仓库
git clone --bare <仓库地址> $HOME/.dotfiles
alias dotfiles='git --git-dir=$HOME/.dotfiles --work-tree=$HOME'
dotfiles config --local status.showUntrackedFiles no

# 3. 检出配置（目标文件已存在会报错，先备份再 -f 强制覆盖）
dotfiles checkout -f

# 4. 编译电源菜单（源码在 .config/waybar/power_dropdown.c）
gcc -O2 -Wall power_dropdown.c -o ~/.local/bin/power-dropdown \
    $(pkg-config --cflags --libs gtk+-3.0 gtk-layer-shell-0)
```

壁纸不在此仓库（太大），单独拷贝 `~/Pictures/Wallpapers/` 和 `~/aesthetic-wallpapers/`。

## 日常使用

```bash
alias dotfiles='git --git-dir=$HOME/.dotfiles --work-tree=$HOME'
dotfiles status          # 查看改动
dotfiles add <路径>      # 暂存
dotfiles commit -m "..." # 提交
dotfiles push            # 推送
```

fish 用户把 alias 写进 `~/.config/fish/conf.d/dotfiles.fish`：
`alias dotfiles='git --git-dir=$HOME/.dotfiles --work-tree=$HOME'`

## 注意

- 换机后记得改 git 身份：`git config --global user.name/email`
- `dotfiles add -A` 会扫描整个 HOME，误加无关目录前先看 `dotfiles status`
- fish_variables（含 PATH 等机器相关变量）刻意不进仓库
