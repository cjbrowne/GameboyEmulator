#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include "gb_cpu.h"

void gb_start_graphics_subsystem(gb_cpu* processor, uint8_t* ram, char* title);
void gb_shutdown_graphics_subsystem(void);

#endif // GRAPHICS_H