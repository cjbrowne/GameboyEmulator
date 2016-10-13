#ifndef GB_CARTRIDGE_H
#define GB_CARTRIDGE_H

enum cartridge_type {
	ROM_ONLY 					= 0x00,
	ROM_MBC1 					= 0x01,
	ROM_MBC1_RAM 				= 0x02,
	ROM_MBC1_RAM_BATT			= 0x03,
	ROM_MBC2					= 0x05,
	ROM_MBC2_BATT				= 0x06,
	ROM_RAM 					= 0x08,
	ROM_RAM_BATT 				= 0x09,
	ROM_MMM01					= 0x0B,
	ROM_MMM01_SRAM				= 0x0C,
	ROM_MMM01_SRAM_BATT 		= 0x0D,
	ROM_MBC3_TIMER_BATT			= 0x0F,
	ROM_MBC3_TIMER_RAM_BATT		= 0x10,
	ROM_MBC3 					= 0x11,
	ROM_MBC3_RAM 				= 0x12,
	ROM_MBC3_RAM_BATT 			= 0x13,
	ROM_MBC5 					= 0x19,
	ROM_MBC5_RAM				= 0x1A,
	ROM_MBC5_RAM_BATT   		= 0x1B,
	ROM_MBC5_RUMBLE				= 0x1C,
	ROM_MBC5_RUMBLE_SRAM		= 0x1D,
	ROM_MBC5_RUMBLE_SRAM_BATT   = 0x1E,
	POCKET_CAMERA				= 0x1F,
	BANDAI_TAMA5				= 0xFD,
	HUDSON_HUC_3				= 0xFE,
	HUDSON_HUC_5				= 0xFF
};

#define GB_FLAG_SUPER 0x01
#define GB_FLAG_COLOR 0x02

typedef struct gb_cartridge_header_str {
	enum cartridge_type type;
	char game_title[16];
	// bit map: (bit 0 is least significant)
	// 0: super gameboy
	// 1: gameboy color
	// 2~8: not used
	// NB: if this value is not 0x00 we will declare it incompatible and exit.
	// 0x0143 in the cartridge tells us if it's color (if the value of 0x0143 is 0x80 then it's color gb, any other value is regular)
	// 0x0146 indicates if it's super gameboy (0x03 is super gameboy, any other value is gameboy)
	char gameboy_type_flags;
	unsigned int rom_size;
} gb_cartridge_header;

void gb_populate_cartridge_header(unsigned char* cartridge, gb_cartridge_header* header);

#endif // GB_CARTRIDGE_H