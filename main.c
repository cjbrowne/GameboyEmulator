#include <stdio.h>
#include <stdlib.h>
#include "gb_cpu.h"
#include "gb_init.h"
#include "gb_cartridge.h"
#include "graphics.h"

// maximum supported cartridge is currently 1 MB (1024x1024 bytes)
// no doubt this will increase in the future
#define MAX_CART_SIZE 1048576

int main(int argc, char** argv)
{
	printf("Gameboy Emulator \u00A92016 Chris Browne\n");
	gb_cpu* processor = malloc(sizeof (gb_cpu));
	uint8_t* ram = malloc(GAMEBOY_RAM_SIZE);
	uint8_t* cartridge = malloc(MAX_CART_SIZE);
	gb_cartridge_header cartridge_header;
	FILE* cartridge_file = NULL;

	if(argc == 1)
	{
		printf("Usage: %s romfile\n", argv[0]);
	}
	else
	{
		cartridge_file = fopen(argv[1], "rb");
		if(cartridge_file == NULL)
		{
			printf("Error: could not open %s for reading.\n", argv[1]);
		}
		else
		{
			size_t cartridge_size = fread(cartridge, 1, MAX_CART_SIZE, cartridge_file);

			if(cartridge_size % 1024 != 0)
			{
				printf("Warning: cartridge not exact multiple of 1024, this may cause issues with reading the cartridge.\n");
			}

			gb_populate_cartridge_header(cartridge, &cartridge_header);

			if(cartridge_header.gameboy_type_flags & GB_FLAG_COLOR)
			{
				printf("Error: incompatible cartridge for gameboy color found.  Gameboy color support will come later.\n");
			}
			else
			{
				gb_init_system(processor, ram);

				gb_start_graphics_subsystem(processor, ram, cartridge_header.game_title);

				printf("Cartridge %s loading...", cartridge_header.game_title);

				gb_load_cart(ram, cartridge);

				printf("Done\n");

				
				// TODO: command-line param for debug mode
				#ifdef DEBUG
				processor->debug_mode = 1;
				#endif

				gb_start_cpu(processor, ram);
			}

		}
	}
	
	gb_shutdown_graphics_subsystem();
	free(ram);
	free(processor);
	return 0;
}