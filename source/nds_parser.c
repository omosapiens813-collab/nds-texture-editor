/*
 * nds_parser.c
 * NDS Texture Editor - Implementazione parser NDS ROM e NitroFS
 * Compatibile con devkitARM + libnds + libfat
 */

#include "../include/nds_parser.h"

/* =========================================================
 * Tabella CRC16 (standard NDS)
 * ========================================================= */
static const uint16_t crc16_table[256] = {
    0x0000,0xC0C1,0xC181,0x0140,0xC301,0x03C0,0x0280,0xC241,
    0xC601,0x06C0,0x0780,0xC741,0x0500,0xC5C1,0xC481,0x0440,
    0xCC01,0x0CC0,0x0D80,0xCD41,0x0F00,0xCFC1,0xCE81,0x0E40,
    0x0A00,0xCAC1,0xCB81,0x0B40,0xC901,0x09C0,0x0880,0xC841,
    0xD801,0x18C0,0x1980,0xD941,0x1B00,0xDBC1,0xDA81,0x1A40,
    0x1E00,0xDEC1,0xDF81,0x1F40,0xDD01,0x1DC0,0x1C80,0xDC41,
    0x1400,0xD4C1,0xD581,0x1540,0xD701,0x17C0,0x1680,0xD641,
    0xD201,0x12C0,0x1380,0xD341,0x1100,0xD1C1,0xD081,0x1040,
    0xF001,0x30C0,0x3180,0xF141,0x3300,0xF3C1,0xF281,0x3240,
    0x3600,0xF6C1,0xF781,0x3740,0xF501,0x35C0,0x3480,0xF441,
    0x3C00,0xFCC1,0xFD81,0x3D40,0xFF01,0x3FC0,0x3E80,0xFE41,
    0xFA01,0x3AC0,0x3B80,0xFB41,0x3900,0xF9C1,0xF881,0x3840,
    0x2800,0xE8C1,0xE981,0x2940,0xEB01,0x2BC0,0x2A80,0xEA41,
    0xEE01,0x2EC0,0x2F80,0xEF41,0x2D00,0xEDC1,0xEC81,0x2C40,
    0xE401,0x24C0,0x2580,0xE541,0x2700,0xE7C1,0xE681,0x2640,
    0x2200,0xE2C1,0xE381,0x2340,0xE101,0x21C0,0x2080,0xE041,
    0xA001,0x60C0,0x6180,0xA141,0x6300,0xA3C1,0xA281,0x6240,
    0x6600,0xA6C1,0xA781,0x6740,0xA501,0x65C0,0x6480,0xA441,
    0x6C00,0xACC1,0xAD81,0x6D40,0xAF01,0x6FC0,0x6E80,0xAE41,
    0xAA01,0x6AC0,0x6B80,0xAB41,0x6900,0xA9C1,0xA881,0x6840,
    0x7800,0xB8C1,0xB981,0x7940,0xBB01,0x7BC0,0x7A80,0xBA41,
    0xBE01,0x7EC0,0x7F80,0xBF41,0x7D00,0xBDC1,0xBC81,0x7C40,
    0xB401,0x74C0,0x7580,0xB541,0x7700,0xB7C1,0xB681,0x7640,
    0x7200,0xB2C1,0xB381,0x7340,0xB101,0x71C0,0x7080,0xB041,
    0x5000,0x90C1,0x9181,0x5140,0x9301,0x53C0,0x5280,0x9241,
    0x9601,0x56C0,0x5780,0x9741,0x5500,0x95C1,0x9481,0x5440,
    0x9C01,0x5CC0,0x5D80,0x9D41,0x5F00,0x9FC1,0x9E81,0x5E40,
    0x5A00,0x9AC1,0x9B81,0x5B40,0x9901,0x59C0,0x5880,0x9841,
    0x8801,0x48C0,0x4980,0x8941,0x4B00,0x8BC1,0x8A81,0x4A40,
    0x4E00,0x8EC1,0x8F81,0x4F40,0x8D01,0x4DC0,0x4C80,0x8C41,
    0x4400,0x84C1,0x8581,0x4540,0x8701,0x47C0,0x4680,0x8641,
    0x8201,0x42C0,0x4380,0x8341,0x4100,0x81C1,0x8081,0x4040
};

uint16_t crc16(const uint8_t* data, uint32_t len) {
    uint16_t crc = 0xFFFF;
    while (len--) {
        crc = (crc >> 8) ^ crc16_table[(crc ^ *data++) & 0xFF];
    }
    return crc;
}

/* =========================================================
 * Carica ROM NDS
 * ========================================================= */
bool nds_load_rom(AppContext* ctx, const char* path) {
    strncpy(ctx->nds_path, path, MAX_PATH - 1);

    ctx->nds_file = fopen(path, "r+b");
    if (!ctx->nds_file) {
        snprintf(ctx->status_msg, 64, "Errore: impossibile aprire");
        return false;
    }

    /* Leggi header */
    fseek(ctx->nds_file, 0, SEEK_SET);
    if (fread(&ctx->header, sizeof(NDSHeader), 1, ctx->nds_file) != 1) {
        fclose(ctx->nds_file);
        ctx->nds_file = NULL;
        snprintf(ctx->status_msg, 64, "Errore: header non valido");
        return false;
    }

    /* Verifica Nintendo Logo CRC */
    uint16_t logo_crc = crc16(ctx->header.nintendo_logo, 156);
    if (logo_crc != 0xCF56) {
        /* Avviso ma non blocca */
        snprintf(ctx->status_msg, 64, "Avviso: CRC logo non valido");
    }

    ctx->rom_loaded   = true;
    ctx->rom_modified = false;
    ctx->fat          = NULL;
    ctx->fat_count    = 0;
    ctx->nitro_file_count = 0;
    ctx->texture_count    = 0;

    snprintf(ctx->status_msg, 64, "ROM: %.12s caricata", ctx->header.game_title);
    ctx->status_timer = 120;

    return true;
}

/* =========================================================
 * Libera ROM dalla memoria
 * ========================================================= */
void nds_free_rom(AppContext* ctx) {
    if (ctx->nds_file) {
        fclose(ctx->nds_file);
        ctx->nds_file = NULL;
    }
    if (ctx->fat) {
        free(ctx->fat);
        ctx->fat = NULL;
    }
    /* Libera dati texture */
    for (uint32_t i = 0; i < ctx->texture_count; i++) {
        if (ctx->textures[i].raw_data) {
            free(ctx->textures[i].raw_data);
            ctx->textures[i].raw_data = NULL;
        }
    }
    ctx->rom_loaded       = false;
    ctx->rom_modified     = false;
    ctx->fat_count        = 0;
    ctx->nitro_file_count = 0;
    ctx->texture_count    = 0;
}

/* =========================================================
 * Legge il filesystem NitroFS (FNT + FAT)
 * ========================================================= */
bool nds_read_nitroFS(AppContext* ctx) {
    if (!ctx->rom_loaded || !ctx->nds_file) return false;

    /* Leggi FAT (File Allocation Table) */
    uint32_t fat_count = ctx->header.fat_size / sizeof(FATEntry);
    if (fat_count == 0 || fat_count > 8192) {
        snprintf(ctx->status_msg, 64, "FAT non valida");
        return false;
    }

    ctx->fat = (FATEntry*)malloc(ctx->header.fat_size);
    if (!ctx->fat) {
        snprintf(ctx->status_msg, 64, "Memoria esaurita (FAT)");
        return false;
    }

    fseek(ctx->nds_file, ctx->header.fat_offset, SEEK_SET);
    if (fread(ctx->fat, ctx->header.fat_size, 1, ctx->nds_file) != 1) {
        free(ctx->fat);
        ctx->fat = NULL;
        snprintf(ctx->status_msg, 64, "Errore lettura FAT");
        return false;
    }
    ctx->fat_count = fat_count;

    /* -------------------------------------------------------
     * Leggi FNT (File Name Table)
     * Il FNT ha una directory root con subtable entries.
     * Formato root entry: subtable_offset(4), first_file_id(2), parent_id(2)
     * ----------------------------------------------------- */
    uint32_t fnt_offset = ctx->header.fnt_offset;
    uint32_t fnt_size   = ctx->header.fnt_size;

    /* Carica tutto l'FNT in memoria */
    uint8_t* fnt_data = (uint8_t*)malloc(fnt_size);
    if (!fnt_data) {
        snprintf(ctx->status_msg, 64, "Memoria esaurita (FNT)");
        return false;
    }

    fseek(ctx->nds_file, fnt_offset, SEEK_SET);
    if (fread(fnt_data, fnt_size, 1, ctx->nds_file) != 1) {
        free(fnt_data);
        snprintf(ctx->status_msg, 64, "Errore lettura FNT");
        return false;
    }

    /* Leggi root directory entry */
    uint32_t root_subtable_off = *(uint32_t*)(fnt_data + 0);
    uint16_t first_file_id     = *(uint16_t*)(fnt_data + 4);
    /* uint16_t total_dirs     = *(uint16_t*)(fnt_data + 6); */

    ctx->nitro_file_count = 0;
    uint32_t cur_file_id  = first_file_id;
    uint32_t pos          = root_subtable_off;

    /* Scansiona subtable della root */
    while (pos < fnt_size && ctx->nitro_file_count < MAX_NITRO_FILES) {
        uint8_t entry_len = fnt_data[pos++];

        if (entry_len == 0x00) break; /* Fine subtable */

        bool is_dir     = (entry_len & 0x80) != 0;
        uint8_t name_len = entry_len & 0x7F;

        if (name_len == 0 || pos + name_len > fnt_size) break;

        NitroFile* nf = &ctx->nitro_files[ctx->nitro_file_count];
        memset(nf, 0, sizeof(NitroFile));

        /* Copia nome file */
        memcpy(nf->name, fnt_data + pos, name_len);
        nf->name[name_len] = '\0';
        pos += name_len;

        nf->is_dir = is_dir;

        if (is_dir) {
            /* Directory: 2 byte ID directory */
            if (pos + 2 > fnt_size) break;
            /* uint16_t dir_id = *(uint16_t*)(fnt_data + pos); */
            pos += 2;
            nf->file_id = 0xFFFF;
        } else {
            /* File: assegna file_id corrente */
            nf->file_id = cur_file_id;
            if (cur_file_id < ctx->fat_count) {
                nf->start_offset = ctx->fat[cur_file_id].start_offset;
                nf->size = ctx->fat[cur_file_id].end_offset - ctx->fat[cur_file_id].start_offset;
            }
            cur_file_id++;
        }

        ctx->nitro_file_count++;
    }

    free(fnt_data);

    snprintf(ctx->status_msg, 64, "%lu file trovati", (unsigned long)ctx->nitro_file_count);
    ctx->status_timer = 90;

    return (ctx->nitro_file_count > 0);
}

/* =========================================================
 * Legge un file dal NitroFS dato il file_id
 * ========================================================= */
bool nds_read_file(AppContext* ctx, uint16_t file_id, uint8_t** out_data, uint32_t* out_size) {
    if (!ctx->rom_loaded || !ctx->nds_file) return false;
    if (file_id >= ctx->fat_count) return false;

    FATEntry* entry = &ctx->fat[file_id];
    uint32_t  size  = entry->end_offset - entry->start_offset;

    if (size == 0 || size > 512 * 1024) return false; /* Max 512KB per file */

    uint8_t* data = (uint8_t*)malloc(size);
    if (!data) return false;

    fseek(ctx->nds_file, entry->start_offset, SEEK_SET);
    if (fread(data, size, 1, ctx->nds_file) != 1) {
        free(data);
        return false;
    }

    *out_data = data;
    *out_size = size;
    return true;
}

/* =========================================================
 * Scrive dati nel file NDS (patch in-place)
 * ========================================================= */
bool nds_write_data(AppContext* ctx, uint32_t offset, const uint8_t* data, uint32_t size) {
    if (!ctx->rom_loaded || !ctx->nds_file) return false;

    fseek(ctx->nds_file, offset, SEEK_SET);
    if (fwrite(data, size, 1, ctx->nds_file) != 1) {
        snprintf(ctx->status_msg, 64, "Errore scrittura ROM!");
        return false;
    }
    fflush(ctx->nds_file);
    ctx->rom_modified = true;
    return true;
}

/* =========================================================
 * Ricalcola e corregge il CRC dell'header NDS
 * ========================================================= */
void nds_fix_header_crc(AppContext* ctx) {
    if (!ctx->rom_loaded || !ctx->nds_file) return;

    /* Leggi i primi 0x15E byte per ricalcolare header CRC */
    uint8_t header_buf[0x160];
    fseek(ctx->nds_file, 0, SEEK_SET);
    fread(header_buf, 0x15E, 1, ctx->nds_file);

    uint16_t new_crc = crc16(header_buf, 0x15E);

    /* Scrivi nuovo CRC a offset 0x15E */
    fseek(ctx->nds_file, 0x15E, SEEK_SET);
    fwrite(&new_crc, 2, 1, ctx->nds_file);
    fflush(ctx->nds_file);
}

/* =========================================================
 * Scansiona texture nel NitroFS (cerca NCGR e NARC con texture)
 * ========================================================= */
bool nds_scan_textures(AppContext* ctx) {
    if (!ctx->rom_loaded || !ctx->nds_file) return false;

    ctx->texture_count = 0;

    /* Estensioni che possono contenere texture */
    const char* tex_exts[] = {
        ".ncg", ".ncgr", ".nbg", ".nbgr",
        ".narc", ".nsbmd", ".nsbtx",
        ".bin", NULL
    };

    for (uint32_t i = 0; i < ctx->nitro_file_count && ctx->texture_count < MAX_TEXTURES; i++) {
        NitroFile* nf = &ctx->nitro_files[i];
        if (nf->is_dir || nf->file_id == 0xFFFF) continue;

        /* Controlla estensione */
        const char* dot = strrchr(nf->name, '.');
        if (!dot) continue;

        bool is_tex = false;
        for (int e = 0; tex_exts[e]; e++) {
            /* Confronto case-insensitive */
            char lower_dot[16];
            int k = 0;
            while (dot[k] && k < 15) {
                char c = dot[k];
                if (c >= 'A' && c <= 'Z') c += 32;
                lower_dot[k] = c;
                k++;
            }
            lower_dot[k] = '\0';
            if (strcmp(lower_dot, tex_exts[e]) == 0) { is_tex = true; break; }
        }
        if (!is_tex) continue;

        /* Leggi inizio file per determinare formato */
        uint8_t sig[8];
        fseek(ctx->nds_file, nf->start_offset, SEEK_SET);
        if (fread(sig, 8, 1, ctx->nds_file) != 1) continue;

        NDSTexture* tex = &ctx->textures[ctx->texture_count];
        memset(tex, 0, sizeof(NDSTexture));
        strncpy(tex->name, nf->name, MAX_FILENAME - 1);

        /* Identifica formato dalla firma */
        if (memcmp(sig, "RGCN", 4) == 0) {
            /* NCGR - Character Graphic Resource */
            tex->format       = TEX_256COLOR;
            tex->data_offset  = nf->start_offset + 32; /* Dopo header */
            tex->data_size    = (nf->size > 32) ? nf->size - 32 : 0;
            tex->width        = 64;
            tex->height       = 64;
            tex->loaded       = false;
            ctx->texture_count++;
        } else if (memcmp(sig, "NARC", 4) == 0) {
            /* NARC archive - aggiungi come entry speciale */
            tex->format       = TEX_NONE; /* Marker NARC */
            tex->data_offset  = nf->start_offset;
            tex->data_size    = nf->size;
            tex->width        = 0;
            tex->height       = 0;
            tex->loaded       = false;
            ctx->texture_count++;
        } else if (memcmp(sig, "BMD0", 4) == 0 || memcmp(sig, "0DMB", 4) == 0) {
            /* NSBMD - 3D model con texture */
            tex->format       = TEX_DIRECT;
            tex->data_offset  = nf->start_offset;
            tex->data_size    = nf->size;
            tex->width        = 64;
            tex->height       = 64;
            tex->loaded       = false;
            ctx->texture_count++;
        } else if (memcmp(sig, "BTX0", 4) == 0 || memcmp(sig, "0XTB", 4) == 0) {
            /* NSBTX - texture pack */
            tex->format       = TEX_DIRECT;
            tex->data_offset  = nf->start_offset;
            tex->data_size    = nf->size;
            tex->width        = 64;
            tex->height       = 64;
            tex->loaded       = false;
            ctx->texture_count++;
        } else if (nf->size >= 8 && nf->size <= 32768) {
            /* File binario generico di dimensione ragionevole */
            tex->format       = TEX_256COLOR;
            tex->data_offset  = nf->start_offset;
            tex->data_size    = nf->size;
            /* Cerca di dedurre dimensioni */
            uint32_t px_count = nf->size; /* 1 byte per pixel (256 colori) */
            if      (px_count >= 128*128) { tex->width = 128; tex->height = 128; }
            else if (px_count >= 64*64)   { tex->width = 64;  tex->height = 64; }
            else if (px_count >= 32*32)   { tex->width = 32;  tex->height = 32; }
            else if (px_count >= 16*16)   { tex->width = 16;  tex->height = 16; }
            else                          { tex->width = 8;   tex->height = 8; }
            tex->loaded = false;
            ctx->texture_count++;
        }
    }

    snprintf(ctx->status_msg, 64, "%lu texture trovate", (unsigned long)ctx->texture_count);
    ctx->status_timer = 90;

    return (ctx->texture_count > 0);
}

/* =========================================================
 * Nome formato texture
 * ========================================================= */
const char* nds_texture_format_name(TextureFormat fmt) {
    switch (fmt) {
        case TEX_NONE:     return "NARC";
        case TEX_A3I5:     return "A3I5";
        case TEX_4COLOR:   return "4Col";
        case TEX_16COLOR:  return "16Col";
        case TEX_256COLOR: return "256Col";
        case TEX_COMP4X4:  return "4x4";
        case TEX_A5I3:     return "A5I3";
        case TEX_DIRECT:   return "Direct";
        default:           return "???";
    }
}
