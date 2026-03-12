/*
 * ui.h
 * NDS Texture Editor - UI Renderer e gestione schermi
 */

#ifndef UI_H
#define UI_H

#include "nds_types.h"

/* ---- Inizializzazione video NDS ---- */
void ui_init_video(void);

/* ---- Rendering schermo superiore ---- */
void ui_draw_top_screen(AppContext* ctx);

/* ---- Rendering schermo inferiore ---- */
void ui_draw_bottom_screen(AppContext* ctx);

/* ---- Widget primitivi ---- */
void ui_fill_rect(bool top, int x, int y, int w, int h, uint16_t color);
void ui_draw_rect(bool top, int x, int y, int w, int h, uint16_t color);
void ui_draw_pixel(bool top, int x, int y, uint16_t color);
void ui_draw_char(bool top, int x, int y, char c, uint16_t fg, uint16_t bg);
void ui_draw_string(bool top, int x, int y, const char* str, uint16_t fg, uint16_t bg);
void ui_draw_string_center(bool top, int y, const char* str, uint16_t fg, uint16_t bg);

/* ---- Schermate specifiche ---- */
void ui_draw_file_browser(AppContext* ctx);
void ui_draw_nitro_browser(AppContext* ctx);
void ui_draw_texture_list(AppContext* ctx);
void ui_draw_texture_view(AppContext* ctx);
void ui_draw_pixel_editor(AppContext* ctx);
void ui_draw_palette_editor(AppContext* ctx);
void ui_draw_save_confirm(AppContext* ctx);
void ui_draw_info_screen(AppContext* ctx);

/* ---- Status bar ---- */
void ui_draw_status_bar(bool top, const char* msg, uint16_t bg_col);

/* ---- Font 5x8 ---- */
extern const uint8_t ui_font_5x8[128][8];

#endif /* UI_H */
