#ifndef GB_INIT_H
#define GB_INIT_H

#include "gb_cpu.h"
#include "gb_cartridge.h"

// we define the gameboy as having all addressable RAM in one array for simplicity's sake
// this means that when we load a "cart" that has RAM, the extra ram is still addressable.
// it also means that the VRAM appears (correctly) at 0x8000~0x9FFF
// technically some of this "ram" is the cartridge, some of it is I/O, and other shit
// but we don't really care.
#define GAMEBOY_RAM_SIZE 0x10000

void gb_init_system(gb_cpu* processor, uint8_t* ram);
void gb_load_cart(uint8_t* ram, uint8_t* cartridge);

#endif