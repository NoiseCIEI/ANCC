/*
 * mkccwfdisc makes cross-correlation (CCs) from existing
 * cross-correlation database. Each CC is an ASCII flat file 
 * which describes the attributes about unique cross correlation.
 * The fields dir and dfile define relative path to the binary SAC
 * file containing cross-correlation records.
 * Usage: mkccwfdisc db_path db_name
 */ 
#define MAIN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <math.h>
#include "mysac.h"
#include "ccwfdisc.h"
#include "etime.h"
#include "trim.h"

#define NPTSMAX 40000

/* Finction prorotypes */

SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, int nmax);
float dist(float o_fi,float o_lam,float s_fi,float s_lam);

SAC_HD shd;
CCWFDISC ccwf;

int main(int argc,char *argv[])
{
DIR *pdir,*spdir;
struct dirent *ep,*sep;
char dir[255],sdir[255],file[255];
float sig[NPTSMAX];
int np2,count=0;
time_t t;
char swor[8];

  shd = sac_null;
  ccwf = ccwfdisc_null;
  time(&t);
  if(argc != 3) {
  printf(" Usage: mkccwfdisc db_path db_name\n");
  exit(-1);
  }
  sprintf(dir,"%s/%s",argv[1],argv[2]);
  if((pdir = opendir(dir)) == NULL) {
    printf("Direcrory %s does not exist\n",dir);
    exit(-1);
  }
  while ((ep = readdir(pdir))) {
    if(strcmp(ep->d_name,".") == 0) continue;
    if(strcmp(ep->d_name,"..") == 0) continue;
/*    puts (ep->d_name);                         */
      sprintf(sdir,"%s/%s",dir,ep->d_name);
      if((spdir = opendir(sdir)) == NULL) {
        printf("Direcrory %s does not exist\n",sdir);
        exit(-1);
      }
      while ((sep = readdir(spdir))) {
        if(strcmp(sep->d_name,".") == 0) continue;
        if(strcmp(sep->d_name,"..") == 0) continue;
/*        puts (sep->d_name);                    */
        sprintf(file,"%s/%s",sdir,sep->d_name);
/*        puts(file);                            */
        if ( !read_sac(file,sig,&shd,NPTSMAX)) {
          printf("File %s not found\n",file);
          exit(-1);
        }
/*=========== fill out descriptor table ===================== */
        ccwf = ccwfdisc_null;
        BFIL(ccwf.sta1,6);
        strncpy(ccwf.sta1,shd.kevnm,6);
        TRIM(ccwf.sta1,6);
        BFIL(ccwf.sta2,6);
        strncpy(ccwf.sta2,shd.kstnm,6);
        TRIM(ccwf.sta2,6)
        strncpy(ccwf.net1,shd.knetwk,8);
        TRIM(ccwf.net1,8)
        strncpy(ccwf.net2,shd.kuser1,8);
        TRIM(ccwf.net2,8)
        strncpy(ccwf.chan1,shd.kcmpnm,8);
        TRIM(ccwf.chan1,8)
        strcpy(ccwf.chan2,ccwf.chan1);
/*  time                                    */
        np2 = shd.npts/2;
        ccwf.time =  -shd.delta*np2;
        count++;
        ccwf.wfid = count; 
/*        ccwf.jdate = 2000001;             */
/* endtime                                  */
        ccwf.endtime = shd.delta*np2;
        ccwf.nsamp = shd.npts;
        ccwf.samprate = 1.0/shd.delta;
        strncpy(swor,shd.kt0,8);
        TRIM(swor,8);
        ccwf.sdate=atoi(swor);
        strncpy(swor,shd.kt1,8);
        TRIM(swor,8);
        ccwf.edate = atoi(swor);
        ccwf.stdays = shd.user0+0.5;
        ccwf.tsnorm = shd.user1;
        ccwf.range = shd.dist;
        strcpy(ccwf.datatype,"f4");
        sprintf(ccwf.dir,"%s/%s",argv[2],ep->d_name);
        strcpy(ccwf.dfile,sep->d_name);
        ccwf.foff = 632;
        sprintf(ccwf.lddate,"%11.5lf",(double)t);
        printf(CCWFDISC_WCS,CWFDISC_WVL(&ccwf));
/*=========================================================== */
      }
      closedir(spdir);
  }
  closedir(pdir);
  return 0;
}
/*--------------------------------------------------------------------------*/
        SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, int nmax)
/*--------------------------------------------------------------------------*/
{
 FILE *fsac;
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
/* -------------------------------------------------------- */
float dist(float o_fi,float o_lam,float s_fi,float s_lam)
{
double x1[3],x2[3],delta,drad;
double geo=0.993277;
double fi,fa;

/* From geographical to geocentrical */
  drad=atan(1.0)/45.0;
  fi = atan(geo*tan(drad*o_fi))/drad;
  fa = atan(geo*tan(drad*s_fi))/drad;
  x1[0]=sin((90.0-fi)*drad)*cos(o_lam*drad);
  x1[1]=sin((90.0-fi)*drad)*sin(o_lam*drad);
  x1[2]=cos((90.0-fi)*drad);
  x2[0]=sin((90.0-fa)*drad)*cos(s_lam*drad);
  x2[1]=sin((90.0-fa)*drad)*sin(s_lam*drad);
  x2[2]=cos((90.0-fa)*drad);
/* Evaluating delta             */
  delta=acos(x1[0]*x2[0]+x1[1]*x2[1]+x1[2]*x2[2])*6371.0;
  return (float)delta;
}
