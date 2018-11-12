#include <stdio.h>
#include <stdlib.h>
#include "ccwfdisc.h"
int main(int argc, char *argv[])
{
FILE *fin, *fout;
CCWFDISC ccwf=ccwfdisc_null;
int icount=0,n;
  if(argc != 3) {
    printf("Usage: add_num in_file out_file\n");
    exit(-1);
  }
  if((fin = fopen(argv[1],"r")) == NULL) {
     printf("Could no open file %s\n",argv[1]);
  }
  if((fout = fopen(argv[2],"w")) == NULL) {
     printf("Could no open file %s\n",argv[2]);
  }
  while((n = fscanf(fin,CCWFDISC_RCS,WFDISC_RVL(&ccwf))) == 23) {
    icount++;
    ccwf.wfid = icount;
    fprintf(fout,CCWFDISC_WCS,CWFDISC_WVL(&ccwf));
  }
  fclose(fin);
  fclose(fout);
  return 0;
}
