/*
 * tex_decoder.h
 * NDS Texture Editor - Decoder texture NDS
 */

#ifndef TEX_DECODER_H
#define TEX_DECODER_H

#include "nds_types.h"

/* Decodifica texture in base al suo formato */
bool tex_decode(NDSTexture* tex);

/* Codifica pixel editor -> formato NDS grezzo (per salvataggio) */
bool tex_encode(NDSTexture* tex);

/* Carica texture dal file NDS */
bool tex_load(AppContext* ctx, int tex_index);

/* Salva texture modificata nel file NDS */
bool tex_save(AppContext* ctx, int tex_index);

/* Render texture su framebuffer NDS (schermo superiore o inferiore) */
void tex_render(NDSTexture* tex, int screen_x, int screen_y,
                int view_x, int view_y, int zoom,
                bool top_screen);

/* Ottieni colore pixel (indice palette o direct) */
uint16_t tex_get_pixel(NDSTexture* tex, int x, int y);

/* Imposta colore pixel */
void tex_set_pixel(NDSTexture* tex, int x, int y, uint16_t color);

/* Converte BGR15 -> RGB componenti */
void bgr15_to_rgb(uint16_t bgr, uint8_t* r, uint8_t* g, uint8_t* b);

/* Converte componenti RGB -> BGR15 */
uint16_t rgb_to_bgr15(uint8_t r, uint8_t g, uint8_t b);

#endif /* TEX_DECODER_H */
