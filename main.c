#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
//#include "graphics.h"

#define CHUNK 10
#define STACK 32

void decode(int opcode);
int ldROM(char* path);
int fetch(char* ram2);
void end();

unsigned char registers[16] = { 0 };
int registerI = 0;
int registerF = 0;
unsigned char ram[4096];
int pc = 512;
unsigned short stack[16];
unsigned char sp = 0;

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("Usage: ./emulator [Rom name]\n");
		exit(0);
	}
    int i;
    int opcode = 0;
	int fileSize = ldROM(argv[1]);
    
    for (i = 0; i < 1460; i++)
    {
        unsigned short opcode = (unsigned char)ram[pc] << 8 | (unsigned char)ram[pc + 1];
        pc += 2; 
        decode(opcode);
    }

	printf("\nEND OF PROGRAM\n");
	return 0;
}

/*
 * FETCH
 * Desc: load the opcode from the ram and return the opcode as integer.
 * Output: the opcode as little endian integer.
 */
int fetch(char *ram2)
{   
    int opcode = 0;
    unsigned char strOPCODE[2] = {0};
    
    memcpy(strOPCODE, ram2, 2);
    
    //change the order of the bytes in memory because of little endian.
    opcode += strOPCODE[0];
    opcode = (opcode << 8) + strOPCODE[1];

    return opcode;
}

/*
 * ldROM
 * Desc: Loads the ROM raw bytes to the RAM at 0x200.
 * Input: ROM file path.
 * Output: ROM file size.
 */
int ldROM(char* path)
{
	int read_bytes = 0;

	int fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		printf("Cannot load the Rom file...\n");
		exit(0);
	}
	read_bytes = read(fd, ram + 512, 4096 - 512);
	return read_bytes;
}

/*
 * DECODE
 * Desc: Decode the opcode and execute the opcode.
 * Input: Opcode.
 */
void decode(int opcode)
{
    int opcodeDATA = opcode & 0x0FFF;
    int i = 0;
    int rnd = 0;

    printf("%04x: %04x\n", pc-2, opcode);
    switch(opcode >> 12)
    {
        case 0:
            if (opcode == 0x00E0)
            {
                printf("CLS\n");
            }
            else if (opcode == 0x00EE)
            {
                sp--;   
                pc = stack[sp]; 
                printf("stack[%d] = %x\n", sp, stack[sp]);
            }
            else
            {
                pc = opcodeDATA;
            }
            break;
        case 1:
            pc = opcodeDATA;
            break;
        case 2:
            printf("1. stack[%d] = %x\n", sp, stack[sp]);
            stack[sp] = pc; 
            printf("2. stack[%d] = %x\n", sp, stack[sp]);
            sp++;
            pc = opcodeDATA;
            break;
        case 3: 
            printf("V[%d] = %x\n", (opcodeDATA >> 8), registers[(opcodeDATA >> 8)]);
            if (registers[(opcodeDATA >> 8)] == (opcodeDATA & 0xFF))
            {
                pc += 2;
            }
            break;
        case 4:
            if (registers[(opcodeDATA >> 8)] != (opcodeDATA & 0xFF))
            {
                pc += 2;
            }
            break;
        case 5:
            if (registers[(opcodeDATA >> 8)] == registers[((opcodeDATA & 0xFF) >> 4)])
            {
                pc += 2;
            }
            break;
        case 6:
            printf("1. V[%d] = %x\n", (opcodeDATA >> 8), registers[(opcodeDATA >> 8)]);
            registers[(opcodeDATA >> 8)] = opcodeDATA & 0xFF;
            printf("2. V[%d] = %x\n", (opcodeDATA >> 8), registers[(opcodeDATA >> 8)]);
            break;
        case 7:
            printf("1. V[%d] = %x\n", (opcodeDATA >> 8), registers[(opcodeDATA >> 8)]);
            registers[(opcodeDATA >> 8)] += (opcodeDATA & 0xFF); 
            printf("2. V[%d] = %x\n", (opcodeDATA >> 8), registers[(opcodeDATA >> 8)]);
            break;
        case 8:
            if ((opcodeDATA & 0xF) == 0x0)
            {
                printf("V[%d] = %x | V[%d] = %x\n", (opcodeDATA >> 8), registers[(opcodeDATA >> 8)], (opcodeDATA >> 4) & 0x0F, registers[(opcodeDATA >> 4) & 0x0F]);
                registers[(opcodeDATA >> 8)] = registers[((opcodeDATA & 0xFF) >> 4)];
                printf("V[%d] = %x\n", (opcodeDATA >> 8), registers[(opcodeDATA >> 8)]);
            }
            else if ((opcodeDATA & 0xF) == 0x1)
            { 
                registers[(opcodeDATA >> 8)] |= registers[((opcodeDATA & 0xFF) >> 4)];
            }
            else if ((opcodeDATA & 0xF) == 0x2)
            { 
                registers[(opcodeDATA >> 8)] &= registers[((opcodeDATA & 0xFF) >> 4)];
            }
            else if ((opcodeDATA & 0xF) == 0x3)
            { 
                registers[(opcodeDATA >> 8)] ^= registers[((opcodeDATA & 0xFF) >> 4)];
            }
            else if ((opcodeDATA & 0xF) == 0x4)
            { 
                printf("1. V[%d] = %x\n", (opcodeDATA >> 8), registers[(opcodeDATA >> 8)]);
                if ((registers[(opcodeDATA >> 8)] + registers[((opcodeDATA & 0xFF) >> 4)]) > 255)
                {
                   registerF = 1;
                }
                else
                {
                    registerF = 0;
                }
                registers[(opcodeDATA >> 8)] += registers[((opcodeDATA & 0xFF) >> 4)];
                printf("2. V[%d] = %x\n", (opcodeDATA >> 8), registers[(opcodeDATA >> 8)]);
            }
            else if ((opcodeDATA & 0xF) == 0x5)
            { 
                if (registers[(opcodeDATA >> 8)] > registers[((opcodeDATA & 0xFF) >> 4)])
                {
                   registerF = 1;
                }
                else
                {
                    registerF = 0;
                }
                registers[(opcodeDATA >> 8)] -= registers[((opcodeDATA & 0xFF) >> 4)];
            }
            else if ((opcodeDATA & 0xF) == 0x6)
            {
                printf("V[%d] = %x\n", (opcodeDATA >> 8), registers[(opcodeDATA >> 8)]);
                if ((registers[(opcodeDATA >> 8)] & 0b1))
                {
                    registerF = 1;
                }
                else
                {
                    registerF = 0;
                }
                registers[(opcodeDATA >> 8)] = registers[(opcodeDATA >> 8)] >> 1; 
                printf("FLAG = %d | V[%d] = %x\n", registerF,(opcodeDATA>>8),registers[(opcodeDATA >> 8)]);
            }
            else if ((opcodeDATA & 0xF) == 0x7)
            { 
                if (registers[(opcodeDATA >> 8)] < registers[((opcodeDATA & 0xFF) >> 4)])
                {
                   registerF = 1;
                }
                else
                {
                    registerF = 0;
                }
                registers[(opcodeDATA >> 8)] = registers[((opcodeDATA & 0xFF) >> 4)] - registers[(opcodeDATA >> 8)];
            }
            else if ((opcodeDATA & 0xF) == 0xE)
            {
                printf("V[%d] = %x\n", (opcodeDATA >> 8), registers[(opcodeDATA >> 8)]);
                if ((registers[(opcodeDATA >> 8)] & 0b10000000))
                {
                    registerF = 1;
                }
                else
                {
                    registerF = 0;
                }
                registers[(opcodeDATA >> 8)] = registers[(opcodeDATA >> 8)] << 1;  
                printf("FLAG = %d | V[%d] = %x\n", registerF,(opcodeDATA>>8),registers[(opcodeDATA >> 8)]);
            }

            break;
        case 9:
            printf("V[%d] = %x\n", (opcodeDATA >> 8), registers[(opcodeDATA >> 8)]); 
            printf("V[%d] = %x\n", ((opcodeDATA & 0xFF) >> 4), registers[((opcodeDATA & 0xFF) >> 4)]);
            if (registers[(opcodeDATA >> 8)] != registers[((opcodeDATA & 0xFF) >> 4)])
            {
                pc += 2;
            }
            break;
        case 0xA:
            registerI = opcodeDATA;
            printf("I = %x\n", opcodeDATA);
            break;
        case 0xB:
            pc = registers[0] + opcodeDATA;
            break;
        case 0xC:
            srand(time(NULL));
            rnd = (rand() % 255);
            registers[(opcodeDATA >> 8)] = rnd & (opcodeDATA & 0xFF);
            break;
        case 0xD:
            printf("Display n-byte sprite\n");
            break;
        case 0xE:
            if ((opcodeDATA & 0xF) == 0xE)
            {
                printf("Keyboard is pressed\n");
            }
            else
            {
                printf("Keyboard isnt pressed\n");
            }
            break;
        case 0xF:
            if ((opcodeDATA & 0xF) == 0x7)
            {
                printf("Timer delay\n");
            }
            else if ((opcodeDATA & 0xF) == 0xA)
            {
                printf("Wait for key press\n");
            }
            else if ((opcodeDATA & 0xFF) == 0x15)
            {
                printf("Timer delay set\n");
            }
            else if ((opcodeDATA & 0xF) == 0x8)
            {
                printf("Sound timer\n");
            }
            else if ((opcodeDATA & 0xF) == 0xE)
            {
                printf("I = %x | V[%d] = %x\n", registerI, (opcodeDATA >> 8), registers[(opcodeDATA >> 8)]);
                registerI += (unsigned char)registers[(opcodeDATA >> 8)];
                printf("I = %x\n", registerI);
            }
            else if ((opcodeDATA & 0xF) == 0x9)
            {
                printf("Location of sprite\n");
            }
            else if ((opcodeDATA & 0xF) == 0x3)
            {
                ram[registerI + 2] = (unsigned char)registers[(opcodeDATA >> 8)] % 10;
                ram[registerI + 1] = ((unsigned char)registers[(opcodeDATA >> 8)] % 100) - ram[registerI + 2];
                ram[registerI] =  ((unsigned char)registers[(opcodeDATA >> 8)]) - ram[registerI + 1];
            }
            else if ((opcodeDATA & 0xFF) == 0x55)
            {
                for (i = 0; i <= (opcodeDATA >> 8); i++)
                {
                    ram[registerI + i] = registers[i]; 
                }
            }
            else if ((opcodeDATA & 0xFF) == 0x65)
            {
                printf("I = %x\n", registerI);
                for (i = 0; i <= (opcodeDATA >> 8); i++)
                {
                    registers[i] = ram[registerI + i];
                    printf("V[%d] = %x | ram[I + i] = %x\n", i, registers[i], ram[registerI + i]);
                }
            }
            break;
        default:
            printf("DEFAULT: ");
            break;        
    }
}
