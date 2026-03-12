/*
 * tex_decoder.c
 * NDS Texture Editor - Decoder/Encoder texture NDS
 *
 * Formati supportati:
 *   TEX_4COLOR   - 2 bit per pixel, palette 4 colori
 *   TEX_16COLOR  - 4 bit per pixel, palette 16 colori
 *   TEX_256COLOR - 8 bit per pixel, palette 256 colori
 *   TEX_A3I5     - 8 bit: 3bit alpha + 5bit indice (8 colori)
 *   TEX_A5I3     - 8 bit: 5bit alpha + 3bit indice (8 colori)
 *   TEX_DIRECT   - 16 bit per pixel BGR15 diretto
 */

#include "../include/tex_decoder.h"
#include "../include/nds_parser.h"

/* =========================================================
 * Conversioni colore BGR15 <-> RGB
 * ========================================================= */
void bgr15_to_rgb(uint16_t bgr, uint8_t* r, uint8_t* g, uint8_t* b) {
    /* Formato NDS: 0bbbbbgggggrrrrr (5 bit per canale, bit 15 = alpha) */
    *r = (bgr & 0x1F) << 3;
    *g = ((bgr >> 5) & 0x1F) << 3;
    *b = ((bgr >> 10) & 0x1F) << 3;
}

uint16_t rgb_to_bgr15(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint16_t)(b >> 3) << 10) |
           ((uint16_t)(g >> 3) << 5)  |
           ((uint16_t)(r >> 3));
}

/* =========================================================
 * Ottieni pixel decodificato (BGR15)
 * ========================================================= */
uint16_t tex_get_pixel(NDSTexture* tex, int x, int y) {
    if (!tex->loaded || x < 0 || y < 0 || x >= tex->width || y >= tex->height)
        return 0;
    return tex->pixels[y * tex->width + x];
}

/* =========================================================
 * Imposta pixel (modifica pixels[] buffer)
 * ========================================================= */
void tex_set_pixel(NDSTexture* tex, int x, int y, uint16_t color) {
    if (!tex->loaded || x < 0 || y < 0 || x >= tex->width || y >= tex->height)
        return;
    tex->pixels[y * tex->width + x] = color;
}

/* =========================================================
 * Decodifica texture in pixels[] (BGR15 per pixel)
 * ========================================================= */
bool tex_decode(NDSTexture* tex) {
    if (!tex->raw_data || tex->data_size == 0) return false;

    uint32_t total_px = (uint32_t)tex->width * tex->height;
    if (total_px == 0 || total_px > MAX_TEXTURE_SIZE) return false;

    memset(tex->pixels, 0, sizeof(tex->pixels));

    switch (tex->format) {

    /* --------------------------------------------------
     * TEX_4COLOR - 2 bit per pixel (palette 4 colori)
     * -------------------------------------------------- */
    case TEX_4COLOR: {
        uint32_t byte_count = (total_px + 3) / 4;
        if (byte_count > tex->data_size) byte_count = tex->data_size;
        for (uint32_t b = 0; b < byte_count; b++) {
            uint8_t byte = tex->raw_data[b];
            for (int bit = 0; bit < 4 && (b * 4 + bit) < total_px; bit++) {
                uint8_t idx = (byte >> (bit * 2)) & 0x3;
                uint32_t px = b * 4 + bit;
                tex->pixels[px] = tex->palette[idx];
            }
        }
        break;
    }

    /* --------------------------------------------------
     * TEX_16COLOR - 4 bit per pixel (palette 16 colori)
     * -------------------------------------------------- */
    case TEX_16COLOR: {
        uint32_t byte_count = (total_px + 1) / 2;
        if (byte_count > tex->data_size) byte_count = tex->data_size;
        for (uint32_t b = 0; b < byte_count; b++) {
            uint8_t byte = tex->raw_data[b];
            for (int nib = 0; nib < 2 && (b * 2 + nib) < total_px; nib++) {
                uint8_t idx = (nib == 0) ? (byte & 0x0F) : (byte >> 4);
                uint32_t px = b * 2 + nib;
                tex->pixels[px] = tex->palette[idx];
            }
        }
        break;
    }

    /* --------------------------------------------------
     * TEX_256COLOR - 8 bit per pixel (palette 256 colori)
     * -------------------------------------------------- */
    case TEX_256COLOR: {
        uint32_t byte_count = total_px;
        if (byte_count > tex->data_size) byte_count = tex->data_size;
        for (uint32_t b = 0; b < byte_count; b++) {
            uint8_t idx = tex->raw_data[b];
            tex->pixels[b] = tex->palette[idx];
        }
        break;
    }

    /* --------------------------------------------------
     * TEX_A3I5 - 3bit alpha + 5bit palette index (8 colori)
     * -------------------------------------------------- */
    case TEX_A3I5: {
        uint32_t byte_count = total_px;
        if (byte_count > tex->data_size) byte_count = tex->data_size;
        for (uint32_t b = 0; b < byte_count; b++) {
            uint8_t byte  = tex->raw_data[b];
            uint8_t idx   = byte & 0x1F;   /* 5 bit indice */
            uint8_t alpha = (byte >> 5) & 0x7; /* 3 bit alpha */
            uint16_t col  = tex->palette[idx];
            /* Usa bit 15 come "visible" se alpha >= 4 */
            if (alpha < 2) col = 0; /* Trasparente */
            tex->pixels[b] = col;
        }
        break;
    }

    /* --------------------------------------------------
     * TEX_A5I3 - 5bit alpha + 3bit palette index (8 colori)
     * -------------------------------------------------- */
    case TEX_A5I3: {
        uint32_t byte_count = total_px;
        if (byte_count > tex->data_size) byte_count = tex->data_size;
        for (uint32_t b = 0; b < byte_count; b++) {
            uint8_t byte  = tex->raw_data[b];
            uint8_t idx   = byte & 0x07;   /* 3 bit indice */
            uint8_t alpha = (byte >> 3) & 0x1F; /* 5 bit alpha */
            uint16_t col  = tex->palette[idx];
            if (alpha < 4) col = 0; /* Trasparente */
            tex->pixels[b] = col;
        }
        break;
    }

    /* --------------------------------------------------
     * TEX_DIRECT - 16 bit per pixel BGR15
     * -------------------------------------------------- */
    case TEX_DIRECT: {
        uint32_t word_count = total_px;
        uint32_t avail = tex->data_size / 2;
        if (word_count > avail) word_count = avail;
        for (uint32_t w = 0; w < word_count; w++) {
            uint16_t col = ((uint16_t*)tex->raw_data)[w];
            /* Bit 15: alpha (1=opaco, 0=trasparente) - mostra comunque */
            tex->pixels[w] = col & 0x7FFF;
        }
        break;
    }

    default:
        /* Formato sconosciuto: mostra dati raw come 256colori */
        for (uint32_t b = 0; b < total_px && b < tex->data_size; b++) {
            uint8_t v = tex->raw_data[b];
            tex->pixels[b] = RGB15(v >> 3, v >> 3, v >> 3); /* Grayscale */
        }
        break;
    }

    tex->loaded = true;
    return true;
}

/* =========================================================
 * Encoder: pixels[] -> raw_data (per salvataggio)
 * ========================================================= */
bool tex_encode(NDSTexture* tex) {
    if (!tex->loaded || !tex->raw_data) return false;

    uint32_t total_px = (uint32_t)tex->width * tex->height;
    if (total_px == 0) return false;

    switch (tex->format) {

    /* --------------------------------------------------
     * TEX_256COLOR - trova indice palette più vicino
     * -------------------------------------------------- */
    case TEX_256COLOR: {
        for (uint32_t px = 0; px < total_px && px < tex->data_size; px++) {
            uint16_t target = tex->pixels[px];
            /* Trova colore palette più vicino */
            uint8_t  best_idx  = 0;
            uint32_t best_dist = 0xFFFFFFFF;
            for (int c = 0; c < 256; c++) {
                uint16_t col = tex->palette[c];
                /* Distanza Manhattan in spazio BGR15 */
                int dr = (int)(target & 0x1F) - (int)(col & 0x1F);
                int dg = (int)((target >> 5) & 0x1F) - (int)((col >> 5) & 0x1F);
                int db = (int)((target >> 10) & 0x1F) - (int)((col >> 10) & 0x1F);
                uint32_t dist = (uint32_t)(dr*dr + dg*dg + db*db);
                if (dist < best_dist) {
                    best_dist = dist;
                    best_idx  = (uint8_t)c;
                    if (dist == 0) break;
                }
            }
            tex->raw_data[px] = best_idx;
        }
        break;
    }

    /* --------------------------------------------------
     * TEX_16COLOR - 4 bit per pixel
     * -------------------------------------------------- */
    case TEX_16COLOR: {
        uint32_t byte_count = (total_px + 1) / 2;
        memset(tex->raw_data, 0, byte_count);
        for (uint32_t px = 0; px < total_px; px++) {
            uint16_t target = tex->pixels[px];
            uint8_t best_idx = 0;
            uint32_t best_dist = 0xFFFFFFFF;
            for (int c = 0; c < 16; c++) {
                uint16_t col = tex->palette[c];
                int dr = (int)(target & 0x1F) - (int)(col & 0x1F);
                int dg = (int)((target >> 5) & 0x1F) - (int)((col >> 5) & 0x1F);
                int db = (int)((target >> 10) & 0x1F) - (int)((col >> 10) & 0x1F);
                uint32_t dist = (uint32_t)(dr*dr + dg*dg + db*db);
                if (dist < best_dist) { best_dist = dist; best_idx = (uint8_t)c; if(!dist)break; }
            }
            uint32_t byte_idx = px / 2;
            if (byte_idx < tex->data_size) {
                if (px % 2 == 0)
                    tex->raw_data[byte_idx] = (tex->raw_data[byte_idx] & 0xF0) | (best_idx & 0x0F);
                else
                    tex->raw_data[byte_idx] = (tex->raw_data[byte_idx] & 0x0F) | (best_idx << 4);
            }
        }
        break;
    }

    /* --------------------------------------------------
     * TEX_DIRECT - salva direttamente BGR15
     * -------------------------------------------------- */
    case TEX_DIRECT: {
        uint32_t word_count = total_px;
        uint32_t avail = tex->data_size / 2;
        if (word_count > avail) word_count = avail;
        for (uint32_t w = 0; w < word_count; w++) {
            ((uint16_t*)tex->raw_data)[w] = tex->pixels[w] | (1 << 15); /* alpha=1 */
        }
        break;
    }

    default:
        /* Formato non modificabile direttamente */
        return false;
    }

    return true;
}

/* =========================================================
 * Carica texture dal file NDS (raw_data + decode)
 * ========================================================= */
bool tex_load(AppContext* ctx, int tex_index) {
    if (tex_index < 0 || tex_index >= (int)ctx->texture_count) return false;

    NDSTexture* tex = &ctx->textures[tex_index];

    /* Alloca buffer raw */
    if (tex->raw_data) { free(tex->raw_data); tex->raw_data = NULL; }
    if (tex->data_size == 0) return false;

    tex->raw_data = (uint8_t*)malloc(tex->data_size);
    if (!tex->raw_data) {
        snprintf(ctx->status_msg, 64, "Memoria esaurita!");
        return false;
    }

    /* Leggi dati dal file */
    fseek(ctx->nds_file, tex->data_offset, SEEK_SET);
    if (fread(tex->raw_data, tex->data_size, 1, ctx->nds_file) != 1) {
        free(tex->raw_data);
        tex->raw_data = NULL;
        snprintf(ctx->status_msg, 64, "Errore lettura texture");
        return false;
    }

    /* Genera palette di default se non caricata */
    if (tex->palette[0] == 0 && tex->palette[1] == 0) {
        /* Palette arcobaleno semplice */
        for (int c = 0; c < 256; c++) {
            uint8_t r = (c & 0x07) << 2;
            uint8_t g = ((c >> 3) & 0x07) << 2;
            uint8_t b = ((c >> 6) & 0x03) << 3;
            tex->palette[c] = rgb_to_bgr15(r << 1, g << 1, b << 1);
        }
        /* Colori base per 16 e 4 */
        tex->palette[0]  = RGB15(0,  0,  0);  /* Nero */
        tex->palette[1]  = RGB15(31, 31, 31); /* Bianco */
        tex->palette[2]  = RGB15(31, 0,  0);  /* Rosso */
        tex->palette[3]  = RGB15(0,  31, 0);  /* Verde */
        tex->palette[4]  = RGB15(0,  0,  31); /* Blu */
        tex->palette[5]  = RGB15(31, 31, 0);  /* Giallo */
        tex->palette[6]  = RGB15(0,  31, 31); /* Cyan */
        tex->palette[7]  = RGB15(31, 0,  31); /* Magenta */
    }

    /* Decodifica */
    bool ok = tex_decode(tex);
    if (!ok) {
        snprintf(ctx->status_msg, 64, "Decoder: formato %s",
                 nds_texture_format_name(tex->format));
    }

    return ok;
}

/* =========================================================
 * Salva texture modificata nel file NDS
 * ========================================================= */
bool tex_save(AppContext* ctx, int tex_index) {
    if (tex_index < 0 || tex_index >= (int)ctx->texture_count) return false;

    NDSTexture* tex = &ctx->textures[tex_index];
    if (!tex->loaded || !tex->raw_data) return false;

    /* Codifica pixels -> raw */
    if (!tex_encode(tex)) {
        snprintf(ctx->status_msg, 64, "Encode fallito!");
        return false;
    }

    /* Scrivi nel file NDS */
    if (!nds_write_data(ctx, tex->data_offset, tex->raw_data, tex->data_size)) {
        return false;
    }

    /* Correggi CRC header */
    nds_fix_header_crc(ctx);

    snprintf(ctx->status_msg, 64, "Salvato: %s", tex->name);
    ctx->status_timer = 120;
    return true;
}

/* =========================================================
 * Render texture su schermo NDS
 * Disegna direttamente nel framebuffer Mode 5 (BG2)
 * top_screen=true -> schermo superiore, false -> inferiore
 * ========================================================= */
void tex_render(NDSTexture* tex, int screen_x, int screen_y,
                int view_x, int view_y, int zoom,
                bool top_screen) {
    if (!tex || !tex->loaded) return;

    /* Seleziona framebuffer */
    uint16_t* fb = top_screen
                   ? (uint16_t*)BG_BMP_RAM(0)     /* Top: VRAM A */
                   : (uint16_t*)BG_BMP_RAM_SUB(0); /* Bottom: VRAM C */

    /* zoom: 1=x1, 2=x2, 4=x4, 8=x8 */
    if (zoom < 1) zoom = 1;
    if (zoom > 8) zoom = 8;

    int tex_w = tex->width;
    int tex_h = tex->height;

    /* Area visibile texture */
    for (int py = 0; py < NDS_SCREEN_H - screen_y; py++) {
        int ty = view_y + py / zoom;
        if (ty < 0 || ty >= tex_h) continue;
        for (int px = 0; px < NDS_SCREEN_W - screen_x; px++) {
            int tx = view_x + px / zoom;
            if (tx < 0 || tx >= tex_w) continue;

            uint16_t col = tex->pixels[ty * tex_w + tx];
            int fb_x = screen_x + px;
            int fb_y = screen_y + py;
            if (fb_x < NDS_SCREEN_W && fb_y < NDS_SCREEN_H) {
                fb[fb_y * NDS_SCREEN_W + fb_x] = col | (1 << 15);
            }
        }
    }
}
