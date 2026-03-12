/*
 * nds_types.h
 * NDS Texture Editor - Tipi base per NDS homebrew
 * Compatibile con devkitARM + libnds
 */

#ifndef NDS_TYPES_H
#define NDS_TYPES_H

#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <fat.h>
#include <dirent.h>

/* =========================================================
 * Costanti globali
 * ========================================================= */

#define APP_NAME        "NDS Texture Editor"
#define APP_VERSION     "1.0.0"
#define APP_AUTHOR      "NDS Homebrew"

#define MAX_PATH        256
#define MAX_FILES       128
#define MAX_FILENAME    64

/* Dimensioni schermi NDS */
#define NDS_SCREEN_W    256
#define NDS_SCREEN_H    192

/* Colori palette 15bit BGR (formato NDS) */
#define COL_BLACK       RGB15(0,  0,  0)
#define COL_WHITE       RGB15(31, 31, 31)
#define COL_GRAY        RGB15(15, 15, 15)
#define COL_DARKGRAY    RGB15(8,  8,  8)
#define COL_LIGHTGRAY   RGB15(22, 22, 22)
#define COL_CYAN        RGB15(0,  28, 31)
#define COL_PURPLE      RGB15(18, 0,  28)
#define COL_DPURPLE     RGB15(10, 0,  18)
#define COL_MAGENTA     RGB15(31, 0,  20)
#define COL_YELLOW      RGB15(31, 28, 0)
#define COL_RED         RGB15(31, 0,  0)
#define COL_GREEN       RGB15(0,  28, 0)
#define COL_BLUE        RGB15(0,  0,  31)
#define COL_NEON_CYAN   RGB15(0,  31, 31)
#define COL_NEON_PURPLE RGB15(20, 0,  31)
#define COL_ACCENT      RGB15(0,  24, 31)

/* =========================================================
 * Struttura Header NDS ROM
 * ========================================================= */
typedef struct {
    char    game_title[12];      /* Titolo gioco */
    char    game_code[4];        /* Codice gioco */
    char    maker_code[2];       /* Codice produttore */
    uint8_t unit_code;           /* 0=DS, 2=DSi, 3=DS+DSi */
    uint8_t encryption_seed;
    uint8_t device_capacity;
    uint8_t reserved1[7];
    uint8_t nds_region;
    uint8_t rom_version;
    uint8_t autostart;
    uint32_t arm9_rom_offset;
    uint32_t arm9_entry_address;
    uint32_t arm9_load_address;
    uint32_t arm9_size;
    uint32_t arm7_rom_offset;
    uint32_t arm7_entry_address;
    uint32_t arm7_load_address;
    uint32_t arm7_size;
    uint32_t fnt_offset;         /* File Name Table offset */
    uint32_t fnt_size;           /* File Name Table size */
    uint32_t fat_offset;         /* File Allocation Table offset */
    uint32_t fat_size;           /* File Allocation Table size */
    uint32_t arm9_overlay_offset;
    uint32_t arm9_overlay_size;
    uint32_t arm7_overlay_offset;
    uint32_t arm7_overlay_size;
    uint32_t normal_card_control;
    uint32_t secure_card_control;
    uint32_t icon_banner_offset;
    uint16_t secure_area_crc;
    uint16_t secure_transfer_timeout;
    uint32_t arm9_auto_load;
    uint32_t arm7_auto_load;
    uint64_t secure_disable;
    uint32_t ntr_rom_size;
    uint32_t header_size;
    uint8_t  reserved2[56];
    uint8_t  nintendo_logo[156];
    uint16_t nintendo_logo_crc;
    uint16_t header_crc;
} __attribute__((packed)) NDSHeader;

/* =========================================================
 * Struttura FAT Entry (NitroFS)
 * ========================================================= */
typedef struct {
    uint32_t start_offset;
    uint32_t end_offset;
} FATEntry;

/* =========================================================
 * Struttura File in NitroFS
 * ========================================================= */
typedef struct {
    char     name[MAX_FILENAME];
    uint32_t start_offset;
    uint32_t size;
    uint16_t file_id;
    bool     is_dir;
} NitroFile;

/* =========================================================
 * Tipi texture NDS
 * ========================================================= */
typedef enum {
    TEX_NONE     = 0,
    TEX_A3I5     = 1,   /* 8 colori + 8 livelli alpha */
    TEX_4COLOR   = 2,   /* 4 colori palette */
    TEX_16COLOR  = 3,   /* 16 colori palette */
    TEX_256COLOR = 4,   /* 256 colori palette */
    TEX_COMP4X4  = 5,   /* Compresso 4x4 texel */
    TEX_A5I3     = 6,   /* 8 colori + 32 livelli alpha */
    TEX_DIRECT   = 7,   /* Colori diretti 16bit */
} TextureFormat;

/* =========================================================
 * Struttura Texture
 * ========================================================= */
#define MAX_TEXTURE_SIZE  (128 * 128)
#define MAX_PALETTE_SIZE  256

typedef struct {
    char          name[MAX_FILENAME];
    TextureFormat format;
    uint16_t      width;
    uint16_t      height;
    uint32_t      data_offset;    /* Offset nel file NDS */
    uint32_t      data_size;
    uint32_t      palette_offset;
    uint32_t      palette_size;
    uint8_t*      raw_data;       /* Dati grezzi texture */
    uint16_t      palette[MAX_PALETTE_SIZE]; /* Palette BGR15 */
    uint16_t      pixels[MAX_TEXTURE_SIZE];  /* Pixel RGBA15 decodificati */
    bool          loaded;
} NDSTexture;

/* =========================================================
 * Struttura NCGR (Character Graphic Resource)
 * ========================================================= */
typedef struct {
    char     magic[4];    /* "RGCN" */
    uint16_t bom;         /* Byte Order Mark */
    uint16_t version;
    uint32_t file_size;
    uint16_t header_size;
    uint16_t section_count;
} __attribute__((packed)) NCGRHeader;

/* =========================================================
 * Struttura NCLR (Color Resource - Palette)
 * ========================================================= */
typedef struct {
    char     magic[4];    /* "RLCN" */
    uint16_t bom;
    uint16_t version;
    uint32_t file_size;
    uint16_t header_size;
    uint16_t section_count;
} __attribute__((packed)) NCLRHeader;

/* =========================================================
 * Struttura NARC (Archive)
 * ========================================================= */
typedef struct {
    char     magic[4];    /* "NARC" */
    uint16_t bom;
    uint16_t version;
    uint32_t file_size;
    uint16_t header_size;
    uint16_t section_count;
} __attribute__((packed)) NARCHeader;

typedef struct {
    char     magic[4];    /* "BTAF" */
    uint32_t section_size;
    uint16_t file_count;
    uint16_t reserved;
} __attribute__((packed)) BTAFHeader;

typedef struct {
    char     magic[4];    /* "BTNF" */
    uint32_t section_size;
} __attribute__((packed)) BTNFHeader;

typedef struct {
    char     magic[4];    /* "GMIF" */
    uint32_t section_size;
} __attribute__((packed)) GMIFHeader;

/* =========================================================
 * Stato applicazione
 * ========================================================= */
typedef enum {
    STATE_FILE_BROWSER = 0,
    STATE_NITRO_BROWSER,
    STATE_TEXTURE_LIST,
    STATE_TEXTURE_VIEW,
    STATE_PIXEL_EDITOR,
    STATE_PALETTE_EDITOR,
    STATE_SAVE_CONFIRM,
    STATE_INFO,
} AppState;

/* =========================================================
 * Struttura principale applicazione
 * ========================================================= */
#define MAX_TEXTURES    64
#define MAX_NITRO_FILES 256

typedef struct {
    AppState    state;
    char        nds_path[MAX_PATH];
    FILE*       nds_file;
    NDSHeader   header;
    bool        rom_loaded;
    bool        rom_modified;

    /* NitroFS */
    FATEntry*   fat;
    uint32_t    fat_count;
    NitroFile   nitro_files[MAX_NITRO_FILES];
    uint32_t    nitro_file_count;

    /* Texture */
    NDSTexture  textures[MAX_TEXTURES];
    uint32_t    texture_count;
    int         selected_texture;

    /* File browser */
    char        current_dir[MAX_PATH];
    char        file_list[MAX_FILES][MAX_FILENAME];
    uint32_t    file_count;
    int         file_cursor;
    int         file_scroll;

    /* Nitro browser */
    int         nitro_cursor;
    int         nitro_scroll;

    /* Texture list */
    int         tex_list_cursor;
    int         tex_list_scroll;

    /* Pixel editor */
    int         px_cursor_x;
    int         px_cursor_y;
    int         px_zoom;        /* 1=8x, 2=4x, 3=2x, 4=1x */
    int         px_offset_x;
    int         px_offset_y;
    int         selected_color; /* Colore selezionato in palette */
    bool        px_drawing;

    /* Texture view */
    int         view_zoom;
    int         view_offset_x;
    int         view_offset_y;

    /* Messaggi */
    char        status_msg[64];
    int         status_timer;

    /* Input touchscreen */
    touchPosition touch;
    bool          touching;

} AppContext;

#endif /* NDS_TYPES_H */
