CC=gcc
flags= -Wall -g -o notjustcats

project4: FAT12.h FAT12.c
	$(CC) $(flags) FAT12.c

tar:
	tar cvzf project4.tgz FAT12.c FAT12.h Makefile README.txt


