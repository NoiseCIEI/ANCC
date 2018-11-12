/* 
 * The program merge_cor merges cross-corelation databases
 * into single database.
 * Usage: merge_cor path_list output_path
 */
#define MAIN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "mysac.h"
#include "trim.h"

#define NPTSMAX 40000
/* Finction prorotypes */

SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, int nmax);
void write_sac (char *fname, float *sig, SAC_HD *SHD);

SAC_HD shdin,shd_cor;

int main(int argc, char *argv[])
{
FILE *fin,*fout;
char *p,buf[255],path[255],cmd[255],list1[255],fname[255],fname1[255];
int i,icount=0,icount1=0,irev,n,num;
char dir[40],sta12[40],sta1[40],sta2[40];
char cdir[40],csta1[40],csta2[40],cwor[9];
float dd,ddh,wor,sig[NPTSMAX],sig1[NPTSMAX];
int bjday,bbjday,ejday,eejday;

  shd_cor=sac_null;
  if(argc !=3) {
    printf("Usage: merge_cor path_list output_path\n");
    exit(-1);
  }
  strcpy(list1,argv[1]);
  strcat(list1,"_1");
  if((fin=fopen(argv[1],"r")) == NULL) {
    printf("Can not open file %s\n",argv[1]);
    exit(-1);
  }
  if((fout=fopen("tmp.1","w")) == NULL) {
    printf("Can not open file %s\n","tmp.1");
    exit(-1);
  }
/* Create complete reference list */
  while((n = fscanf(fin,"%d %s",&num,buf)) == 2) {
    strcpy(path,buf);
    p = strrchr(buf,'/');
    *p = 0; p++;
    strcpy(sta12,p);
    p = strrchr(sta12,'.');
    *p = 0;
    p = strrchr(sta12,'_');
    *p = 0; p++;
    strcpy(sta2,p);
    p = strrchr(sta12,'_');
    *p = 0; p++;
    strcpy(sta1,p);
    p = strrchr(buf,'/');
    *p = 0; p++;
    strcpy(dir,p);
    irev = 1;
    if(strcmp(sta1,sta2) > 0) {
      irev = -1;
      fprintf(fout,"%s %s %d %d %s %s\n",sta2,sta1,num,irev,dir,path);
    } else {
      fprintf(fout,"%s %s %d %d %s %s\n",sta1,sta2,num,irev,dir,path);
    }
  }
  fclose(fin);
  fclose(fout);
/* sprintf(cmd,"/bin/sort +0.0 -1.0 +1.0 -2.0 +2.0n -3.0 tmp.1 > %s",list1); */
  sprintf(cmd,"/bin/sort -b -k 1,1 -k 2,2 -k 3,3n tmp.1 > %s",list1);
  system(cmd);
/* merge data     */
  strcpy(csta1," ");
  strcpy(csta2," ");
  strcpy(cdir," ");
  for(i=0; i < NPTSMAX; i++) sig[i]=0.0;
  dd = 0.0;
  ddh = 0.0;
  bjday = 2100*1000;
  ejday = 0;
  if((fin=fopen(list1,"r")) == NULL) {
    printf("Can not open file %s\n",list1);
    exit(-1);
  }
  while((n = fscanf(fin,"%s %s %d %d %s %s",sta1,sta2,&num,&irev,dir,path)) == 6){
    if((strcmp(sta1,csta1) !=0 || strcmp(sta2,csta2) !=0) && icount != 0) {
/* writesac      */
      sprintf(fname1,"%s/COR_%s_%s.SAC",fname,csta1,csta2);
printf("Wr_sac: %s\n",fname1);
      shd_cor.user0 = dd;
      shd_cor.user1 = sac_null.user1;
      shd_cor.user1 = ddh;
      sprintf(shd_cor.kt0,"%7d",bjday);
      shd_cor.kt0[7]=' ';
      sprintf(shd_cor.kt1,"%7d",ejday);
      shd_cor.kt1[7]=' ';
      write_sac(fname1,sig,&shd_cor);
      for(i=0; i < shd_cor.npts; i++) sig[i]=0.0;
      dd = 0.0;
      ddh = 0.0;
      bjday = 2100*1000;
      ejday = 0;
      icount1++;
    }
    if(strcmp(dir,cdir) != 0) {
      sprintf(fname,"%s/%s",argv[2],dir);
      mkdir(fname,0755);
printf("Dir: %s\n",fname);
    }
      if ( !read_sac(path,sig1,&shdin,NPTSMAX)) {
        printf("file %s not found\n",path);
        continue;
      }
/* find stacking interval  */
      sscanf(shdin.kt0,"%7d",&bbjday);
      sscanf(shdin.kt1,"%7d",&eejday);
      if(bbjday < bjday) bjday = bbjday;
      if(eejday > ejday) ejday = eejday;
      icount++;
  printf("Add: %s %s %d %d %s %s %f\n",sta1,sta2,num,irev,dir,path,shdin.delta);
      if(irev > 0) {
        for(i=0; i < shdin.npts; i++) sig[i] += sig1[i];
      } else {
        for(i=0; i < shdin.npts; i++) sig[i] += sig1[shdin.npts-i-1];
/* swap SAC header stations fields             */
        wor = shdin.stla;                    /* latitude  */
        shdin.stla = shdin.evla;
        shdin.evla = wor;
        wor = shdin.stlo;                    /* longitude */
        shdin.stlo = shdin.evlo;
        shdin.evlo = wor;
        strncpy(cwor,shdin.kstnm,8);         /* station name */
        strncpy(shdin.kstnm,&(shdin.kevnm[0]),8);
        strncpy(&(shdin.kevnm[0]),cwor,8);
        strncpy(cwor,shdin.knetwk,8);         /* network */
        strncpy(shdin.knetwk,shdin.kuser1,8);
        strncpy(shdin.kuser1,cwor,8);
      }
      dd = dd+shdin.user0;
      ddh = ddh+shdin.user1;
      shd_cor = shdin;
      strcpy(csta1,sta1);
      strcpy(csta2,sta2);
      strcpy(cdir,dir);
  }
  sprintf(fname1,"%s/COR_%s_%s.SAC",fname,csta1,csta2);
  shd_cor = shdin;
    if(dd != 0) {
      shd_cor.user0 = dd;
      shd_cor.user1 = ddh;
      sprintf(shd_cor.kt0,"%7d",bjday);
      shd_cor.kt0[7]=' ';
      sprintf(shd_cor.kt1,"%7d",ejday);
      shd_cor.kt1[7]=' ';
      write_sac(fname1,sig,&shd_cor);
printf("Wr_sac: %s\n",fname1);
      icount1++;
  }
  fclose(fin);
  printf("Cross-corr functions: %d, merged c-c: %d\n",icount,icount1);
  return 0;
}
/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
        SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, int nmax)
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
{
 FILE *fsac;
/*..........................................................................*/
        if((fsac=fopen(fname, "rb")) == NULL) return NULL;

        if ( !SHD ) SHD = &SAC_HEADER;

         fread(SHD,sizeof(SAC_HD),1,fsac);

         if ( SHD->npts > nmax ) {
           fprintf(stderr,
           "ATTENTION !!! in the file %s npts exceeds limit  %d",fname,nmax);
           SHD->npts = nmax;
         }

         fread(sig,sizeof(float),(int)(SHD->npts),fsac);

         fclose (fsac);

   /*-------------  calculate from t0  ----------------*/
   {
        int eh, em ,i;
        float fes;
        char koo[9];

        for ( i = 0; i < 8; i++ ) koo[i] = SHD->ko[i];
        koo[8] = '\0';

        SHD->o = SHD->b + SHD->nzhour*3600. + SHD->nzmin*60 +
         SHD->nzsec + SHD->nzmsec*.001;

        sscanf(koo,"%d%*[^0123456789]%d%*[^.0123456789]%g",&eh,&em,&fes);

        SHD->o  -= (eh*3600. + em*60. + fes);
   /*-------------------------------------------*/}

        return SHD;
}




/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
        void write_sac (char *fname, float *sig, SAC_HD *SHD)
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
{
 FILE *fsac;
 int i;
/*..........................................................................*/
        fsac = fopen(fname, "wb");

        if ( !SHD ) SHD = &SAC_HEADER;


        SHD->iftype = (int)ITIME;
        SHD->leven = (int)TRUE;

        SHD->lovrok = (int)TRUE;
        SHD->internal4 = 6L;



  /*+++++++++++++++++++++++++++++++++++++++++*/
     SHD->depmin = sig[0];
     SHD->depmax = sig[0];
 
   for ( i = 0; i < SHD->npts ; i++ ) {
    if ( SHD->depmin > sig[i] ) SHD->depmin = sig[i];
    if ( SHD->depmax < sig[i] ) SHD->depmax = sig[i];
   }

   fwrite(SHD,sizeof(SAC_HD),1,fsac);

   fwrite(sig,sizeof(float),(int)(SHD->npts),fsac);


   fclose (fsac);
}

