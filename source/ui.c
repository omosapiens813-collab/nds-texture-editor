/*
 * ui.c
 * NDS Texture Editor - UI Renderer
 * Usa Mode 5 bitmap (256x192 16bit) su entrambi gli schermi
 */

#include "../include/ui.h"
#include "../include/tex_decoder.h"

/* =========================================================
 * Font 5x8 pixel (ASCII 32-127)
 * Ogni char: 8 byte, ogni byte = 1 riga (5 bit usati)
 * ========================================================= */
const uint8_t ui_font_5x8[128][8] = {
    /* 0x20 SPACE */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 0x21 !     */ {0x04,0x04,0x04,0x04,0x00,0x04,0x00,0x00},
    /* 0x22 "     */ {0x0A,0x0A,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 0x23 #     */ {0x0A,0x1F,0x0A,0x0A,0x1F,0x0A,0x00,0x00},
    /* 0x24 $     */ {0x04,0x0F,0x14,0x0E,0x05,0x1E,0x04,0x00},
    /* 0x25 %     */ {0x18,0x19,0x02,0x04,0x08,0x13,0x03,0x00},
    /* 0x26 &     */ {0x0C,0x12,0x14,0x08,0x15,0x12,0x0D,0x00},
    /* 0x27 '     */ {0x06,0x04,0x08,0x00,0x00,0x00,0x00,0x00},
    /* 0x28 (     */ {0x02,0x04,0x08,0x08,0x08,0x04,0x02,0x00},
    /* 0x29 )     */ {0x08,0x04,0x02,0x02,0x02,0x04,0x08,0x00},
    /* 0x2A *     */ {0x00,0x04,0x15,0x0E,0x15,0x04,0x00,0x00},
    /* 0x2B +     */ {0x00,0x04,0x04,0x1F,0x04,0x04,0x00,0x00},
    /* 0x2C ,     */ {0x00,0x00,0x00,0x00,0x06,0x04,0x08,0x00},
    /* 0x2D -     */ {0x00,0x00,0x00,0x1F,0x00,0x00,0x00,0x00},
    /* 0x2E .     */ {0x00,0x00,0x00,0x00,0x00,0x06,0x06,0x00},
    /* 0x2F /     */ {0x01,0x01,0x02,0x04,0x08,0x10,0x10,0x00},
    /* 0x30 0     */ {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E,0x00},
    /* 0x31 1     */ {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E,0x00},
    /* 0x32 2     */ {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F,0x00},
    /* 0x33 3     */ {0x1F,0x02,0x04,0x02,0x01,0x11,0x0E,0x00},
    /* 0x34 4     */ {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02,0x00},
    /* 0x35 5     */ {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E,0x00},
    /* 0x36 6     */ {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E,0x00},
    /* 0x37 7     */ {0x1F,0x01,0x02,0x04,0x08,0x08,0x08,0x00},
    /* 0x38 8     */ {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E,0x00},
    /* 0x39 9     */ {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C,0x00},
    /* 0x3A :     */ {0x00,0x06,0x06,0x00,0x06,0x06,0x00,0x00},
    /* 0x3B ;     */ {0x00,0x06,0x06,0x00,0x06,0x04,0x08,0x00},
    /* 0x3C <     */ {0x02,0x04,0x08,0x10,0x08,0x04,0x02,0x00},
    /* 0x3D =     */ {0x00,0x00,0x1F,0x00,0x1F,0x00,0x00,0x00},
    /* 0x3E >     */ {0x08,0x04,0x02,0x01,0x02,0x04,0x08,0x00},
    /* 0x3F ?     */ {0x0E,0x11,0x01,0x02,0x04,0x00,0x04,0x00},
    /* 0x40 @     */ {0x0E,0x11,0x01,0x0D,0x15,0x15,0x0E,0x00},
    /* 0x41 A     */ {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11,0x00},
    /* 0x42 B     */ {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E,0x00},
    /* 0x43 C     */ {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E,0x00},
    /* 0x44 D     */ {0x1C,0x12,0x11,0x11,0x11,0x12,0x1C,0x00},
    /* 0x45 E     */ {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F,0x00},
    /* 0x46 F     */ {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10,0x00},
    /* 0x47 G     */ {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F,0x00},
    /* 0x48 H     */ {0x11,0x11,0x11,0x1F,0x11,0x11,0x11,0x00},
    /* 0x49 I     */ {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E,0x00},
    /* 0x4A J     */ {0x07,0x02,0x02,0x02,0x02,0x12,0x0C,0x00},
    /* 0x4B K     */ {0x11,0x12,0x14,0x18,0x14,0x12,0x11,0x00},
    /* 0x4C L     */ {0x10,0x10,0x10,0x10,0x10,0x10,0x1F,0x00},
    /* 0x4D M     */ {0x11,0x1B,0x15,0x15,0x11,0x11,0x11,0x00},
    /* 0x4E N     */ {0x11,0x11,0x19,0x15,0x13,0x11,0x11,0x00},
    /* 0x4F O     */ {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E,0x00},
    /* 0x50 P     */ {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10,0x00},
    /* 0x51 Q     */ {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D,0x00},
    /* 0x52 R     */ {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11,0x00},
    /* 0x53 S     */ {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E,0x00},
    /* 0x54 T     */ {0x1F,0x04,0x04,0x04,0x04,0x04,0x04,0x00},
    /* 0x55 U     */ {0x11,0x11,0x11,0x11,0x11,0x11,0x0E,0x00},
    /* 0x56 V     */ {0x11,0x11,0x11,0x11,0x11,0x0A,0x04,0x00},
    /* 0x57 W     */ {0x11,0x11,0x11,0x15,0x15,0x1B,0x11,0x00},
    /* 0x58 X     */ {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11,0x00},
    /* 0x59 Y     */ {0x11,0x11,0x0A,0x04,0x04,0x04,0x04,0x00},
    /* 0x5A Z     */ {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F,0x00},
    /* 0x5B [     */ {0x0E,0x08,0x08,0x08,0x08,0x08,0x0E,0x00},
    /* 0x5C \     */ {0x10,0x10,0x08,0x04,0x02,0x01,0x01,0x00},
    /* 0x5D ]     */ {0x0E,0x02,0x02,0x02,0x02,0x02,0x0E,0x00},
    /* 0x5E ^     */ {0x04,0x0A,0x11,0x00,0x00,0x00,0x00,0x00},
    /* 0x5F _     */ {0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x00},
    /* 0x60 `     */ {0x08,0x04,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 0x61 a     */ {0x00,0x00,0x0E,0x01,0x0F,0x11,0x0F,0x00},
    /* 0x62 b     */ {0x10,0x10,0x16,0x19,0x11,0x11,0x1E,0x00},
    /* 0x63 c     */ {0x00,0x00,0x0E,0x10,0x10,0x11,0x0E,0x00},
    /* 0x64 d     */ {0x01,0x01,0x0D,0x13,0x11,0x11,0x0F,0x00},
    /* 0x65 e     */ {0x00,0x00,0x0E,0x11,0x1F,0x10,0x0E,0x00},
    /* 0x66 f     */ {0x06,0x09,0x08,0x1C,0x08,0x08,0x08,0x00},
    /* 0x67 g     */ {0x00,0x00,0x0F,0x11,0x11,0x0F,0x01,0x0E},
    /* 0x68 h     */ {0x10,0x10,0x16,0x19,0x11,0x11,0x11,0x00},
    /* 0x69 i     */ {0x04,0x00,0x0C,0x04,0x04,0x04,0x0E,0x00},
    /* 0x6A j     */ {0x02,0x00,0x06,0x02,0x02,0x02,0x12,0x0C},
    /* 0x6B k     */ {0x10,0x10,0x12,0x14,0x18,0x14,0x12,0x00},
    /* 0x6C l     */ {0x0C,0x04,0x04,0x04,0x04,0x04,0x0E,0x00},
    /* 0x6D m     */ {0x00,0x00,0x1A,0x15,0x15,0x11,0x11,0x00},
    /* 0x6E n     */ {0x00,0x00,0x16,0x19,0x11,0x11,0x11,0x00},
    /* 0x6F o     */ {0x00,0x00,0x0E,0x11,0x11,0x11,0x0E,0x00},
    /* 0x70 p     */ {0x00,0x00,0x1E,0x11,0x11,0x1E,0x10,0x10},
    /* 0x71 q     */ {0x00,0x00,0x0F,0x11,0x11,0x0F,0x01,0x01},
    /* 0x72 r     */ {0x00,0x00,0x16,0x19,0x10,0x10,0x10,0x00},
    /* 0x73 s     */ {0x00,0x00,0x0E,0x10,0x0E,0x01,0x1E,0x00},
    /* 0x74 t     */ {0x08,0x08,0x1C,0x08,0x08,0x09,0x06,0x00},
    /* 0x75 u     */ {0x00,0x00,0x11,0x11,0x11,0x13,0x0D,0x00},
    /* 0x76 v     */ {0x00,0x00,0x11,0x11,0x11,0x0A,0x04,0x00},
    /* 0x77 w     */ {0x00,0x00,0x11,0x11,0x15,0x15,0x0A,0x00},
    /* 0x78 x     */ {0x00,0x00,0x11,0x0A,0x04,0x0A,0x11,0x00},
    /* 0x79 y     */ {0x00,0x00,0x11,0x11,0x0F,0x01,0x0E,0x00},
    /* 0x7A z     */ {0x00,0x00,0x1F,0x02,0x04,0x08,0x1F,0x00},
    /* 0x7B {     */ {0x03,0x04,0x04,0x08,0x04,0x04,0x03,0x00},
    /* 0x7C |     */ {0x04,0x04,0x04,0x00,0x04,0x04,0x04,0x00},
    /* 0x7D }     */ {0x18,0x04,0x04,0x02,0x04,0x04,0x18,0x00},
    /* 0x7E ~     */ {0x08,0x15,0x02,0x00,0x00,0x00,0x00,0x00},
    /* 0x7F DEL   */ {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x00},
};

/* =========================================================
 * Framebuffer helper
 * ========================================================= */
static inline uint16_t* get_fb(bool top) {
    return top ? (uint16_t*)BG_BMP_RAM(0)
               : (uint16_t*)BG_BMP_RAM_SUB(0);
}

/* =========================================================
 * Inizializzazione video
 * ========================================================= */
void ui_init_video(void) {
    /* Schermo superiore: Mode 5, BG2 bitmap 16bit */
    videoSetMode(MODE_5_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    bgInit(2, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    bgSetPriority(2, 3);

    /* Schermo inferiore: Mode 5, BG2 bitmap 16bit */
    videoSetModeSub(MODE_5_2D);
    vramSetBankC(VRAM_C_SUB_BG);
    bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    bgSetPriority(2, 3);
}

/* =========================================================
 * Primitivi grafici
 * ========================================================= */
void ui_draw_pixel(bool top, int x, int y, uint16_t color) {
    if (x < 0 || x >= NDS_SCREEN_W || y < 0 || y >= NDS_SCREEN_H) return;
    get_fb(top)[y * NDS_SCREEN_W + x] = color | BIT(15);
}

void ui_fill_rect(bool top, int x, int y, int w, int h, uint16_t color) {
    uint16_t c = color | BIT(15);
    uint16_t* fb = get_fb(top);
    for (int ry = y; ry < y + h; ry++) {
        if (ry < 0 || ry >= NDS_SCREEN_H) continue;
        for (int rx = x; rx < x + w; rx++) {
            if (rx < 0 || rx >= NDS_SCREEN_W) continue;
            fb[ry * NDS_SCREEN_W + rx] = c;
        }
    }
}

void ui_draw_rect(bool top, int x, int y, int w, int h, uint16_t color) {
    uint16_t c = color | BIT(15);
    uint16_t* fb = get_fb(top);
    /* Top/Bottom */
    for (int rx = x; rx < x + w; rx++) {
        if (rx >= 0 && rx < NDS_SCREEN_W) {
            if (y >= 0 && y < NDS_SCREEN_H)         fb[y * NDS_SCREEN_W + rx] = c;
            if (y+h-1 >= 0 && y+h-1 < NDS_SCREEN_H) fb[(y+h-1)*NDS_SCREEN_W+rx] = c;
        }
    }
    /* Left/Right */
    for (int ry = y; ry < y + h; ry++) {
        if (ry >= 0 && ry < NDS_SCREEN_H) {
            if (x >= 0 && x < NDS_SCREEN_W)         fb[ry * NDS_SCREEN_W + x] = c;
            if (x+w-1 >= 0 && x+w-1 < NDS_SCREEN_W) fb[ry*NDS_SCREEN_W+x+w-1] = c;
        }
    }
}

/* =========================================================
 * Font rendering 5x8
 * ========================================================= */
void ui_draw_char(bool top, int x, int y, char c, uint16_t fg, uint16_t bg) {
    uint8_t idx = (uint8_t)c;
    if (idx < 0x20 || idx > 0x7F) idx = 0x20;
    idx -= 0x20;
    const uint8_t* glyph = ui_font_5x8[idx];
    uint16_t fg_c = fg | BIT(15);
    uint16_t bg_c = bg | BIT(15);

    for (int row = 0; row < 8; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 5; col++) {
            int px = x + col;
            int py = y + row;
            if (px >= 0 && px < NDS_SCREEN_W && py >= 0 && py < NDS_SCREEN_H) {
                get_fb(top)[py * NDS_SCREEN_W + px] =
                    (bits & (1 << (4 - col))) ? fg_c : bg_c;
            }
        }
    }
}

void ui_draw_string(bool top, int x, int y, const char* str, uint16_t fg, uint16_t bg) {
    int cx = x;
    for (int i = 0; str[i]; i++) {
        ui_draw_char(top, cx, y, str[i], fg, bg);
        cx += 6; /* 5px char + 1px spacing */
        if (cx >= NDS_SCREEN_W - 5) break;
    }
}

void ui_draw_string_center(bool top, int y, const char* str, uint16_t fg, uint16_t bg) {
    int len = 0;
    while (str[len]) len++;
    int x = (NDS_SCREEN_W - len * 6) / 2;
    if (x < 0) x = 0;
    ui_draw_string(top, x, y, str, fg, bg);
}

/* =========================================================
 * Status bar (in basso su ogni schermo)
 * ========================================================= */
void ui_draw_status_bar(bool top, const char* msg, uint16_t bg_col) {
    ui_fill_rect(top, 0, NDS_SCREEN_H - 10, NDS_SCREEN_W, 10, bg_col);
    ui_draw_string(top, 2, NDS_SCREEN_H - 9, msg, COL_WHITE, bg_col);
}

/* =========================================================
 * Header bar
 * ========================================================= */
static void draw_header(bool top, const char* title) {
    ui_fill_rect(top, 0, 0, NDS_SCREEN_W, 12, COL_DPURPLE);
    ui_draw_rect(top, 0, 0, NDS_SCREEN_W, 12, COL_NEON_CYAN);
    ui_draw_string_center(top, 2, title, COL_NEON_CYAN, COL_DPURPLE);
}

/* =========================================================
 * FILE BROWSER
 * ========================================================= */
void ui_draw_file_browser(AppContext* ctx) {
    /* ------ Schermo SUPERIORE: titolo + info ---- */
    ui_fill_rect(true, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, COL_DARKGRAY);
    draw_header(true, "NDS TEXTURE EDITOR v1.0");

    ui_draw_string(true, 4, 16, "Seleziona file .nds:", COL_NEON_CYAN, COL_DARKGRAY);
    ui_draw_string(true, 4, 26, "Dir: ", COL_LIGHTGRAY, COL_DARKGRAY);

    /* Mostra path corrente (tronca) */
    char short_path[32];
    int plen = strlen(ctx->current_dir);
    if (plen > 26) {
        strncpy(short_path, "...", 3);
        strncpy(short_path + 3, ctx->current_dir + plen - 23, 23);
        short_path[26] = '\0';
    } else {
        strncpy(short_path, ctx->current_dir, 31);
        short_path[31] = '\0';
    }
    ui_draw_string(true, 34, 26, short_path, COL_WHITE, COL_DARKGRAY);

    /* Legenda tasti */
    ui_fill_rect(true, 0, NDS_SCREEN_H - 34, NDS_SCREEN_W, 24, COL_PURPLE);
    ui_draw_rect(true, 0, NDS_SCREEN_H - 34, NDS_SCREEN_W, 24, COL_NEON_PURPLE);
    ui_draw_string(true, 4, NDS_SCREEN_H - 32, "UP/DOWN: naviga", COL_LIGHTGRAY, COL_PURPLE);
    ui_draw_string(true, 4, NDS_SCREEN_H - 22, "A: apri   B: torna su", COL_LIGHTGRAY, COL_PURPLE);

    ui_draw_status_bar(true, "Touch=seleziona file NDS", COL_DPURPLE);

    /* ------ Schermo INFERIORE: lista file ------- */
    ui_fill_rect(false, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, COL_DARKGRAY);
    draw_header(false, "FILE BROWSER");

    int list_y = 14;
    int item_h = 12;
    int visible = (NDS_SCREEN_H - list_y - 10) / item_h;

    for (int i = 0; i < visible && (ctx->file_scroll + i) < (int)ctx->file_count; i++) {
        int idx = ctx->file_scroll + i;
        bool selected = (idx == ctx->file_cursor);

        uint16_t bg = selected ? COL_ACCENT    : ((i % 2) ? COL_DARKGRAY : COL_GRAY);
        uint16_t fg = selected ? COL_WHITE     : COL_LIGHTGRAY;

        ui_fill_rect(false, 0, list_y + i * item_h, NDS_SCREEN_W, item_h, bg);

        /* Icona tipo file */
        const char* fname = ctx->file_list[idx];
        const char* dot = strrchr(fname, '.');
        bool is_nds = (dot && (strcmp(dot, ".nds") == 0 || strcmp(dot, ".NDS") == 0));
        bool is_dir = (fname[0] == '['); /* directory mostrata come [nome] */

        if (is_dir)       ui_draw_string(false, 2, list_y + i * item_h + 2, "D", COL_YELLOW, bg);
        else if (is_nds)  ui_draw_string(false, 2, list_y + i * item_h + 2, "*", COL_NEON_CYAN, bg);
        else              ui_draw_string(false, 2, list_y + i * item_h + 2, " ", fg, bg);

        /* Nome file (tronca a 36 char) */
        char disp[38];
        strncpy(disp, fname, 37);
        disp[37] = '\0';
        ui_draw_string(false, 10, list_y + i * item_h + 2, disp, fg, bg);
    }

    if (ctx->file_count == 0) {
        ui_draw_string_center(false, 80, "Nessun file trovato", COL_GRAY, COL_DARKGRAY);
        ui_draw_string_center(false, 92, "Inserisci scheda SD", COL_GRAY, COL_DARKGRAY);
    }

    ui_draw_status_bar(false, "Tocca file per selezionare", COL_DPURPLE);
}

/* =========================================================
 * NITRO BROWSER - lista file nel NitroFS
 * ========================================================= */
void ui_draw_nitro_browser(AppContext* ctx) {
    /* Superiore: info ROM */
    ui_fill_rect(true, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, COL_DARKGRAY);
    draw_header(true, "NitroFS BROWSER");

    char buf[48];
    snprintf(buf, 48, "ROM: %.12s  [%.4s]",
             ctx->header.game_title, ctx->header.game_code);
    ui_draw_string(true, 4, 16, buf, COL_NEON_CYAN, COL_DARKGRAY);

    snprintf(buf, 48, "File: %lu  FAT: %lu",
             (unsigned long)ctx->nitro_file_count,
             (unsigned long)ctx->fat_count);
    ui_draw_string(true, 4, 28, buf, COL_LIGHTGRAY, COL_DARKGRAY);

    ui_fill_rect(true, 0, NDS_SCREEN_H - 22, NDS_SCREEN_W, 22, COL_PURPLE);
    ui_draw_string(true, 4, NDS_SCREEN_H - 20, "A: scan texture  B: indietro", COL_LIGHTGRAY, COL_PURPLE);
    ui_draw_string(true, 4, NDS_SCREEN_H - 11, "SELECT: info ROM", COL_LIGHTGRAY, COL_PURPLE);

    ui_draw_status_bar(true, ctx->status_msg[0] ? ctx->status_msg : "NitroFS", COL_DPURPLE);

    /* Inferiore: lista file nitro */
    ui_fill_rect(false, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, COL_DARKGRAY);
    draw_header(false, "FILE NDS");

    int list_y = 14, item_h = 11;
    int visible = (NDS_SCREEN_H - list_y - 10) / item_h;

    for (int i = 0; i < visible && (ctx->nitro_scroll + i) < (int)ctx->nitro_file_count; i++) {
        int idx = ctx->nitro_scroll + i;
        bool sel = (idx == ctx->nitro_cursor);
        NitroFile* nf = &ctx->nitro_files[idx];

        uint16_t bg = sel ? COL_ACCENT : ((i % 2) ? COL_DARKGRAY : COL_GRAY);
        uint16_t fg = sel ? COL_WHITE  : COL_LIGHTGRAY;

        ui_fill_rect(false, 0, list_y + i * item_h, NDS_SCREEN_W, item_h, bg);

        /* Tipo */
        if (nf->is_dir) {
            ui_draw_string(false, 2, list_y + i * item_h + 1, "DIR", COL_YELLOW, bg);
        } else {
            char id_str[8];
            snprintf(id_str, 8, "%04X", nf->file_id);
            ui_draw_string(false, 2, list_y + i * item_h + 1, id_str, COL_CYAN, bg);
        }

        /* Nome */
        char disp[30];
        strncpy(disp, nf->name, 29); disp[29] = '\0';
        ui_draw_string(false, 32, list_y + i * item_h + 1, disp, fg, bg);

        /* Dimensione */
        if (!nf->is_dir && nf->size > 0) {
            char sz[10];
            if (nf->size >= 1024)
                snprintf(sz, 10, "%luK", (unsigned long)(nf->size / 1024));
            else
                snprintf(sz, 10, "%luB", (unsigned long)nf->size);
            ui_draw_string(false, NDS_SCREEN_W - 36, list_y + i * item_h + 1, sz,
                           COL_GRAY, bg);
        }
    }

    if (ctx->nitro_file_count == 0) {
        ui_draw_string_center(false, 80, "Premi A per scansionare", COL_GRAY, COL_DARKGRAY);
    }

    ui_draw_status_bar(false, "UP/DOWN:naviga  A:scan tex", COL_DPURPLE);
}

/* =========================================================
 * TEXTURE LIST
 * ========================================================= */
void ui_draw_texture_list(AppContext* ctx) {
    /* Superiore: info ROM */
    ui_fill_rect(true, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, COL_DARKGRAY);
    draw_header(true, "TEXTURE TROVATE");

    char buf[48];
    snprintf(buf, 48, "%lu texture nel file NDS", (unsigned long)ctx->texture_count);
    ui_draw_string(true, 4, 16, buf, COL_NEON_CYAN, COL_DARKGRAY);

    /* Preview texture selezionata */
    if (ctx->tex_list_cursor >= 0 && ctx->tex_list_cursor < (int)ctx->texture_count) {
        NDSTexture* tex = &ctx->textures[ctx->tex_list_cursor];
        snprintf(buf, 48, "Nome: %.20s", tex->name);
        ui_draw_string(true, 4, 30, buf, COL_WHITE, COL_DARKGRAY);
        snprintf(buf, 48, "Fmt: %-8s  %dx%d",
                 nds_texture_format_name(tex->format), tex->width, tex->height);
        ui_draw_string(true, 4, 42, buf, COL_LIGHTGRAY, COL_DARKGRAY);
        snprintf(buf, 48, "Offset: 0x%08lX  %lu byte",
                 (unsigned long)tex->data_offset, (unsigned long)tex->data_size);
        ui_draw_string(true, 4, 54, buf, COL_GRAY, COL_DARKGRAY);

        /* Mini-preview texture se caricata */
        if (tex->loaded) {
            int px_x = 100, px_y = 30, px_sz = 3;
            int preview_w = 48, preview_h = 48;
            for (int ry = 0; ry < preview_h / px_sz; ry++) {
                for (int rx = 0; rx < preview_w / px_sz; rx++) {
                    uint16_t col = tex_get_pixel(tex, rx, ry);
                    ui_fill_rect(true, px_x + rx * px_sz, px_y + ry * px_sz,
                                 px_sz, px_sz, col);
                }
            }
            ui_draw_rect(true, px_x - 1, px_y - 1, preview_w + 2, preview_h + 2, COL_NEON_CYAN);
        }
    }

    ui_fill_rect(true, 0, NDS_SCREEN_H - 22, NDS_SCREEN_W, 22, COL_PURPLE);
    ui_draw_string(true, 4, NDS_SCREEN_H - 20, "A: visualizza  B: indietro", COL_LIGHTGRAY, COL_PURPLE);
    ui_draw_string(true, 4, NDS_SCREEN_H - 11, "X: carica  Y: salva tutte", COL_LIGHTGRAY, COL_PURPLE);

    ui_draw_status_bar(true, ctx->status_msg[0] ? ctx->status_msg : "Seleziona texture", COL_DPURPLE);

    /* Inferiore: lista texture */
    ui_fill_rect(false, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, COL_DARKGRAY);
    draw_header(false, "LISTA TEXTURE");

    int list_y = 14, item_h = 13;
    int visible = (NDS_SCREEN_H - list_y - 10) / item_h;

    for (int i = 0; i < visible && (ctx->tex_list_scroll + i) < (int)ctx->texture_count; i++) {
        int idx = ctx->tex_list_scroll + i;
        bool sel = (idx == ctx->tex_list_cursor);
        NDSTexture* tex = &ctx->textures[idx];

        uint16_t bg = sel ? COL_ACCENT : ((i % 2) ? COL_DARKGRAY : COL_GRAY);
        uint16_t fg = sel ? COL_WHITE  : COL_LIGHTGRAY;

        ui_fill_rect(false, 0, list_y + i * item_h, NDS_SCREEN_W, item_h, bg);

        /* Numero */
        char num[5];
        snprintf(num, 5, "%3d", idx);
        ui_draw_string(false, 2, list_y + i * item_h + 2, num, COL_CYAN, bg);

        /* Nome */
        char disp[22];
        strncpy(disp, tex->name, 21); disp[21] = '\0';
        ui_draw_string(false, 22, list_y + i * item_h + 2, disp, fg, bg);

        /* Formato */
        ui_draw_string(false, 160, list_y + i * item_h + 2,
                       nds_texture_format_name(tex->format), COL_YELLOW, bg);

        /* Indicatore caricata */
        if (tex->loaded) {
            ui_draw_string(false, 210, list_y + i * item_h + 2, "[OK]", COL_GREEN, bg);
        }
    }

    if (ctx->texture_count == 0) {
        ui_draw_string_center(false, 80, "Nessuna texture trovata", COL_GRAY, COL_DARKGRAY);
        ui_draw_string_center(false, 92, "Premi B e riscansiona", COL_GRAY, COL_DARKGRAY);
    }

    ui_draw_status_bar(false, "UP/DOWN  A:apri  X:carica", COL_DPURPLE);
}

/* =========================================================
 * TEXTURE VIEWER
 * ========================================================= */
void ui_draw_texture_view(AppContext* ctx) {
    NDSTexture* tex = (ctx->selected_texture >= 0 && ctx->selected_texture < (int)ctx->texture_count)
                       ? &ctx->textures[ctx->selected_texture] : NULL;

    /* Superiore: rendering texture */
    ui_fill_rect(true, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, COL_DARKGRAY);

    if (tex && tex->loaded) {
        /* Render texture con zoom */
        tex_render(tex,
                   0, 12,
                   ctx->view_offset_x, ctx->view_offset_y,
                   ctx->view_zoom,
                   true);
        draw_header(true, tex->name);

        /* Info overlay */
        char buf[32];
        snprintf(buf, 32, "%dx%d z:%d", tex->width, tex->height, ctx->view_zoom);
        ui_fill_rect(true, 0, NDS_SCREEN_H - 10, NDS_SCREEN_W, 10, COL_DPURPLE);
        ui_draw_string(true, 2, NDS_SCREEN_H - 9, buf, COL_WHITE, COL_DPURPLE);
    } else {
        draw_header(true, "TEXTURE VIEWER");
        ui_draw_string_center(true, 80, "Texture non caricata", COL_GRAY, COL_DARKGRAY);
        ui_draw_string_center(true, 94, "Premi X per caricare", COL_GRAY, COL_DARKGRAY);
    }

    /* Inferiore: controlli */
    ui_fill_rect(false, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, COL_DPURPLE);
    draw_header(false, "CONTROLLI VIEWER");

    ui_draw_string(false, 4,  16, "D-Pad: muovi vista", COL_LIGHTGRAY, COL_DPURPLE);
    ui_draw_string(false, 4,  28, "L/R: zoom -/+",      COL_LIGHTGRAY, COL_DPURPLE);
    ui_draw_string(false, 4,  40, "A:   editor pixel",  COL_NEON_CYAN, COL_DPURPLE);
    ui_draw_string(false, 4,  52, "X:   carica texture",COL_LIGHTGRAY, COL_DPURPLE);
    ui_draw_string(false, 4,  64, "Y:   palette editor",COL_YELLOW,    COL_DPURPLE);
    ui_draw_string(false, 4,  76, "B:   torna lista",   COL_LIGHTGRAY, COL_DPURPLE);

    if (tex) {
        char buf[40];
        ui_fill_rect(false, 0, 100, NDS_SCREEN_W, 60, COL_GRAY);
        ui_draw_rect(false, 0, 100, NDS_SCREEN_W, 60, COL_NEON_PURPLE);
        snprintf(buf, 40, "File: %.20s", tex->name);
        ui_draw_string(false, 4, 104, buf, COL_WHITE, COL_GRAY);
        snprintf(buf, 40, "Fmt:  %s", nds_texture_format_name(tex->format));
        ui_draw_string(false, 4, 116, buf, COL_YELLOW, COL_GRAY);
        snprintf(buf, 40, "Dim:  %dx%d px", tex->width, tex->height);
        ui_draw_string(false, 4, 128, buf, COL_CYAN, COL_GRAY);
        snprintf(buf, 40, "Off:  0x%08lX", (unsigned long)tex->data_offset);
        ui_draw_string(false, 4, 140, buf, COL_LIGHTGRAY, COL_GRAY);
    }

    ui_draw_status_bar(false, "D-Pad muove  L/R zoom  A:edit", COL_DPURPLE);
}

/* =========================================================
 * PIXEL EDITOR
 * ========================================================= */
void ui_draw_pixel_editor(AppContext* ctx) {
    NDSTexture* tex = (ctx->selected_texture >= 0 && ctx->selected_texture < (int)ctx->texture_count)
                       ? &ctx->textures[ctx->selected_texture] : NULL;

    /* ------ Superiore: canvas pixel editor ---- */
    ui_fill_rect(true, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, RGB15(6,6,6));

    if (!tex || !tex->loaded) {
        ui_draw_string_center(true, 90, "Nessuna texture caricata", COL_RED, RGB15(6,6,6));
        return;
    }

    draw_header(true, "PIXEL EDITOR");

    int zoom = ctx->px_zoom;  /* Pixel size: 1,2,4,8 */
    int off_x = ctx->px_offset_x;
    int off_y = ctx->px_offset_y;
    int canvas_x = 2, canvas_y = 14;
    int canvas_w = NDS_SCREEN_W - 4;
    int canvas_h = NDS_SCREEN_H - 24;

    /* Disegna area canvas con checker background */
    for (int py = 0; py < canvas_h; py++) {
        for (int px = 0; px < canvas_w; px++) {
            int tx = off_x + px / zoom;
            int ty = off_y + py / zoom;
            uint16_t col;
            if (tx >= 0 && tx < tex->width && ty >= 0 && ty < tex->height) {
                col = tex_get_pixel(tex, tx, ty);
            } else {
                /* Checker per area fuori texture */
                col = ((px / 4 + py / 4) % 2) ? RGB15(12,12,12) : RGB15(8,8,8);
            }
            ui_draw_pixel(true, canvas_x + px, canvas_y + py, col);
        }
    }

    /* Cursore pixel (con bordo colorato) */
    int cur_px = canvas_x + (ctx->px_cursor_x - off_x) * zoom;
    int cur_py = canvas_y + (ctx->px_cursor_y - off_y) * zoom;
    if (cur_px >= canvas_x && cur_px < canvas_x + canvas_w - zoom &&
        cur_py >= canvas_y && cur_py < canvas_y + canvas_h - zoom) {
        ui_draw_rect(true, cur_px - 1, cur_py - 1, zoom + 2, zoom + 2, COL_WHITE);
        ui_draw_rect(true, cur_px - 2, cur_py - 2, zoom + 4, zoom + 4, COL_NEON_CYAN);
    }

    /* Griglia (solo se zoom >= 4) */
    if (zoom >= 4) {
        for (int gx = 0; gx < canvas_w; gx += zoom) {
            for (int gy = canvas_y; gy < canvas_y + canvas_h; gy++) {
                uint16_t* fb = get_fb(true);
                if (canvas_x + gx < NDS_SCREEN_W && gy < NDS_SCREEN_H) {
                    uint16_t col = fb[gy * NDS_SCREEN_W + canvas_x + gx];
                    /* Mescola con grigio scuro */
                    fb[gy * NDS_SCREEN_W + canvas_x + gx] =
                        RGB15(((col & 0x1F) >> 1),
                              (((col>>5) & 0x1F) >> 1),
                              (((col>>10) & 0x1F) >> 1)) | BIT(15);
                }
            }
        }
    }

    /* Info pixel corrente */
    uint16_t cur_col = tex_get_pixel(tex, ctx->px_cursor_x, ctx->px_cursor_y);
    uint8_t r, g, b;
    bgr15_to_rgb(cur_col, &r, &g, &b);
    char buf[40];
    snprintf(buf, 40, "x:%d y:%d  z:%d  [%d,%d,%d]",
             ctx->px_cursor_x, ctx->px_cursor_y, zoom, r>>3, g>>3, b>>3);
    ui_fill_rect(true, 0, NDS_SCREEN_H - 10, NDS_SCREEN_W, 10, COL_DPURPLE);
    ui_draw_string(true, 2, NDS_SCREEN_H - 9, buf, COL_WHITE, COL_DPURPLE);

    /* ------ Inferiore: palette colori + controlli ---- */
    ui_fill_rect(false, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, COL_DARKGRAY);
    draw_header(false, "PALETTE");

    /* Disegna palette */
    int pal_cols = 16;
    int pal_rows = (tex->format == TEX_256COLOR) ? 16 : 4;
    int sw = NDS_SCREEN_W / pal_cols;
    int sh = 10;

    for (int row = 0; row < pal_rows; row++) {
        for (int col = 0; col < pal_cols; col++) {
            int idx = row * pal_cols + col;
            if (idx >= MAX_PALETTE_SIZE) break;
            bool sel = (idx == ctx->selected_color);
            ui_fill_rect(false, col * sw, 14 + row * sh, sw, sh, tex->palette[idx]);
            if (sel) {
                ui_draw_rect(false, col * sw, 14 + row * sh, sw, sh, COL_WHITE);
            }
        }
    }

    /* Colore selezionato preview */
    int pal_bottom = 14 + pal_rows * sh + 4;
    ui_fill_rect(false, 0, pal_bottom, NDS_SCREEN_W, 24, COL_GRAY);
    ui_draw_rect(false, 0, pal_bottom, NDS_SCREEN_W, 24, COL_NEON_PURPLE);

    /* Swatch colore corrente */
    uint16_t sel_col = tex->palette[ctx->selected_color];
    ui_fill_rect(false, 4, pal_bottom + 3, 18, 18, sel_col);
    ui_draw_rect(false, 4, pal_bottom + 3, 18, 18, COL_WHITE);

    bgr15_to_rgb(sel_col, &r, &g, &b);
    char col_info[32];
    snprintf(col_info, 32, "Idx:%d  R:%d G:%d B:%d",
             ctx->selected_color, r>>3, g>>3, b>>3);
    ui_draw_string(false, 26, pal_bottom + 4, col_info, COL_WHITE, COL_GRAY);

    /* Controlli */
    ui_fill_rect(false, 0, NDS_SCREEN_H - 32, NDS_SCREEN_W, 22, COL_PURPLE);
    ui_draw_rect(false, 0, NDS_SCREEN_H - 32, NDS_SCREEN_W, 22, COL_NEON_PURPLE);
    ui_draw_string(false, 2, NDS_SCREEN_H - 30, "D-Pad:muovi  A:disegna  B:esci",
                   COL_LIGHTGRAY, COL_PURPLE);
    ui_draw_string(false, 2, NDS_SCREEN_H - 20, "L/R:zoom  X:salva  Y:palette",
                   COL_LIGHTGRAY, COL_PURPLE);

    ui_draw_status_bar(false, "Touch palette=seleziona colore", COL_DPURPLE);
}

/* =========================================================
 * PALETTE EDITOR
 * ========================================================= */
void ui_draw_palette_editor(AppContext* ctx) {
    NDSTexture* tex = (ctx->selected_texture >= 0 && ctx->selected_texture < (int)ctx->texture_count)
                       ? &ctx->textures[ctx->selected_texture] : NULL;

    ui_fill_rect(true, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, COL_DARKGRAY);
    draw_header(true, "PALETTE EDITOR");

    if (!tex) { return; }

    /* Mostra palette completa 16x16 */
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            int idx = row * 16 + col;
            bool sel = (idx == ctx->selected_color);
            int px = col * 15 + 4;
            int py = 14 + row * 10;
            ui_fill_rect(true, px, py, 14, 9, tex->palette[idx]);
            if (sel) ui_draw_rect(true, px-1, py-1, 16, 11, COL_WHITE);
        }
    }

    ui_fill_rect(true, 0, NDS_SCREEN_H - 22, NDS_SCREEN_W, 22, COL_PURPLE);
    ui_draw_string(true, 4, NDS_SCREEN_H - 20, "D-Pad: sposta cursore",    COL_LIGHTGRAY, COL_PURPLE);
    ui_draw_string(true, 4, NDS_SCREEN_H - 11, "A: modifica  B: torna",    COL_LIGHTGRAY, COL_PURPLE);

    /* Inferiore: slider RGB */
    ui_fill_rect(false, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, COL_DARKGRAY);
    draw_header(false, "MODIFICA COLORE");

    uint16_t sel_col = tex->palette[ctx->selected_color];
    uint8_t cr, cg, cb;
    bgr15_to_rgb(sel_col, &cr, &cg, &cb);
    cr >>= 3; cg >>= 3; cb >>= 3; /* 0-31 */

    /* Preview colore */
    ui_fill_rect(false, 4, 16, 40, 40, sel_col);
    ui_draw_rect(false, 4, 16, 40, 40, COL_WHITE);

    char buf[32];
    snprintf(buf, 32, "Idx: %d", ctx->selected_color);
    ui_draw_string(false, 50, 20, buf, COL_WHITE, COL_DARKGRAY);
    snprintf(buf, 32, "Hex: %04X", sel_col);
    ui_draw_string(false, 50, 32, buf, COL_CYAN, COL_DARKGRAY);

    /* Barre RGB */
    const char* ch_names[] = {"R", "G", "B"};
    uint8_t ch_vals[]  = {cr, cg, cb};
    uint16_t ch_cols[] = {COL_RED, COL_GREEN, COL_BLUE};

    for (int c = 0; c < 3; c++) {
        int by = 62 + c * 28;
        ui_draw_string(false, 4, by, ch_names[c], ch_cols[c], COL_DARKGRAY);
        /* Track */
        ui_fill_rect(false, 18, by + 1, 200, 9, COL_GRAY);
        /* Fill */
        int fill_w = (int)ch_vals[c] * 200 / 31;
        ui_fill_rect(false, 18, by + 1, fill_w, 9, ch_cols[c]);
        ui_draw_rect(false, 18, by, 201, 11, COL_LIGHTGRAY);
        /* Valore */
        char val[4];
        snprintf(val, 4, "%2d", ch_vals[c]);
        ui_draw_string(false, 222, by, val, COL_WHITE, COL_DARKGRAY);
    }

    ui_fill_rect(false, 0, NDS_SCREEN_H - 32, NDS_SCREEN_W, 22, COL_PURPLE);
    ui_draw_string(false, 2, NDS_SCREEN_H - 30, "Touch barra = modifica canale",
                   COL_LIGHTGRAY, COL_PURPLE);
    ui_draw_string(false, 2, NDS_SCREEN_H - 20, "A:applica  B:annulla",
                   COL_LIGHTGRAY, COL_PURPLE);

    ui_draw_status_bar(false, "Tocca R/G/B per modificare", COL_DPURPLE);
}

/* =========================================================
 * SAVE CONFIRM
 * ========================================================= */
void ui_draw_save_confirm(AppContext* ctx) {
    ui_fill_rect(true, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, COL_DPURPLE);
    draw_header(true, "SALVA MODIFICHE");

    ui_draw_string_center(true, 40,  "Salvare le modifiche", COL_WHITE,    COL_DPURPLE);
    ui_draw_string_center(true, 52,  "nella ROM NDS?",       COL_WHITE,    COL_DPURPLE);
    ui_draw_string_center(true, 70,  "ATTENZIONE:",          COL_YELLOW,   COL_DPURPLE);
    ui_draw_string_center(true, 82,  "Operazione irrevers.",  COL_YELLOW,   COL_DPURPLE);
    ui_draw_string_center(true, 94,  "Esegui backup prima!",  COL_MAGENTA,  COL_DPURPLE);

    /* Pulsanti */
    ui_fill_rect(true, 20, 120, 90, 24, COL_GREEN);
    ui_draw_rect(true, 20, 120, 90, 24, COL_WHITE);
    ui_draw_string_center(true, 129, "A = SALVA", COL_BLACK, COL_GREEN);

    ui_fill_rect(true, 146, 120, 90, 24, COL_RED);
    ui_draw_rect(true, 146, 120, 90, 24, COL_WHITE);
    ui_draw_string_center(true, 129, "       B = ANNULLA", COL_WHITE, COL_RED);

    char path_short[36];
    strncpy(path_short, ctx->nds_path, 35); path_short[35] = '\0';
    ui_draw_string(true, 4, NDS_SCREEN_H - 20, path_short, COL_GRAY, COL_DPURPLE);

    ui_fill_rect(false, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, COL_DARKGRAY);
    ui_draw_string_center(false, 60, "Premi A sullo schermo", COL_WHITE,    COL_DARKGRAY);
    ui_draw_string_center(false, 72, "superiore per salvare", COL_WHITE,    COL_DARKGRAY);
    ui_draw_string_center(false, 90, "oppure B per tornare",  COL_LIGHTGRAY,COL_DARKGRAY);
    ui_draw_status_bar(false, "A=salva  B=annulla", COL_DPURPLE);
}

/* =========================================================
 * INFO SCREEN
 * ========================================================= */
void ui_draw_info_screen(AppContext* ctx) {
    ui_fill_rect(true, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, COL_DARKGRAY);
    draw_header(true, "INFO ROM NDS");

    if (!ctx->rom_loaded) {
        ui_draw_string_center(true, 80, "Nessuna ROM caricata", COL_GRAY, COL_DARKGRAY);
        return;
    }

    char buf[48];
    int y = 16;
    snprintf(buf, 48, "Titolo: %.12s", ctx->header.game_title);
    ui_draw_string(true, 4, y, buf, COL_NEON_CYAN, COL_DARKGRAY); y += 12;
    snprintf(buf, 48, "Codice: %.4s  Maker: %.2s",
             ctx->header.game_code, ctx->header.maker_code);
    ui_draw_string(true, 4, y, buf, COL_WHITE, COL_DARKGRAY); y += 12;
    snprintf(buf, 48, "Unita: %s  Regione: %d",
             ctx->header.unit_code == 0 ? "DS" :
             ctx->header.unit_code == 2 ? "DSi" : "DS+DSi",
             ctx->header.nds_region);
    ui_draw_string(true, 4, y, buf, COL_LIGHTGRAY, COL_DARKGRAY); y += 12;
    snprintf(buf, 48, "ROM Size: ~%d MB",
             (1 << ctx->header.device_capacity) / 1024);
    ui_draw_string(true, 4, y, buf, COL_LIGHTGRAY, COL_DARKGRAY); y += 12;
    snprintf(buf, 48, "ARM9 off: 0x%08lX (%lu B)",
             (unsigned long)ctx->header.arm9_rom_offset,
             (unsigned long)ctx->header.arm9_size);
    ui_draw_string(true, 4, y, buf, COL_GRAY, COL_DARKGRAY); y += 12;
    snprintf(buf, 48, "FNT: 0x%lX (%lu B)",
             (unsigned long)ctx->header.fnt_offset,
             (unsigned long)ctx->header.fnt_size);
    ui_draw_string(true, 4, y, buf, COL_GRAY, COL_DARKGRAY); y += 12;
    snprintf(buf, 48, "FAT: 0x%lX (%lu file)",
             (unsigned long)ctx->header.fat_offset,
             (unsigned long)ctx->fat_count);
    ui_draw_string(true, 4, y, buf, COL_GRAY, COL_DARKGRAY); y += 12;
    snprintf(buf, 48, "Modific: %s",
             ctx->rom_modified ? "SI (non salvato)" : "NO");
    ui_draw_string(true, 4, y, buf,
                   ctx->rom_modified ? COL_YELLOW : COL_GREEN, COL_DARKGRAY);

    ui_draw_status_bar(true, "B: torna indietro", COL_DPURPLE);

    ui_fill_rect(false, 0, 0, NDS_SCREEN_W, NDS_SCREEN_H, COL_DPURPLE);
    draw_header(false, APP_NAME " " APP_VERSION);
    ui_draw_string_center(false, 30, "NDS Texture Editor",   COL_NEON_CYAN, COL_DPURPLE);
    ui_draw_string_center(false, 44, "Ispirato a NitroEdit", COL_LIGHTGRAY, COL_DPURPLE);
    ui_draw_string_center(false, 58, "by XorTroll",          COL_LIGHTGRAY, COL_DPURPLE);
    ui_draw_string_center(false, 80, "Compatibile con:",     COL_WHITE,     COL_DPURPLE);
    ui_draw_string_center(false, 92, "R4 DS/R4i/Acekard/",  COL_CYAN,      COL_DPURPLE);
    ui_draw_string_center(false, 104,"SuperCard DSTwo",       COL_CYAN,      COL_DPURPLE);
    ui_draw_string_center(false, 120,"Formati: NCGR NARC",   COL_YELLOW,    COL_DPURPLE);
    ui_draw_string_center(false, 132,"NSBMD NSBTX + BIN",    COL_YELLOW,    COL_DPURPLE);
    ui_draw_status_bar(false, "B: chiudi info", COL_DPURPLE);
}

/* =========================================================
 * DISPATCHER schermata superiore
 * ========================================================= */
void ui_draw_top_screen(AppContext* ctx) {
    switch (ctx->state) {
        case STATE_FILE_BROWSER:   ui_draw_file_browser(ctx);   break;
        case STATE_NITRO_BROWSER:  ui_draw_nitro_browser(ctx);  break;
        case STATE_TEXTURE_LIST:   ui_draw_texture_list(ctx);   break;
        case STATE_TEXTURE_VIEW:   ui_draw_texture_view(ctx);   break;
        case STATE_PIXEL_EDITOR:   ui_draw_pixel_editor(ctx);   break;
        case STATE_PALETTE_EDITOR: ui_draw_palette_editor(ctx); break;
        case STATE_SAVE_CONFIRM:   ui_draw_save_confirm(ctx);   break;
        case STATE_INFO:           ui_draw_info_screen(ctx);    break;
        default: break;
    }
}

/* =========================================================
 * DISPATCHER schermata inferiore
 * ========================================================= */
void ui_draw_bottom_screen(AppContext* ctx) {
    /* Già gestito dalle funzioni sopra (disegnano entrambi gli schermi) */
    (void)ctx;
}
