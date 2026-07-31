/*
 * power-dropdown — 电源下拉菜单
 * GTK3 + gtk-layer-shell，锚定屏幕右上角（bar 的 ⏻ 按钮正下方）。
 * 点击 ⏻ 弹出 3 个选项：Suspend / Reboot / Shutdown。
 * Esc 或鼠标移出窗口关闭。
 *
 * 编译：
 *   gcc -O2 -Wall power_dropdown.c -o power-dropdown \
 *       $(pkg-config --cflags --libs gtk+-3.0 gtk-layer-shell-0)
 *
 * 位置参数（改这里即可微调）：
 *   MARGIN_TOP  = bar 高度 + 间距（bar 内容驱动高度 ~29px，这里取 32）
 *   MARGIN_RIGHT = 按钮距屏幕右缘距离
 */
#include <gtk/gtk.h>
#include <gtk-layer-shell.h>

#define MARGIN_TOP    27  /* waybar height=30 - 模块 margin 3 = 胶囊底 27 逻辑px，菜单顶紧贴胶囊 */
#define MARGIN_RIGHT  8
#define WIN_WIDTH     220
#define WIN_HEIGHT    126

static void run_cmd(const char *cmd)
{
    char *argv[] = {"/bin/sh", "-c", (char *)cmd, NULL};
    g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
                  NULL, NULL, NULL, NULL);
}

static void on_action(GtkWidget *w, gpointer user_data)
{
    run_cmd((const char *)user_data);
    g_printerr("power-dropdown: close by action (%s)\n", (const char *)user_data);
    gtk_main_quit();
}

/* 纯鼠标：左键按下即执行（EventBox 无键盘语义，回车/空格不可能触发） */
static gboolean on_press(GtkWidget *w, GdkEventButton *ev, gpointer user_data)
{
    if (ev->button == 1 && ev->type == GDK_BUTTON_PRESS)
        on_action(w, user_data);
    return TRUE;
}

static gboolean on_key_press(GtkWidget *w, GdkEventKey *ev, gpointer d)
{
    if (ev->keyval == GDK_KEY_Escape) {
        g_printerr("power-dropdown: close by escape\n");
        gtk_main_quit();
    }
    return TRUE; /* 吞掉一切键盘输入：本菜单只能鼠标操作（防误触休眠/重启） */
}

/* 键盘焦点丢失（即点击了菜单以外的其他表面）→ 关闭 */
static gboolean on_focus_out(GtkWidget *w, GdkEventFocus *ev, gpointer d)
{
    g_printerr("power-dropdown: close by focus-out\n");
    gtk_main_quit();
    return FALSE;
}

/* 从 ~/.cache/wal/colors-waybar.css 读取 @define-color，跟随配色方案 */
static gboolean parse_wal_color(const gchar *content, const gchar *name,
                                gint *r, gint *g, gint *b, gdouble *a)
{
    gchar needle[64];
    g_snprintf(needle, sizeof needle, "@define-color %s ", name);
    const gchar *p = strstr(content, needle);
    if (!p)
        return FALSE;
    p += strlen(needle);
    guint tr = 0, tg = 0, tb = 0;
    if (sscanf(p, "rgba(%u, %u, %u, %lf)", &tr, &tg, &tb, a) == 4) {
        *r = tr; *g = tg; *b = tb;
        return TRUE;
    }
    if (sscanf(p, "#%2x%2x%2x", &tr, &tg, &tb) == 3) {
        *r = tr; *g = tg; *b = tb; *a = 1.0;
        return TRUE;
    }
    return FALSE;
}

/* 生成与 bar 胶囊一致的样式：同 @bg/@fg、同透明度；bar 无 blur 故这里也不加 */
static gchar *wal_css(void)
{
    gint br = 136, bg = 110, bb = 123; gdouble ba = 0.8; /* 兜底 */
    gint fr = 255, fg = 255, fb = 255;
    gchar *path = g_build_filename(g_get_home_dir(),
                                   ".cache/wal/colors-waybar.css", NULL);
    gchar *content = NULL;
    if (g_file_get_contents(path, &content, NULL, NULL)) {
        gdouble fa = 1.0;
        parse_wal_color(content, "bg", &br, &bg, &bb, &ba);
        parse_wal_color(content, "fg", &fr, &fg, &fb, &fa);
    }
    g_free(content);
    g_free(path);
    return g_strdup_printf(
        "window#power-dropdown {"
        "  background-color: rgba(%d, %d, %d, %.2f);"
        "  border-radius: 10px;"
        "  font-family: FontAwesome, Roboto, Helvetica, Arial, sans-serif;"
        "  font-size: 13px;"
        "}"
        "eventbox#power-item { background: transparent; padding: 11px 16px;"
        "  border-radius: 8px; }"
        "eventbox#power-item label { color: rgba(%d, %d, %d, 1.0); }"
        "eventbox#power-item:hover { background-color: rgba(255, 255, 255, 0.12); }"
        "eventbox#power-item:active { background-color: rgba(0, 0, 0, 0.2); }"
        "separator { background-color: rgba(255, 255, 255, 0.08);"
        "  min-height: 1px; }",
        br, bg, bb, ba, fr, fg, fb);
}

int main(int argc, char **argv)
{
    g_set_prgname("power-dropdown");
    gtk_init(&argc, &argv);

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "power-dropdown");
    gtk_window_set_decorated(GTK_WINDOW(win), FALSE);
    gtk_window_set_keep_above(GTK_WINDOW(win), TRUE);
    gtk_window_set_accept_focus(GTK_WINDOW(win), TRUE);

    /* layer-shell：overlay 层，锚定右上，正好在 bar 下面 */
    gtk_layer_init_for_window(GTK_WINDOW(win));
    gtk_layer_set_namespace(GTK_WINDOW(win), "power-dropdown");
    gtk_layer_set_layer(GTK_WINDOW(win), GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_anchor(GTK_WINDOW(win), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(win), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
    gtk_layer_set_margin(GTK_WINDOW(win), GTK_LAYER_SHELL_EDGE_TOP, MARGIN_TOP);
    gtk_layer_set_margin(GTK_WINDOW(win), GTK_LAYER_SHELL_EDGE_RIGHT, MARGIN_RIGHT);
    gtk_layer_set_keyboard_mode(GTK_WINDOW(win),
                                GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND);
    gtk_window_resize(GTK_WINDOW(win), WIN_WIDTH, WIN_HEIGHT);
    /* 关键：GTK3 会用错误的"自然尺寸"覆盖 resize（scale 混乱导致缩到 77x53），
       用 size_request 强制最小尺寸 = 目标尺寸，防止 set_size 被覆盖 */
    gtk_widget_set_size_request(GTK_WIDGET(win), WIN_WIDTH, WIN_HEIGHT);

    /* 样式：动态读取 wal 配色，与 bar 胶囊一致 */
    GtkCssProvider *css = gtk_css_provider_new();
    gchar *css_data = wal_css();
    gtk_css_provider_load_from_data(css, css_data, -1, NULL);
    g_free(css_data);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(), GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(win), box);

    static const struct { const char *label; const char *cmd; } items[] = {
        {"⏾  Suspend",  "systemctl suspend"},
        {"↻  Reboot",   "systemctl reboot"},
        {"⏻  Shutdown", "systemctl poweroff"},
    };
    for (int i = 0; i < 3; i++) {
        if (i > 0) {
            GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
            gtk_box_pack_start(GTK_BOX(box), sep, FALSE, FALSE, 0);
        }
        /* EventBox + Label：无键盘激活语义，只能鼠标点（回车/空格绝对安全） */
        GtkWidget *eb = gtk_event_box_new();
        gtk_widget_set_can_focus(eb, FALSE);
        gtk_widget_set_name(eb, "power-item");
        gtk_widget_add_events(eb, GDK_BUTTON_PRESS_MASK);
        GtkWidget *lbl = gtk_label_new(items[i].label);
        gtk_label_set_xalign(GTK_LABEL(lbl), 0.0);
        gtk_container_add(GTK_CONTAINER(eb), lbl);
        gtk_box_pack_start(GTK_BOX(box), eb, TRUE, TRUE, 0);
        g_signal_connect(eb, "button-press-event", G_CALLBACK(on_press),
                         (gpointer)items[i].cmd);
    }

    g_signal_connect(win, "key-press-event", G_CALLBACK(on_key_press), NULL);
    gtk_widget_add_events(win, GDK_FOCUS_CHANGE_MASK);
    g_signal_connect(win, "focus-out-event", G_CALLBACK(on_focus_out), NULL);

    gtk_widget_set_name(win, "power-dropdown");
    gtk_widget_show_all(win);
    gtk_main();
    return 0;
}
