#include "gb_cartridge.h"
#include <string.h>

void gb_populate_cartridge_header(unsigned char* cartridge, gb_cartridge_header* header)
{
	header->type = cartridge[0x0147];
	memcpy(header->game_title, cartridge + 0x0134, 16);
	header->gameboy_type_flags = 
		(cartridge[0x0143] == 0x80 ? GB_FLAG_COLOR : 0x00) |
		(cartridge[0x0146] == 0x03 ? GB_FLAG_SUPER : 0x00);

	// if the high nibble of 0x0148 is unset, we can cheat
	// and set the rom size to 2 << [0x0148]
	// because the number of ROM banks (that is, the number of 16 KB pages of ROM)
	// is = 2 << [0x0148] in this case
	if((cartridge[0x0148] & 0xF0) == 0x00)
	{
		header->rom_size = 2 << cartridge[0x0148];
	} 
	else
	{
		switch(cartridge[0x0148])
		{
			case 0x52:
				header->rom_size = 72;
				break;
			case 0x53:
				header->rom_size = 80;
				break;
			case 0x54:
				header->rom_size = 96;
				break;
		}
	}
}