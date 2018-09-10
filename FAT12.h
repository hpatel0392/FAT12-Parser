/*
*
*  Holds constants and structure information for FAT12 files
*/

#include <stdlib.h>
#include <stdio.h>


// constants
#define SECTOR_SIZE 512

#define FAT1 1
#define FAT2 10
#define ROOT 19
#define DATA 33

#define START_CLUSTER 2

#define FAT_ENTRIES 3072
#define DIREC_ENTRY 32

#define DEL_FILE (char)0xEF
#define SPACE (char)0x20
#define FAT_EOF 0xFFF
#define FAT_FREE 0x000

#define TRUE 1
#define FALSE 0

typedef struct file_info{
   char deleted;
   char filepath[64];
   int size;
   void * data;
} file_info_t;

typedef struct file_list{
   struct file_list *next;
   file_info_t *info;
} file_list_t;

typedef struct dir_ent{
   char fname[8];
   char ext[3];
   char atr;
   short res;
   short time_created;
   short date_created;
   short date_lastAccess;
   short unused;
   short time_lastWrite;
   short date_lastWrite;
   short firstCluster;
   int fileSize;
} dir_ent_t;

int* decodeFAT(FILE * fp);
void goTo(FILE * fp, int sector);
int parseDirectory(FILE * fp, char * path, int deleted);
void getData(void **dest, FILE * fp, int *size, int firstCluster, int deleted);
void output(void);


