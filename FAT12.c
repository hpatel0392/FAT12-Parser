/*
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FAT12.h"
file_list_t *flist, *tail;
int * fat;
char* dir;

int main(int argc, char* argv[]){
   if(argc != 3)
      exit(-1);

   FILE * fp = fopen(argv[1], "rb");
   if(fp == NULL) exit(-1);
   dir = argv[2];  

   goTo(fp, FAT1);
   fat = decodeFAT(fp);
     
   goTo(fp, ROOT);
   flist = (file_list_t*)malloc(sizeof(file_list_t));
   flist->info = NULL;
   tail = flist;
   
   while(parseDirectory(fp, "/", FALSE) == FALSE);
   output();
   
   free(flist);
   free(fat);
   fclose(fp);
   return 0; 
}

int * decodeFAT(FILE * fp){
   int * fat = (int*)malloc(FAT_ENTRIES * sizeof(int));
   if(fat == NULL) exit(-1);
   
   char frame[3]; // read 3 bytes at a time and decode
   int i, f1, f2;
   for(i = 0; i < FAT_ENTRIES; i+=2){
      f1 = 0;
      f2 = 0;
      fread(&frame, 1, 3, fp);
      f1 |= frame[0];
      f1 &= 0x0FF;
      f1 |= (frame[1] & 0x0F)<<8;
      f2 = frame[2]<<4;
      f2 |= (frame[1] & 0xF0)>>4;
      f2 &= 0xFFF;
      fat[i] = f1;
      fat[i+1] = f2;
   }

   return fat;
}

void goTo(FILE * fp, int sector){
   rewind(fp);
   fseek(fp, sector*SECTOR_SIZE, SEEK_SET);
}


int parseDirectory(FILE * fp, char * path, int deleted){
   
   int i;
   if(tail->info == NULL){
      tail->info = (file_info_t*)malloc(sizeof(file_info_t));
   }
   
   file_info_t *fi = tail->info;
   fi->deleted = FALSE;
 
   long prevLoc;
   dir_ent_t curr;

   if(strlen(path) != 1){
      fread(&curr, sizeof(dir_ent_t), 1, fp);
      fread(&curr, sizeof(dir_ent_t), 1, fp);
   }

   fread(&curr, sizeof(dir_ent_t), 1, fp);
   if(curr.fname[0] == 0) return TRUE;

   if(curr.fname[0] == (char)0xE5){
      curr.fname[0] = '_';
      fi->deleted = TRUE;
   }

   for(i = 0; i < 8; i++){
      if(curr.fname[i] == ' '){
         curr.fname[i] = '\0';
         break;
      }
   }

   int filesToFind = SECTOR_SIZE/sizeof(dir_ent_t);
   if(strlen(path) != 1){
      filesToFind -= 2;
   }
   int filesFound = 0;
   int newTailMade = FALSE;
   while( (filesFound != filesToFind) && (curr.fname[0] != 0) ){
         
      if(curr.atr & 0x10 && !(curr.atr & 0xF)){
         char newPath[strlen(path)+strlen(curr.fname)+3];
         newPath[0] = '\0';
         //strncpy(newPath, path, );
         strcat(newPath, path);
         strcat(newPath, curr.fname);   
         strcat(newPath, "/");
         int sector, nextCluster = curr.firstCluster;
         sector = DATA + nextCluster - START_CLUSTER;
         long temp;
         int testCluster;
         long prevDir = ftell(fp);
         do{
            goTo(fp, sector);

            temp = ftell(fp);
            fread(&curr, sizeof(dir_ent_t), 1, fp);
            testCluster = curr.firstCluster;
            fseek(fp, temp, SEEK_SET);

            temp = ftell(fp);
            parseDirectory(fp, newPath, deleted);
            fseek(fp, temp, SEEK_SET);
            fread(&curr, sizeof(dir_ent_t), 1, fp);
            fread(&curr, sizeof(dir_ent_t), 1, fp);           

            if(testCluster == nextCluster){
               break;
            }else{
               nextCluster = fat[testCluster];
               sector = DATA + nextCluster - START_CLUSTER;
            }
         }while(nextCluster != FAT_EOF);
         fseek(fp, prevDir, SEEK_SET);
         newTailMade = TRUE;    
      } else {
         
         strcat(fi->filepath, path);
         strncat(fi->filepath, curr.fname, 8);
         strcat(fi->filepath, ".");
         strncat(fi->filepath, curr.ext, 3);
         fi->size = curr.fileSize;
         fi->data = malloc(fi->size);      
       
         prevLoc = ftell(fp);
         int sector = DATA + curr.firstCluster - START_CLUSTER;
         goTo(fp, sector);
         getData(&fi->data, fp, &fi->size, curr.firstCluster, fi->deleted);
         fseek(fp, prevLoc, SEEK_SET);
       
      } // end else    

         if(filesFound == filesToFind-1) break;
         fread(&curr, sizeof(dir_ent_t), 1, fp);   

         if(newTailMade == FALSE){
            tail->next = (file_list_t*)malloc(sizeof(file_list_t));
            tail = tail->next;
            tail->info = (file_info_t*)malloc(sizeof(file_info_t));
         }
         fi = tail->info;
         fi->deleted = FALSE;
         
         if(curr.fname[0] == (char)0xE5){
            curr.fname[0] = '_';
            fi->deleted = TRUE;
         }

         for(i = 0; i < 8; i++){
            if(curr.fname[i] == ' '){
               curr.fname[i] = '\0';
               break;
            }
         }
         newTailMade = FALSE;          
   } // end while
   
   if(curr.fname[0] == 0)
      return TRUE;
   return FALSE;
}



void getData(void **dest, FILE *fp, int *size, int firstCluster, int deleted){
      
   int sizeLeft = *size;
   void* temp = *dest;
   int sector, nextCluster = firstCluster;

   if(deleted == FALSE){
      while(fat[nextCluster] != FAT_EOF){
         sector = DATA + nextCluster - START_CLUSTER;
         goTo(fp, sector);
         if(sizeLeft < SECTOR_SIZE){
            break;
         }
         fread(temp, 1, SECTOR_SIZE, fp);
         temp += SECTOR_SIZE;
         sizeLeft -= SECTOR_SIZE;
            
         nextCluster = fat[nextCluster];       
      } 
      fread(temp, 1, sizeLeft, fp);
   } else {
      while(fat[nextCluster] == FAT_FREE){
         sector = DATA + nextCluster - START_CLUSTER;
         goTo(fp, sector);
         if(sizeLeft < SECTOR_SIZE){
            fread(temp, 1, sizeLeft, fp);
            sizeLeft = 0;
            break;
         }
         fread(temp, 1, SECTOR_SIZE, fp);
         temp += SECTOR_SIZE;
         sizeLeft -= SECTOR_SIZE;
            
         nextCluster++;
      }
      //*size -= sizeLeft;
   }      
}


void output(){

   int i = 0;
   file_list_t *it = flist;
   file_info_t *curr;
   char ext[5];
   char filepath[128];
   char * temp;   
   FILE * out; 

   while(it != tail){
      curr = it->info;
      printf("FILE\t");
      if(curr->deleted == TRUE)
         printf("DELETED\t");
      else
        printf("NORMAL\t");
      printf("%s\t", curr->filepath);
      printf("%d\n", curr->size);

      temp = curr->filepath;
      temp += strlen(temp);
      temp -= 4;
      strcpy(ext, temp);
      
      sprintf(filepath, "%s/file%d%s", dir, i, ext);
      out = fopen(filepath, "wb+");
      if(out == NULL) exit(-1);
           
      fwrite(curr->data, 1, curr->size, out);
      fclose(out);
      free(curr->data);
      free(curr);
      flist = flist->next;
      free(it);
      it = flist;
      i++;
   }
}
