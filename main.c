#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
//#include "graphics.h"

#define CHUNK 10
#define STACK 32

void decode(int opcode);
int ldROM(char* path);
int fetch(char* ram2);
void end();

char registers[16] = { 0 };
int registerI = 0;
int registerF = 0;
char ram[4096];
int pc = 512;
int sp = STACK-1;

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
    
    for(i = 0; i <  (int)(fileSize / 2); i++)
    {
        opcode = fetch(ram+pc);
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
    char strOPCODE[2] = {0};
    
    memcpy(strOPCODE, ram2, 2);
    
    //change the order of the bytes in memory because of little endian.
    opcode += (unsigned char)(strOPCODE[0]);
    opcode = (opcode << 8) + (unsigned char)(strOPCODE[1]);

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
	int already_read = 512;

	int fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		printf("Cannot load the Rom file...\n");
		exit(0);
	}
	while((read_bytes = read(fd, ram + already_read, CHUNK)) > 0)
	{
		already_read += read_bytes;
	}	
	return (already_read-512);
}

/*
 * DECODE
 * Desc: Decode the opcode and execute the opcode.
 * Input: Opcode.
 */
void decode(int opcode)
{
    char tmpStr[5] = { 0 };
    int opcodeDATA = opcode & 0x0FFF;
    int i = 0;

    printf("DECODE: %04x \n", opcode);
    switch(opcode >> 12)
    {
        case 0:
            if (opcode == 0x00E0)
            {
                printf("CLS\n");
            }
            else if (opcode == 0x00EE)
            {   
                printf("0\n");
                if (sp == STACK-1)
                {
                    printf("Error opcode 00EE");
                    exit(0);
                }
                printf("1\n");
                //printf("SP -> %02x %02x",ram[sp], ram[sp+1]);
               // pc = fetch(ram+sp); 
               // sp += 2;
                //printf("PC AFTER RET: d: %d x: %04x", pc, pc);
               // printf("RET");
            }
            else
            {
                pc = opcodeDATA;
             //   printf("SYS addr (0)");
            }
            break;
        case 1:
            pc = opcodeDATA;
           // printf("JUMP addr (1): %04x\n", opcodeDATA);
            break;
        case 2:
           /* ntos(pc, tmpStr);
            sp -= 4;
            strncpy(ram+sp, tmpStr, 4);
            pc = opcodeDATA;*/
            break;
        case 3:
           // printf("3xkk\n");
            //printf("reg[opDt >> 8] = %x = opDt & 0xFF = %x | opDATA >> 8: %x", registers[opcodeDATA >> 8], (opcodeDATA & 0xFF), (opcodeDATA >> 8));
            if (registers[(opcodeDATA >> 8)] == (opcodeDATA & 0xFF))
            {
                pc += 2;
            }
            break;
        case 6:
           // printf("6xkk\n");
            //printf("opcodeDATA >> 8 = %x = opcodeDATA & 0xFF = %x\n", (opcodeDATA >> 8), (opcodeDATA & 0xFF));
            registers[(opcodeDATA >> 8)] = opcodeDATA & 0xFF;
            break;
        case 8:
           // printf("8xy6\n");
            //printf("reg[opDt >> 8] = %x\n", registers[(opcodeDATA >> 8)]);
            if ((opcodeDATA & 0xF) == 0x6)
            {
                if ((registers[(opcodeDATA >> 8)] & 0b1))
                {
                    registerF = 1;
                }
                else
                {
                    registerF = 0;
                }
                registers[(opcodeDATA >> 8)] = registers[(opcodeDATA >> 8)] >> 1; 

            }
            break;
        case 0xA:
            registerI = opcodeDATA;
            //printf("Register I = %04x", registerI);
            break;
        case 0xF:
            if ((opcode & 0xFF) == 0x65)
            {
               // printf("Fx65\n");
                for (i = 0; i <= (opcodeDATA >> 8); i++)
                {
                    registers[i] = ram[registerI + i];
                   // printf("registers[%d] = %02x\n", i, registers[i]);
                   // printf("registerI = %04x\n", registerI);
                   // printf("ram[RegI] = %02x\n", ram[registerI+i]);
                }
            }
            break;
        default:
            printf("Default...\n");
            printf("OPCODE & 0xF000: %01x | %d\n", opcode >> 12, opcode & 0xF000);
            break;        
    }
}
