#include <string.h>
#include <stdio.h>
#include "gb_init.h"

#define BOOTROM_PATH "bootrom.bin"
#define BOOTROM_SIZE 256
#define MAGIC_BOOTROM_CHECKSUM 0x626E

// NB: checksum is a literal sum of every byte in the bootrom
// pros: fast and easy to implement
// cons: high possibility of a false positive
// but we don't really care that much because we're just checking that the bootrom actually loaded
// if someone wants to load a custom bootrom then they can easily "trick" this checksum
// but frankly, my dear, I just don't give a damn.
void verify_bootrom_integrity(uint8_t* ram, size_t bootrom_size)
{
	unsigned int i = 0;
	unsigned int checksum = 0;
	for(i = 0; i < bootrom_size; i++)
	{
		checksum += ram[i];
	}
	if(checksum == MAGIC_BOOTROM_CHECKSUM)
	{
		printf("Bootrom verified correct. \n");
	}
	else
	{
		printf("Bootrom invalid, unloading...\n");
		memset(ram, 0, BOOTROM_SIZE);
	}
}

void gb_init_system(gb_cpu* cpu, uint8_t* ram)
{
	memset(cpu, 0, sizeof (gb_cpu));
	memset(ram, 0, GAMEBOY_RAM_SIZE);

	cpu->target_address = NO_TARGET_ADDRESS;

	FILE* bootrom = fopen(BOOTROM_PATH, "rb");
	if(bootrom)
	{
		size_t bootrom_read_success = fread(ram, BOOTROM_SIZE, 1, bootrom);
		if(!bootrom_read_success)
		{
			printf("Warning: bootrom was not read successfully.  The boot rom may be corrupt.\n");
		}
		verify_bootrom_integrity(ram, BOOTROM_SIZE);
		fclose(bootrom);
	}
	else
	{
		printf("Error loading boot rom: Could not find %s\n", BOOTROM_PATH);
	}
}

void gb_load_cart(uint8_t* ram, uint8_t* cartridge)
{
	// to begin with we skip the first 256 bytes of RAM because they should be initialised to the bootrom
	// we copy the first bank of ROM into memory at 0x0100, making sure to skip the first 256 bytes.
	// the ROM should end up in memory at 0x0100 ~ 0x4000
	memcpy(ram + 0x0100, cartridge + 0x0100, 0x3f00);

	// the second ROM bank ends up in 0x4000 ~ 0x8000 and is switchable in carts with > 32 kB of ROM
	memcpy(ram + 0x4000, cartridge + 0x4000, 0x4000);
}