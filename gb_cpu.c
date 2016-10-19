#include "gb_cpu.h"
#include "gb_init.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// bytes per line in RAM dumps
#define BYTES_PER_LINE 32

// #define SLOWDOWN_ENABLED


void win_sleep(unsigned int cycles)
{
	HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(20*cycles);

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}

void gb_cpu_print(gb_cpu* cpu, uint8_t current_instruction, uint8_t next_instruction, unsigned int frame_number)
{

	printf("------- FRAME %d ---------\n", frame_number);
	printf("Just executed: %.2x, Next instruction: %.2x\n", current_instruction, next_instruction);
	printf("Registers:\n");
	printf("\t A (accumulator): %.2x\n", cpu->reg.acc);
	printf("\t B: %.2x\n", cpu->reg.b);
	printf("\t C: %.2x\n", cpu->reg.c);
	printf("\t D: %.2x\n", cpu->reg.d);
	printf("\t E: %.2x\n", cpu->reg.e);
	printf("\t HL: %.4x\n", cpu->reg.hl);
	printf("\t flags: %.2x\n", cpu->reg.flags.direct);
	printf("\t\t zero: %.1d\n", cpu->reg.flags.by_flag.zero);
	printf("\t\t subtract: %.1d\n", cpu->reg.flags.by_flag.subtract);
	printf("\t\t half_carry: %.1d\n", cpu->reg.flags.by_flag.half_carry);
	printf("\t\t carry: %.1d\n", cpu->reg.flags.by_flag.carry);
	printf("Program Counter: %.2x\n", cpu->pc);
	printf("Stack Pointer: %.2x\n", cpu->sp);
	printf("Halted: %s\n", cpu->halted ? "true" : "false" );
}

void execute_extended_instruction(gb_cpu* cpu, uint8_t* ram)
{
	uint8_t instruction = ram[cpu->pc + 1];
	int instruction_timing = -1, instruction_length = -1;
	
	switch(instruction)
	{
		// RL C (len: 2, timing: 8)
		case 0x11:
		{
		// NOTE: I am not 100% sure the semantics of this instruction are correctly implemented.
		// if we get bugs, they are probably caused by this instruction.
			// bit 7 becomes the carry flag, the carry flag becomes bit zero
			unsigned int bit_zero = cpu->reg.flags.by_flag.carry;
			cpu->reg.flags.by_flag.carry = (cpu->reg.c >> 7) & 1;
			cpu->reg.c = (cpu->reg.c << 1) | (cpu->reg.c >> 1);
			cpu->reg.c ^= bit_zero;
			instruction_length = 2;
			instruction_timing = 8;
			break;
		}

		// BIT 7, H (len: 2, timing: 8)
		case 0x7C:
			instruction_length = 2;
			instruction_timing = 8;
			// semantics: set zero flag to 1 if bit 7 (most significant) of register H is zero, otherwise clear zero flag.
			cpu->reg.flags.by_flag.zero = (((cpu->reg.hl >> 15) & 1) == 0) ? 1 : 0;
			break;
		default:
			printf("Exception occurred: unknown extended instruction 0xCB%.2x at address %.4x\n", instruction, cpu->pc);
			cpu->halted = 1;
			break;
	}

	if(instruction_length == -1)
	{
		printf("[WARN] instruction length not set for extended instruction 0xCB%.2x\n", instruction);
		instruction_length = 1;
	}
	if(instruction_timing == -1)
	{
		printf("[WARN] instruction timing not set for extended instruction 0xCB%.2x\n", instruction);
	}

	cpu->pc += instruction_length;

	#ifdef SLOWDOWN_ENABLED
	usleep(2 * instruction_timing);
	#endif
}

void gb_cpu_tick(gb_cpu* cpu, uint8_t* ram, unsigned int frame_number)
{
	uint8_t current_instruction = ram[cpu->pc];
	// instruction timing tells us how long to wait
	int instruction_timing = -1;
	int instruction_length = -1;

	// literally just a monolithic switch statement, I know.
	// sorry, mom.
	switch(current_instruction)
	{
		// DEC B (len: 1, timing: 4)
		case 0x05:
			instruction_length = 1;
			instruction_timing = 4;
			cpu->reg.b--;
			break;

		// LD B, imm8 (len: 2, timing: 8)
		case 0x06:
			instruction_timing = 8;
			instruction_length = 2;
			cpu->reg.b = ram[cpu->pc + 1];
			break;

		// LD C, imm8 (len: 2, timing: 8)
		case 0x0E:
			instruction_length = 2;
			instruction_timing = 8;
			cpu->reg.c = ram[cpu->pc + 1];
			break;

		// LD DE, imm16 (len: 3, timing: 12)
		case 0x11:
			instruction_length = 3;
			instruction_timing = 12;
			cpu->reg.e = ram[cpu->pc + 1];
			cpu->reg.d = ram[cpu->pc + 2];
			break;

		// RLA (len: 1, timing: 4)
		case 0x17:
		{
		// NOTE: I am not 100% sure the semantics of this instruction are correctly implemented.
		// if we get bugs, they are probably caused by this instruction.
			// bit 7 becomes the carry flag, the carry flag becomes bit zero
			unsigned int bit_zero = cpu->reg.flags.by_flag.carry;
			cpu->reg.flags.by_flag.carry = (cpu->reg.acc >> 7) & 1;
			cpu->reg.acc = (cpu->reg.acc << 1) | (cpu->reg.acc >> 1);
			cpu->reg.acc ^= bit_zero;
			instruction_length = 1;
			instruction_timing = 4;
			break;
		}

		// LD A, (DE) (len: 1, timing: 8)
		case 0x1A:
			instruction_length = 1;
			instruction_timing = 8;
			cpu->reg.acc = ram[(cpu->reg.d << 8) | cpu->reg.e];
			break;

		// JR NZ, r8 (len: 2, timing: 12/8)
		case 0x20:
			instruction_length = 2;
			if(cpu->reg.flags.by_flag.zero == 0)
			{
				cpu->pc += (int8_t) ram[cpu->pc + 1];
				instruction_timing = 12;
			}
			else
			{
				instruction_timing = 8;
			}
			break;
		// LD HL, imm16 (len: 3, timing: 12)
		case 0x21:
			instruction_timing = 12;
			instruction_length = 3;
			cpu->reg.hl = ram[cpu->pc + 1] | (ram[cpu->pc + 2] << 8);
			break;

		// LD (HL+), A (len: 1, timing: 8)
		case 0x22:
			ram[cpu->reg.hl] = cpu->reg.acc;
			// nb: missing break on purpose, we take advantage of the fact that most of this instruction
			// is the same as INC HL because we are a cheeky cunt
		// INC HL (len: 1, timing: 8)
		case 0x23:
			cpu->reg.hl++;
			instruction_timing = 8;
			instruction_length = 1;

			break;

		// LD SP, imm16 (len: 3, timing: 12)
		case 0x31:
			instruction_timing = 12;
			instruction_length = 3;
			cpu->sp = ram[cpu->pc + 1] | (ram[cpu->pc + 2] << 8);
			break;
		// LD (HL-), A (len: 1, timing: 8)
		case 0x32:
			instruction_timing = 8;
			instruction_length = 1;
			ram[cpu->reg.hl--] = cpu->reg.acc;
			break;
		// LD A, imm8 (len: 2, timing: 8)
		case 0x3e:
			instruction_timing = 8;
			instruction_length = 2;
			cpu->reg.acc = ram[cpu->pc + 1];
			break;

		// LD C, A (len: 1, timing: 4)
		case 0x4F:
			instruction_timing = 4;
			instruction_length = 1;
			cpu->reg.c = cpu->reg.acc;
			break;

		// LD (HL), A (len: 1, timing: 8)
		case 0x77:
			instruction_length = 1;
			instruction_timing = 8;
			ram[cpu->reg.hl] = cpu->reg.acc;
			break;


		// XOR A (len: 1, timing: 4)
		case 0xAF:
			instruction_timing = 4;
			instruction_length = 1;
			cpu->reg.acc ^= cpu->reg.acc;
			cpu->reg.flags.by_flag.zero = 1; // since we XOR A with A, it should always produced a Z flag of 1
			break;

		// POP BC (len: 1, timing: 12)
		case 0xC1:
			instruction_timing = 12;
			instruction_length = 1;

			// I'm assuming they come back off the stack the same way they went on (little-endian)
			// if we get bugs, this is another possible culprit
			cpu->reg.b = ram[cpu->sp];
			cpu->sp++;
			cpu->reg.c = ram[cpu->sp];
			cpu->sp++;

			break;

		// PUSH BC (len: 1, timing: 16)
		case 0xC5:
			instruction_timing = 16;
			instruction_length = 1;

			// I'm assured that the bytes appear on the stack in little-endian order
			// that is, when the instruction is finished SP should point at C and SP+1 should point at B
			cpu->sp--;
			ram[cpu->sp] = cpu->reg.b;
			cpu->sp--;
			ram[cpu->sp] = cpu->reg.c;

			break;

		// RET (len: 1, timing: 16)
		case 0xC9:
			// NB: treat RET as a zero-length instruction because it's an effective jump
			instruction_length = 0;
			// NOTE: timing discrepency between manual and opcode map.  the manual says 8 cycles, the map says 16.
			instruction_timing = 16;

			unsigned char rah = 0x00;
			unsigned char ral = 0x00;

			rah = ram[cpu->sp];
			cpu->sp++;
			ral = ram[cpu->sp];
			cpu->sp++;

			cpu->pc = (rah << 8) | ral;

			printf("Returning to %.4x\n", cpu->pc);

			break;

		// PREFIX CB (extended instruction)
		case 0xCB:
			// NB: timing and length are handled in the execute_extended_instruction function
			// so we set them to zero here because we don't need to handle them in this function
			instruction_timing = 0;
			instruction_length = 0;
			execute_extended_instruction(cpu, ram);
			break;

		// CALL imm16 (len: 3, timing: 24)
		case 0xCD:
			instruction_timing = 24;
			instruction_length = 0;

			// start by pushing the next instruction's address onto the stack and decrementing the stack pointer
			cpu->sp--;
			ram[cpu->sp] = (cpu->pc + 3) & 0xFF; // the lower byte
			cpu->sp--;
			ram[cpu->sp] = ((cpu->pc + 3) & 0xFF00) >> 8; // the upper byte

			// this was kind of useful while debugging so I'm leaving it as a comment
			printf("Jumping to: %.4x\n", ram[(cpu->pc + 2) << 8] | ram[cpu->pc + 1]);

			printf("expecting to return to: %.4x\n", ram[cpu->sp+1] | (ram[cpu->sp+2] << 8));

			// now jump to the specified address
			cpu->pc = ram[(cpu->pc + 2) << 8] | ram[cpu->pc + 1];

			break;

		// LDH (imm8), A (len: 2, timing: 12)
		case 0xE0:
			instruction_timing = 12;
		 	instruction_length = 2;
		 	ram[0xFF00 + ram[cpu->pc + 1]] = cpu->reg.acc;
		 	break;

		// LD (C), A (len: 2, timing: 8)
		case 0xE2:
			instruction_timing = 8;
			instruction_length = 2;
			ram[cpu->reg.c] = cpu->reg.acc;
			break;
		default:
			printf("Exception occurred: unknown instruction %.2x on frame %d, PC=%.4x\n", current_instruction, frame_number, cpu->pc);
			cpu->halted = 1;
			break;
	}
	if(instruction_timing == -1)
	{
		printf("[WARN] Instruction timing not set for opcode %.2x\n", current_instruction);
		instruction_timing = 1;
	}
	if(instruction_length == -1)
	{
		printf("[WARN] Instruction length not set for opcode %.2x\n", current_instruction);
		instruction_length = 1;
	}

	cpu->pc += instruction_length;

	if(cpu->debug_mode != 0 && cpu->skip_debug_frames == 0 &&
		(cpu->pc == cpu->target_address || cpu->target_address == NO_TARGET_ADDRESS)
	)
	{
		int cont = 0;
		char* line = malloc(32);
		unsigned int argc = 0;
		// maximum 32 arguments
		char** argv = malloc(32 * sizeof (char*));
		// print out current state of cpu
		gb_cpu_print(cpu, current_instruction, ram[cpu->pc], frame_number);
		// wait for input
		while(cont == 0)
		{
			printf("> ");
			fflush(stdout);
			fgets(line, 256, stdin);
			memset(argv, 0, (32 * sizeof (char*)));
			argv[0] = strtok(line, " ");
			argc = 0;
			while((argv[++argc] = strtok(NULL, " ")) != NULL);
			if(argc > 0)
			{
				if(strncmp(argv[0], "skip", 4) == 0)
				{
					cpu->skip_debug_frames = atoi(argv[1]);
					cont = 1;
				}
				else if(strncmp(argv[0], "quit", 4) == 0)
				{
					cpu->halted = 1;
					cont = 1;
				}
				else if (strncmp(argv[0], "run", 3) == 0)
				{
					cpu->debug_mode = 0;
					cont = 1;
				}
				else if (strncmp(argv[0], "dump", 4) == 0)
				{
					unsigned int offset = 0, address = 0, output_bytes = 256;

					if(argc > 1) offset = atoi(argv[1]);
					if(argc > 2) output_bytes = atoi(argv[2]);

					address = offset;

					while(address < offset + output_bytes)
					{
						printf("%.2x", ram[address]);
						if(address % 2 == 0) putchar(' ');
						if(address % 0x10 == 0) printf("\n%.4x: ", address);
						address++;
					}
					putchar('\n');
				}
				else if (strncmp(argv[0], "find", 4) == 0)
				{
					if (argc == 1)
					{
						printf("Cannot 'find' without you giving me the opcode to find, dumbass.\n");
					}
					else
					{
						cpu->debug_mode = 0;
						while(ram[cpu->pc] != strtol(argv[1], NULL, 16) && cpu->halted == 0)
						{
							gb_cpu_tick(cpu, ram, frame_number++);
						}
						gb_cpu_print(cpu, ram[cpu->pc], ram[cpu->pc], frame_number);
						cpu->debug_mode = 1;
					}
				}
				else if (strncmp(argv[0], "goto", 4) == 0)
				{
					if(argc > 1)
					{
						cpu->target_address = strtol(argv[1], NULL, 16);
						printf("Going to %.4x\n", cpu->target_address);
						cont = 1;
					}
					else printf("Cannot goto without an address to go to.\n");
					
				}
				else if (argv[0][0] == 'n' || argv[0][0] == '\n')
				{
					cont = 1; // just go to the next frame
				}
				else
				{
					printf("Unrecognised command %s\n", argv[0]);
				}
			}
		}
		free(line);
		free(argv);
	}
	else
	{
		cpu->skip_debug_frames--;
		#ifdef SLOWDOWN_ENABLED
		// TODO: sleep for ~0.2 microseconds using nanosleep(), for now we'll just sleep for 2 microseconds and hope that the 10x slower clock doesn't fuck anything up
		win_sleep(instruction_timing);
		#endif
	}
}

void gb_start_cpu(gb_cpu* cpu, uint8_t* ram)
{
	unsigned int frame_number = 0;
	cpu->halted = 0;
	while(cpu->halted == 0)
	{
		gb_cpu_tick(cpu, ram, frame_number++);
	}
}