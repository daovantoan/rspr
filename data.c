/* ------------------------------------------------------------ */
/*   data.c: To manager datas, files and folders                */
/*   Input : None                                               */
/* ------------------------------------------------------------ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "TGenMT.h"

/* ------------------------------------------------------------ */
/*   check_folder: Check existence status of a folder           */
/*   Input : path or name of folder                             */
/*   Output: an integer is representing for a status            */
/* ------------------------------------------------------------ */
int check_folder(char * folder)
{
   int             check;
   DIR           * dir;
   struct dirent * readir;

   dir = opendir(folder);

   if(dir)
   {
      /*Folder exists*/
      check = 1;
      closedir(dir);
   }else if(errno == ENOTDIR){
      /*Need for creating of  a new folder*/
      check = 0;
   }else{
      perror ("data.c - e000: Can't open folder with opendir()\n");
   }

   return check;
}

/* ------------------------------------------------------------ */
/*   create_folder: Create a new folder                         */
/*   Input : path or name of folder                             */
/*   Output: a folder created with a pricise path               */
/* ------------------------------------------------------------ */
void create_folder(char * pathFolder)
{
   char command[256];

   sprintf(command,"mkdir -p %s", pathFolder);
   if(system(command) == -1)
      perror("data.c - e001: Can't creat a new folder");
}

/* ------------------------------------------------------------ */
/*   count_file: Count number of files in a folder              */
/*   Input : path or name of folder                             */
/*   Output: number of files                                    */
/* ------------------------------------------------------------ */
int count_file( char * path)
{
  int             count = 0;
  DIR           * dir;
  struct dirent * readir;

  dir = opendir(path);

  if(dir)
  {
    while((readir = readdir (dir)) != NULL)
    {
       /*Don't count parent and child's folder*/
       if((strcmp(readir->d_name, ".")) != 0 
            && 
          strcmp(readir->d_name, "..") != 0)
         count++;
    }

    closedir (dir);
  }
  else
    perror ("data.c - e002: Can't open this folder");

  return count;
}

/* ------------------------------------------------------------ */
/*   remove_file: Delete all of files in a folder               */
/*   Input : path or name of folder                             */
/*   Output: None                                               */
/* ------------------------------------------------------------ */
void remove_file( char * path)
{
  DIR           * dir;
  char            buf[256];
  struct dirent * readir;

  dir = opendir(path);
  
  if(dir)
  {
    while((readir = readdir (dir)) != NULL)
    {
      if(strcmp(readir->d_name, ".") != 0 
            && 
         strcmp(readir->d_name, "..") != 0)
      {  
         sprintf(buf, "%s/%s", path, readir->d_name);
         remove(buf);
      }
    }
    closedir (dir);
  }
  else
    perror ("data.c - e003: Can't open folder to delete files");
}

/* ------------------------------------------------------------ */
/*   genMT_new: Generate a set of initial status of PRNG        */
/*   Input : Number of replications                             */
/*   Output: None                                               */
/* ------------------------------------------------------------ */
void genMT_new(int nbRep){
  int i,j;
  unsigned long init[4]={0x123, 0x234, 0x345, 0x456}, length=4;

  init_by_array(init, length);

  for(j = 0; j < nbRep; j++)
  {
     char nameFile[50];
     sprintf(nameFile,"data/inStatus_%06d",j);

     saveStatus(nameFile);

     for(i = 0; i < 1000000000; i++)
     {
        mtRand();
     }
  }
}

/* ------------------------------------------------------------ */
/*   genMT_res: Add the missing initial status                  */
/*   Input : Current number of initial status and               */
/*           number of replication                              */
/*   Output: None                                               */ 
/* ------------------------------------------------------------ */
void genMT_res(int nbReal, int nbRep){
  int i, t, j, kk = nbReal - 1;
  unsigned long init[4]={0x123, 0x234, 0x345, 0x456}, length=4;

  init_by_array(init, length);

  char nameFile[50];
  sprintf(nameFile, "data/inStatus_%06d", kk);
  restoreStatus(nameFile);

  for(t = 0; t < 1000000000; t++)
  {
        mtRand();
  }

  for(j = nbReal; j < nbRep; j++)
  {
     char nameFile[50];
     sprintf(nameFile,"data/inStatus_%06d",j);

     saveStatus(nameFile);

     for(i = 0; i < 1000000000; i++)
     {
        mtRand();
     }
  }
}

/* ------------------------------------------------------------ */
/*   search_file: Find a file in a folder                       */
/*   Input : Name of file                                       */
/*   Output: Status of file                                     */
/* ------------------------------------------------------------ */
int file_exist(const char *nameFile)
{
    struct stat buf;
    int checkValue = 0;

    if (stat(nameFile, &buf) == 0) 
      checkValue = 1;
    
   return checkValue;
}

/* ------------------------------------------------------------ */
/*   compare_immediate: Compare two files (immediate results)   */
/*   Input : two files                                          */
/*   Output: Status of comparaison                              */
/* ------------------------------------------------------------ */
int compare_intermediate(char * nameFile1, char * nameFile2)
{
   FILE * fp1, * fp2;
   int    char1, char2, status = 0;

   fp1 = fopen(nameFile1,"r");
   fp2 = fopen(nameFile2,"r");

   /*Cannot open file to compare*/
   if(fp1 == NULL || fp2 == NULL) {
      status = -1;
   }

   char1 = getc(fp1);
   char2 = getc(fp2);

   while((char1 != EOF) && (char2 != EOF) && (char1 == char2))
   {
      char1 = getc(fp1);
      char2 = getc(fp2);
   }

   if(char1 == char2) status = 1;

   fclose(fp1);
   fclose(fp2);

   return status;

}
