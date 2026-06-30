#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define CHUNK 10

void instructions(int instruct);
int ldRom(char* path);

char ram[4096];

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("Usage: ./emulator [Rom name]\n");
		exit(0);
	}
	int fileSize = ldRom(argv[1]);
	for(int i = 0; i < 20; i++)
	{
		printf("%2x  ", ram[200+i] & 0xFF);
	}
	printf("Hello world\n");
	return 0;
}

int ldRom(char* path)
{
	int read_bytes = 0;
	int already_read = 200;

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
	return (already_read-200);
}

void instructions(int instruct)
{
	printf("Set");
}
