#ifndef GB_CPU_H
#define GB_CPU_H

#include <stdint.h>

typedef struct gb_cpu_str {
	struct register_str {
		uint8_t acc;
		uint8_t b;
		uint8_t c;
		uint8_t d;
		uint8_t e;
		uint16_t hl;
		union flag_union {
			uint8_t direct;
			struct flag_str {
				uint8_t zero: 1;
				uint8_t subtract: 1;
				uint8_t half_carry: 1;
				uint8_t carry: 1;
				uint8_t do_not_use: 4;
			} by_flag;
		} flags;
	} reg;
	uint16_t sp;
	uint16_t pc;
	unsigned char halted;
	unsigned char debug_mode;
	unsigned int skip_debug_frames;
	unsigned int target_address;
} gb_cpu;

// must be > GAMEBOY_RAM_SIZE
#define NO_TARGET_ADDRESS 0x10000

void gb_start_cpu(gb_cpu* cpu, uint8_t* ram);

#endif //GB_CPU_H