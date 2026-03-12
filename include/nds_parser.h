/*
 * nds_parser.h
 * NDS Texture Editor - Parser NDS ROM e NitroFS
 */

#ifndef NDS_PARSER_H
#define NDS_PARSER_H

#include "nds_types.h"

/* Inizializza e carica un file NDS */
bool nds_load_rom(AppContext* ctx, const char* path);

/* Libera la ROM dalla memoria */
void nds_free_rom(AppContext* ctx);

/* Legge il filesystem NitroFS */
bool nds_read_nitroFS(AppContext* ctx);

/* Legge un file dal NitroFS */
bool nds_read_file(AppContext* ctx, uint16_t file_id, uint8_t** out_data, uint32_t* out_size);

/* Scrive dati nel file NDS (patch in-place) */
bool nds_write_data(AppContext* ctx, uint32_t offset, const uint8_t* data, uint32_t size);

/* Scansiona texture nel NitroFS */
bool nds_scan_textures(AppContext* ctx);

/* Ricalcola CRC header NDS */
void nds_fix_header_crc(AppContext* ctx);

/* Utilities */
const char* nds_texture_format_name(TextureFormat fmt);
uint16_t    crc16(const uint8_t* data, uint32_t len);

#endif /* NDS_PARSER_H */
