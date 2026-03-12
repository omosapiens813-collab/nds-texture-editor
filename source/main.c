/*
 * main.c
 * NDS Texture Editor - Main loop e gestione input
 * Compilare con devkitARM + libnds + libfat
 *
 * Ispirato a NitroEdit by XorTroll
 * Per R4 DS / R4i / Acekard / SuperCard DSTwo
 */

#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "include/nds_types.h"
#include "include/nds_parser.h"
#include "include/tex_decoder.h"
#include "include/ui.h"

/* =========================================================
 * Contesto globale applicazione
 * ========================================================= */
static AppContext g_ctx;

/* =========================================================
 * Inizializza file browser nella directory indicata
 * ========================================================= */
static void fb_load_dir(AppContext* ctx, const char* path) {
    strncpy(ctx->current_dir, path, MAX_PATH - 1);
    ctx->file_count  = 0;
    ctx->file_cursor = 0;
    ctx->file_scroll = 0;

    DIR* dir = opendir(path);
    if (!dir) {
        snprintf(ctx->status_msg, 64, "Errore apertura dir");
        ctx->status_timer = 60;
        return;
    }

    /* Aggiungi ".." per tornare su (se non siamo in root) */
    if (strcmp(path, "/") != 0 && strcmp(path, "fat:/") != 0) {
        strncpy(ctx->file_list[ctx->file_count], "[..]", MAX_FILENAME - 1);
        ctx->file_count++;
    }

    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL && ctx->file_count < MAX_FILES) {
        /* Salta "." e ".." */
        if (strcmp(ent->d_name, ".") == 0) continue;
        if (strcmp(ent->d_name, "..") == 0) continue;

        if (ent->d_type == DT_DIR) {
            /* Directory: aggiungi con prefisso [ ] */
            char dname[MAX_FILENAME];
            snprintf(dname, MAX_FILENAME, "[%s]", ent->d_name);
            strncpy(ctx->file_list[ctx->file_count], dname, MAX_FILENAME - 1);
            ctx->file_list[ctx->file_count][MAX_FILENAME - 1] = '\0';
            ctx->file_count++;
        } else {
            /* File: mostra solo .nds e file generici */
            const char* dot = strrchr(ent->d_name, '.');
            bool is_nds = false;
            if (dot) {
                char ext[8];
                int k = 0;
                while (dot[k] && k < 7) {
                    char c = dot[k];
                    if (c >= 'A' && c <= 'Z') c += 32;
                    ext[k++] = c;
                }
                ext[k] = '\0';
                is_nds = (strcmp(ext, ".nds") == 0);
            }
            /* Mostra solo file .nds nel browser */
            if (is_nds) {
                strncpy(ctx->file_list[ctx->file_count], ent->d_name, MAX_FILENAME - 1);
                ctx->file_list[ctx->file_count][MAX_FILENAME - 1] = '\0';
                ctx->file_count++;
            }
        }
    }
    closedir(dir);

    snprintf(ctx->status_msg, 64, "%lu file/dir trovati", (unsigned long)ctx->file_count);
    ctx->status_timer = 60;
}

/* =========================================================
 * Input handler: FILE BROWSER
 * ========================================================= */
static void input_file_browser(AppContext* ctx, u32 keys_down, u32 keys_held) {
    (void)keys_held;

    if (keys_down & KEY_UP) {
        if (ctx->file_cursor > 0) ctx->file_cursor--;
        if (ctx->file_cursor < ctx->file_scroll) ctx->file_scroll = ctx->file_cursor;
    }
    if (keys_down & KEY_DOWN) {
        if (ctx->file_cursor < (int)ctx->file_count - 1) ctx->file_cursor++;
        int visible = (NDS_SCREEN_H - 14 - 10) / 12;
        if (ctx->file_cursor >= ctx->file_scroll + visible)
            ctx->file_scroll = ctx->file_cursor - visible + 1;
    }

    if (keys_down & KEY_A) {
        if (ctx->file_count == 0) return;
        const char* sel = ctx->file_list[ctx->file_cursor];

        if (sel[0] == '[') {
            /* Navigazione directory */
            char new_path[MAX_PATH];
            if (strcmp(sel, "[..]") == 0) {
                /* Torna alla directory padre */
                strncpy(new_path, ctx->current_dir, MAX_PATH - 1);
                char* last_slash = strrchr(new_path, '/');
                if (last_slash && last_slash != new_path) {
                    *last_slash = '\0';
                } else {
                    strncpy(new_path, "fat:/", MAX_PATH - 1);
                }
            } else {
                /* Entra in sottodirectory */
                snprintf(new_path, MAX_PATH, "%s/%.*s",
                         ctx->current_dir,
                         (int)(strlen(sel) - 2), sel + 1); /* Rimuovi [ e ] */
            }
            fb_load_dir(ctx, new_path);
        } else {
            /* File .nds selezionato */
            char full_path[MAX_PATH];
            snprintf(full_path, MAX_PATH, "%s/%s", ctx->current_dir, sel);

            /* Libera ROM precedente */
            if (ctx->rom_loaded) nds_free_rom(ctx);

            if (nds_load_rom(ctx, full_path)) {
                ctx->state = STATE_NITRO_BROWSER;
                ctx->nitro_cursor = 0;
                ctx->nitro_scroll = 0;
                /* Leggi NitroFS automaticamente */
                nds_read_nitroFS(ctx);
            }
        }
    }

    if (keys_down & KEY_B) {
        /* Torna alla directory padre */
        char new_path[MAX_PATH];
        strncpy(new_path, ctx->current_dir, MAX_PATH - 1);
        char* last_slash = strrchr(new_path, '/');
        if (last_slash && last_slash != new_path) {
            *last_slash = '\0';
            fb_load_dir(ctx, new_path);
        }
    }

    /* Touch screen: click su file */
    if (keys_down & KEY_TOUCH) {
        touchPosition tp;
        touchRead(&tp);
        int list_y = 14, item_h = 12;
        int clicked_idx = ctx->file_scroll + (tp.py - list_y) / item_h;
        if (tp.py >= list_y && clicked_idx >= 0 && clicked_idx < (int)ctx->file_count) {
            ctx->file_cursor = clicked_idx;
            /* Doppio click = apri (semplificazione: apri direttamente) */
        }
    }
}

/* =========================================================
 * Input handler: NITRO BROWSER
 * ========================================================= */
static void input_nitro_browser(AppContext* ctx, u32 keys_down, u32 keys_held) {
    (void)keys_held;
    int visible = (NDS_SCREEN_H - 14 - 10) / 11;

    if (keys_down & KEY_UP) {
        if (ctx->nitro_cursor > 0) ctx->nitro_cursor--;
        if (ctx->nitro_cursor < ctx->nitro_scroll) ctx->nitro_scroll = ctx->nitro_cursor;
    }
    if (keys_down & KEY_DOWN) {
        if (ctx->nitro_cursor < (int)ctx->nitro_file_count - 1) ctx->nitro_cursor++;
        if (ctx->nitro_cursor >= ctx->nitro_scroll + visible)
            ctx->nitro_scroll = ctx->nitro_cursor - visible + 1;
    }

    if (keys_down & KEY_A) {
        /* Scansiona texture */
        nds_scan_textures(ctx);
        if (ctx->texture_count > 0) {
            ctx->state = STATE_TEXTURE_LIST;
            ctx->tex_list_cursor = 0;
            ctx->tex_list_scroll = 0;
        } else {
            snprintf(ctx->status_msg, 64, "Nessuna texture trovata");
            ctx->status_timer = 90;
        }
    }

    if (keys_down & KEY_B) {
        /* Torna al file browser */
        nds_free_rom(ctx);
        ctx->state = STATE_FILE_BROWSER;
    }

    if (keys_down & KEY_SELECT) {
        ctx->state = STATE_INFO;
    }

    /* Touch: click su file nitro */
    if (keys_down & KEY_TOUCH) {
        touchPosition tp;
        touchRead(&tp);
        int list_y = 14, item_h = 11;
        int clicked_idx = ctx->nitro_scroll + (tp.py - list_y) / item_h;
        if (tp.py >= list_y && clicked_idx >= 0 && clicked_idx < (int)ctx->nitro_file_count) {
            ctx->nitro_cursor = clicked_idx;
        }
    }
}

/* =========================================================
 * Input handler: TEXTURE LIST
 * ========================================================= */
static void input_texture_list(AppContext* ctx, u32 keys_down, u32 keys_held) {
    (void)keys_held;
    int visible = (NDS_SCREEN_H - 14 - 10) / 13;

    if (keys_down & KEY_UP) {
        if (ctx->tex_list_cursor > 0) ctx->tex_list_cursor--;
        if (ctx->tex_list_cursor < ctx->tex_list_scroll)
            ctx->tex_list_scroll = ctx->tex_list_cursor;
    }
    if (keys_down & KEY_DOWN) {
        if (ctx->tex_list_cursor < (int)ctx->texture_count - 1) ctx->tex_list_cursor++;
        if (ctx->tex_list_cursor >= ctx->tex_list_scroll + visible)
            ctx->tex_list_scroll = ctx->tex_list_cursor - visible + 1;
    }

    if (keys_down & KEY_A) {
        /* Apri texture viewer */
        ctx->selected_texture = ctx->tex_list_cursor;
        NDSTexture* tex = &ctx->textures[ctx->selected_texture];
        if (!tex->loaded) tex_load(ctx, ctx->selected_texture);
        ctx->state        = STATE_TEXTURE_VIEW;
        ctx->view_zoom    = 2;
        ctx->view_offset_x = 0;
        ctx->view_offset_y = 0;
    }

    if (keys_down & KEY_X) {
        /* Carica texture corrente */
        if (ctx->tex_list_cursor < (int)ctx->texture_count) {
            tex_load(ctx, ctx->tex_list_cursor);
        }
    }

    if (keys_down & KEY_Y) {
        /* Salva tutte le texture modificate */
        int saved = 0;
        for (int i = 0; i < (int)ctx->texture_count; i++) {
            if (ctx->textures[i].loaded) {
                tex_save(ctx, i);
                saved++;
            }
        }
        if (saved > 0) {
            snprintf(ctx->status_msg, 64, "%d texture salvate!", saved);
        } else {
            snprintf(ctx->status_msg, 64, "Nessuna texture da salvare");
        }
        ctx->status_timer = 120;
    }

    if (keys_down & KEY_B) {
        ctx->state = STATE_NITRO_BROWSER;
    }

    if (keys_down & KEY_SELECT) {
        ctx->state = STATE_INFO;
    }

    /* Touch: click su texture */
    if (keys_down & KEY_TOUCH) {
        touchPosition tp;
        touchRead(&tp);
        int list_y = 14, item_h = 13;
        int clicked_idx = ctx->tex_list_scroll + (tp.py - list_y) / item_h;
        if (tp.py >= list_y && clicked_idx >= 0 && clicked_idx < (int)ctx->texture_count) {
            ctx->tex_list_cursor = clicked_idx;
            /* Doppio-tap carica e apre (tap = solo seleziona) */
        }
    }
}

/* =========================================================
 * Input handler: TEXTURE VIEWER
 * ========================================================= */
static void input_texture_view(AppContext* ctx, u32 keys_down, u32 keys_held) {
    int speed = (keys_held & KEY_R) ? 4 : 1; /* R = move veloce */

    if (keys_held & KEY_UP)    ctx->view_offset_y -= speed;
    if (keys_held & KEY_DOWN)  ctx->view_offset_y += speed;
    if (keys_held & KEY_LEFT)  ctx->view_offset_x -= speed;
    if (keys_held & KEY_RIGHT) ctx->view_offset_x += speed;

    /* Clamp offset */
    if (ctx->view_offset_x < 0) ctx->view_offset_x = 0;
    if (ctx->view_offset_y < 0) ctx->view_offset_y = 0;
    if (ctx->selected_texture >= 0 && ctx->selected_texture < (int)ctx->texture_count) {
        NDSTexture* tex = &ctx->textures[ctx->selected_texture];
        if (ctx->view_offset_x >= tex->width)  ctx->view_offset_x = tex->width - 1;
        if (ctx->view_offset_y >= tex->height) ctx->view_offset_y = tex->height - 1;
    }

    if (keys_down & KEY_L) {
        if (ctx->view_zoom > 1) ctx->view_zoom--;
    }
    if (keys_down & KEY_R) {
        if (ctx->view_zoom < 8) ctx->view_zoom++;
    }

    if (keys_down & KEY_A) {
        /* Entra nel pixel editor */
        ctx->state        = STATE_PIXEL_EDITOR;
        ctx->px_zoom      = ctx->view_zoom;
        ctx->px_offset_x  = ctx->view_offset_x;
        ctx->px_offset_y  = ctx->view_offset_y;
        ctx->px_cursor_x  = ctx->view_offset_x;
        ctx->px_cursor_y  = ctx->view_offset_y;
        ctx->selected_color = 1;
        ctx->px_drawing   = false;
    }

    if (keys_down & KEY_X) {
        tex_load(ctx, ctx->selected_texture);
    }

    if (keys_down & KEY_Y) {
        ctx->state = STATE_PALETTE_EDITOR;
    }

    if (keys_down & KEY_B) {
        ctx->state = STATE_TEXTURE_LIST;
    }

    if (keys_down & KEY_SELECT) {
        ctx->state = STATE_INFO;
    }
}

/* =========================================================
 * Input handler: PIXEL EDITOR
 * ========================================================= */
static void input_pixel_editor(AppContext* ctx, u32 keys_down, u32 keys_held) {
    NDSTexture* tex = (ctx->selected_texture >= 0 && ctx->selected_texture < (int)ctx->texture_count)
                       ? &ctx->textures[ctx->selected_texture] : NULL;
    if (!tex || !tex->loaded) {
        if (keys_down & KEY_B) ctx->state = STATE_TEXTURE_VIEW;
        return;
    }

    /* Muovi cursore pixel */
    if (keys_down & KEY_UP    || keys_held & KEY_UP) {
        if (ctx->px_cursor_y > 0) ctx->px_cursor_y--;
        if (ctx->px_cursor_y < ctx->px_offset_y) ctx->px_offset_y = ctx->px_cursor_y;
    }
    if (keys_down & KEY_DOWN  || keys_held & KEY_DOWN) {
        if (ctx->px_cursor_y < tex->height - 1) ctx->px_cursor_y++;
        int vis_h = (NDS_SCREEN_H - 24) / ctx->px_zoom;
        if (ctx->px_cursor_y >= ctx->px_offset_y + vis_h)
            ctx->px_offset_y = ctx->px_cursor_y - vis_h + 1;
    }
    if (keys_down & KEY_LEFT  || keys_held & KEY_LEFT) {
        if (ctx->px_cursor_x > 0) ctx->px_cursor_x--;
        if (ctx->px_cursor_x < ctx->px_offset_x) ctx->px_offset_x = ctx->px_cursor_x;
    }
    if (keys_down & KEY_RIGHT || keys_held & KEY_RIGHT) {
        if (ctx->px_cursor_x < tex->width - 1) ctx->px_cursor_x++;
        int vis_w = (NDS_SCREEN_W - 4) / ctx->px_zoom;
        if (ctx->px_cursor_x >= ctx->px_offset_x + vis_w)
            ctx->px_offset_x = ctx->px_cursor_x - vis_w + 1;
    }

    /* Zoom */
    if (keys_down & KEY_L) { if (ctx->px_zoom > 1) ctx->px_zoom /= 2; }
    if (keys_down & KEY_R) { if (ctx->px_zoom < 8) ctx->px_zoom *= 2; }

    /* Disegna pixel */
    if (keys_down & KEY_A) {
        uint16_t color = tex->palette[ctx->selected_color];
        tex_set_pixel(tex, ctx->px_cursor_x, ctx->px_cursor_y, color);
        ctx->rom_modified = true;
    }

    /* Cancella pixel (colore palette 0) */
    if (keys_down & KEY_B && (keys_held & KEY_SELECT)) {
        tex_set_pixel(tex, ctx->px_cursor_x, ctx->px_cursor_y, tex->palette[0]);
        ctx->rom_modified = true;
    } else if (keys_down & KEY_B) {
        ctx->state = STATE_TEXTURE_VIEW;
        return;
    }

    /* Salva */
    if (keys_down & KEY_X) {
        ctx->state = STATE_SAVE_CONFIRM;
        return;
    }

    /* Palette editor */
    if (keys_down & KEY_Y) {
        ctx->state = STATE_PALETTE_EDITOR;
        return;
    }

    /* Touch: selezione colore palette sullo schermo inferiore */
    if (keys_held & KEY_TOUCH) {
        touchPosition tp;
        touchRead(&tp);

        int pal_cols = 16;
        int sw = NDS_SCREEN_W / pal_cols;
        int sh = 10;
        int pal_rows = (tex->format == TEX_256COLOR) ? 16 : 4;

        int col_x = tp.px / sw;
        int col_y = (tp.py - 14) / sh;

        if (col_x >= 0 && col_x < pal_cols && col_y >= 0 && col_y < pal_rows) {
            int idx = col_y * pal_cols + col_x;
            if (idx >= 0 && idx < MAX_PALETTE_SIZE) {
                ctx->selected_color = idx;
                /* Disegna subito il pixel se A è premuto */
                if (keys_held & KEY_A) {
                    tex_set_pixel(tex, ctx->px_cursor_x, ctx->px_cursor_y,
                                  tex->palette[idx]);
                    ctx->rom_modified = true;
                }
            }
        }

        /* Touch su canvas superiore: disegna pixel */
        if (tp.py < 14) {
            /* Tocca header -> ignora */
        }
    }
}

/* =========================================================
 * Input handler: PALETTE EDITOR
 * ========================================================= */
static void input_palette_editor(AppContext* ctx, u32 keys_down, u32 keys_held) {
    (void)keys_held;
    NDSTexture* tex = (ctx->selected_texture >= 0 && ctx->selected_texture < (int)ctx->texture_count)
                       ? &ctx->textures[ctx->selected_texture] : NULL;
    if (!tex) { if (keys_down & KEY_B) ctx->state = STATE_PIXEL_EDITOR; return; }

    /* Muovi cursore palette (16 per riga) */
    if (keys_down & KEY_UP)    { ctx->selected_color -= 16; if (ctx->selected_color < 0) ctx->selected_color += 256; }
    if (keys_down & KEY_DOWN)  { ctx->selected_color += 16; if (ctx->selected_color >= 256) ctx->selected_color -= 256; }
    if (keys_down & KEY_LEFT)  { if (ctx->selected_color > 0) ctx->selected_color--; }
    if (keys_down & KEY_RIGHT) { if (ctx->selected_color < 255) ctx->selected_color++; }

    /* Touch sullo schermo inferiore: modifica canale R/G/B */
    if (keys_held & KEY_TOUCH) {
        touchPosition tp;
        touchRead(&tp);

        uint16_t sel_col = tex->palette[ctx->selected_color];
        uint8_t cr, cg, cb;
        bgr15_to_rgb(sel_col, &cr, &cg, &cb);
        cr >>= 3; cg >>= 3; cb >>= 3;

        /* Zone slider: R=62, G=90, B=118 */
        int slider_x = 18, slider_w = 200;
        int slider_rows[] = {62, 90, 118};

        for (int c = 0; c < 3; c++) {
            int sy = slider_rows[c];
            if (tp.py >= sy && tp.py < sy + 11 &&
                tp.px >= slider_x && tp.px < slider_x + slider_w) {
                uint8_t val = (uint8_t)((tp.px - slider_x) * 31 / (slider_w - 1));
                if (val > 31) val = 31;
                if (c == 0) cr = val;
                else if (c == 1) cg = val;
                else             cb = val;

                /* Ricostruisce colore */
                tex->palette[ctx->selected_color] = rgb_to_bgr15(
                    (uint8_t)(cr << 3),
                    (uint8_t)(cg << 3),
                    (uint8_t)(cb << 3)
                );
                ctx->rom_modified = true;
                break;
            }
        }

        /* Click sulla palette grande (superiore) */
        int pal_x = 4, pal_y = 14;
        int cell_w = 15, cell_h = 10;
        if (tp.px >= pal_x && tp.px < pal_x + 16 * cell_w &&
            tp.py >= pal_y && tp.py < pal_y + 16 * cell_h) {
            int col_i = (tp.px - pal_x) / cell_w;
            int row_i = (tp.py - pal_y) / cell_h;
            ctx->selected_color = row_i * 16 + col_i;
        }
    }

    if (keys_down & KEY_A) {
        /* Applica: torna al pixel editor con palette aggiornata */
        /* Ricodifica texture con nuova palette */
        if (tex->loaded) tex_decode(tex); /* Ridecodifica con palette aggiornata */
        ctx->state = STATE_PIXEL_EDITOR;
    }

    if (keys_down & KEY_B) {
        ctx->state = STATE_PIXEL_EDITOR;
    }

    if (keys_down & KEY_X) {
        ctx->state = STATE_SAVE_CONFIRM;
    }
}

/* =========================================================
 * Input handler: SAVE CONFIRM
 * ========================================================= */
static void input_save_confirm(AppContext* ctx, u32 keys_down, u32 keys_held) {
    (void)keys_held;

    if (keys_down & KEY_A) {
        /* Salva texture selezionata */
        if (ctx->selected_texture >= 0) {
            if (tex_save(ctx, ctx->selected_texture)) {
                snprintf(ctx->status_msg, 64, "Salvato con successo!");
            } else {
                snprintf(ctx->status_msg, 64, "ERRORE salvataggio!");
            }
            ctx->status_timer = 120;
        }
        ctx->state = STATE_PIXEL_EDITOR;
    }

    if (keys_down & KEY_B) {
        ctx->state = STATE_PIXEL_EDITOR;
    }
}

/* =========================================================
 * Input handler: INFO
 * ========================================================= */
static void input_info(AppContext* ctx, u32 keys_down, u32 keys_held) {
    (void)keys_held;
    if (keys_down & KEY_B || keys_down & KEY_SELECT) {
        /* Torna allo stato precedente */
        if (ctx->rom_loaded)
            ctx->state = ctx->texture_count > 0 ? STATE_TEXTURE_LIST : STATE_NITRO_BROWSER;
        else
            ctx->state = STATE_FILE_BROWSER;
    }
}

/* =========================================================
 * Main loop
 * ========================================================= */
int main(void) {
    /* Inizializza hardware NDS */
    defaultExceptionHandler();

    /* Inizializza FAT (necessario per R4/scheda SD) */
    if (!fatInitDefault()) {
        /* Schermo di errore se FAT non disponibile */
        consoleDemoInit();
        printf("\n\n  NDS Texture Editor\n");
        printf("  ERRORE: FAT non init!\n\n");
        printf("  Assicurati di usare:\n");
        printf("  - R4 DS con DLDI\n");
        printf("  - R4i con FAT driver\n");
        printf("  - Acekard / SuperCard\n\n");
        printf("  Premi START per uscire\n");
        while (1) {
            swiWaitForVBlank();
            scanKeys();
            if (keysDown() & KEY_START) return 0;
        }
    }

    /* Inizializza video */
    ui_init_video();

    /* Inizializza contesto */
    memset(&g_ctx, 0, sizeof(AppContext));
    g_ctx.state           = STATE_FILE_BROWSER;
    g_ctx.selected_texture = -1;
    g_ctx.view_zoom       = 2;
    g_ctx.px_zoom         = 4;
    g_ctx.selected_color  = 1;

    /* Carica directory iniziale */
    fb_load_dir(&g_ctx, "fat:/");

    /* =========================================================
     * MAIN LOOP
     * ========================================================= */
    u32 frame = 0;
    u32 prev_keys = 0;

    while (1) {
        /* Leggi input */
        scanKeys();
        u32 keys_down = keysDown();
        u32 keys_held = keysHeld();

        /* Touchscreen */
        if (keys_held & KEY_TOUCH) {
            touchRead(&g_ctx.touch);
            g_ctx.touching = true;
        } else {
            g_ctx.touching = false;
        }

        /* ---- Gestione input per stato corrente ---- */
        switch (g_ctx.state) {
            case STATE_FILE_BROWSER:
                input_file_browser(&g_ctx, keys_down, keys_held);
                break;
            case STATE_NITRO_BROWSER:
                input_nitro_browser(&g_ctx, keys_down, keys_held);
                break;
            case STATE_TEXTURE_LIST:
                input_texture_list(&g_ctx, keys_down, keys_held);
                break;
            case STATE_TEXTURE_VIEW:
                input_texture_view(&g_ctx, keys_down, keys_held);
                break;
            case STATE_PIXEL_EDITOR:
                input_pixel_editor(&g_ctx, keys_down, keys_held);
                break;
            case STATE_PALETTE_EDITOR:
                input_palette_editor(&g_ctx, keys_down, keys_held);
                break;
            case STATE_SAVE_CONFIRM:
                input_save_confirm(&g_ctx, keys_down, keys_held);
                break;
            case STATE_INFO:
                input_info(&g_ctx, keys_down, keys_held);
                break;
            default:
                g_ctx.state = STATE_FILE_BROWSER;
                break;
        }

        /* Tasto START: menu rapido salva o esci */
        if (keys_down & KEY_START) {
            if (g_ctx.rom_modified && g_ctx.selected_texture >= 0) {
                g_ctx.state = STATE_SAVE_CONFIRM;
            }
        }

        /* ---- Rendering (ogni frame) ---- */
        swiWaitForVBlank();
        ui_draw_top_screen(&g_ctx);

        /* ---- Status timer ---- */
        if (g_ctx.status_timer > 0) {
            g_ctx.status_timer--;
            if (g_ctx.status_timer == 0) {
                g_ctx.status_msg[0] = '\0';
            }
        }

        /* Aggiornamento lento dei tasti tenuti (riduce velocità ripetizione) */
        if (frame % 4 != 0) {
            /* Maschera tasti direzionali per limitare velocità ripetizione */
            if (frame % 8 != 0) {
                /* Solo ogni 8 frame per cursori pixel editor */
            }
        }

        prev_keys = keys_held;
        frame++;

        /* Watchdog: ogni 300 frame flush file se modificato */
        if (frame % 300 == 0 && g_ctx.rom_modified && g_ctx.nds_file) {
            fflush(g_ctx.nds_file);
        }
    }

    /* Mai raggiunto - cleanup */
    nds_free_rom(&g_ctx);
    return 0;
}
