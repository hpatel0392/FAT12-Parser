CC=gcc
flags= -Wall -g -o f12parse

project4: FAT12.h FAT12.c
	$(CC) $(flags) FAT12.c
