/* -------------------------------------------------------------- */
/*                                                                */
/* Compile: gcc mainMt.c TGenMT.c -o mt                           */
/* Run:  ./mt                                                     */
/*                                                                */
/* Updates by DAO Van Toan - May 2016                             */
/* -------------------------------------------------------------- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "TGenMT.h"

//#define MAXREP 16 //00000000
/* ----------------------------------------------------------- */
/* removeData         Remove the text files of MT statut       */
/* ----------------------------------------------------------- */
void removeData()
{
   char command[50];
   sprintf(command, "rm -f data/inStatus_*");
   system(command);
}

/* -------------------------------------------------------------- */
int main(int argc, char **argv)
{
    int i,j, nbStatus;
    unsigned long init[4]={0x123, 0x234, 0x345, 0x456}, length=4;

    if(argc < 1 || argc > 2)
    {
       printf("Number of args invalid: ./genMT nbStatus...\n");
       return -1;
    }

    nbStatus = atoi(argv[1]);

    init_by_array(init, length);

    char command[50];
    sprintf(command, "mkdir -p data");
    system(command);

    for(j = 0; j < nbStatus; j++)
    {
       char nameFile[50];
       sprintf(nameFile,"data/inStatus_%04d",j);

       saveStatus(nameFile);

       for(i = 0; i < 1000000000; i++)
       {
          mtRand();
       }
    }

    return 0;
}
/* -------------------------------------------------------------- */
