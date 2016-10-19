#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// TODO: command line parameters would be better
#define BOOTROM_SIZE 256

int main(void)
{
	FILE* bootrom_file = fopen("bootrom.bin", "r");
	uint8_t* bootrom = malloc(BOOTROM_SIZE);

	fread(bootrom, BOOTROM_SIZE, 1, bootrom_file);
	
	unsigned int checksum = 0;

	unsigned int i = 0;

	for(i = 0; i < BOOTROM_SIZE; i++)
	{
		checksum += bootrom[i];
	}

	printf("Checksum: %x\n", checksum);

	return 0;
}