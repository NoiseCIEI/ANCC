#define MAIN

#include <stdio.h>
#include "mysac.h"
#include <unistd.h>
#include "sac_db.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Finction prorotypes */
int jday ( int y, int m, int d );
SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, int nmax);
void write_sac (char *fname, float *sig, SAC_HD *SHD);
int check_info ( SAC_DB *sdb, int ne, int ns1, int ns2 );
int do_cor( SAC_DB *sdb, int lag , FILE *cv);
void dcommon_(int *len, float *amp,float *phase);
void dmultifft_(int *len,float *amp2,float *phase2,float *amp,float *phase, int *lag,float *seis_out, int *ns);

SAC_DB sdb;
int bjday=0,ejday=0,wjday=0,nnpts=0;
float ddt,sig[100000];
BUF Buf[NEVENTS];
SAC_HD shdamp1, shdph1, shdamp2, shdph2, shd_cor;

/*c/////////////////////////////////////////////////////////////////////////*/
/* =======================================================================
  justCOR_mv_dir program convert spectra to cross-correlations database
  Usage: justCOR_mv_dir lag nnpts
  where.
  lag    - cross-collelation lag
  nnpts  - number of samples in day  
   ======================================================================= */
int main (int na, char *arg[])
/*--------------------------------------------------------------------------*/
{
  FILE *ff, *cv;
  int  lag;

  shd_cor=sac_null;
  cv=fopen("COR_RECORD","w");
  if ( na < 3 )
    {
      fprintf(stderr,"Usage: justCOR_mv_dir lag nnpts\n");
      exit(1);
    }

  lag = atoi(arg[1]);
  nnpts = atoi(arg[2]);

  ff = fopen("sac_db.out","rb");
  fread(&sdb, sizeof(SAC_DB), 1, ff );
  fclose(ff);

fprintf(stderr, "read info fine\n");

/*   do all the work of correlations here */
  fprintf (stderr, "%d %d\n", sdb.nst,sdb.nev);
  do_cor(&sdb,lag,cv);  


  fprintf(stderr, "finished correlations\n");
  fclose(cv);

  return 0;
}
/*------------------------------------------------------------------ */
 int jday ( int y, int m, int d )
/*------------------------------------------------------------------------*/
{
  int jd = 0;
  int i;
 
  for ( i = 1; i < m; i++ )
    {
      if ( (i==1) || (i==3) || (i==5) || (i==7) || (i==8) || (i==10) ) jd += 31;
      else if (i==2)
        {
          if ( y == 4*(y/4) ) jd += 29;
          else jd += 28;
        }
      else jd += 30;
    }
 
  return jd + d;
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

/*************************************************************************/
 int check_info ( SAC_DB *sdb, int ne, int ns1, int ns2 )
/*************************************************************************/
{
  if ( ne >= sdb->nev ) {
    fprintf(stderr,"cannot make correlation: too large event number\n");
    return 0;
  }
  if ( (ns1>=sdb->nst) ||(ns2>=sdb->nst)  ) {
    fprintf(stderr,"cannot make correlation: too large station number\n");
    return 0;
  }
  if ( sdb->rec[ne][ns1].n <= 0 ) {
    fprintf(stderr,"no data for station %s and event %s\n", sdb->st[ns1].n_name, sdb->ev[ne].name );
    return 0;
  }
  if ( sdb->rec[ne][ns2].n <= 0 ) {
    fprintf(stderr,"no data for station %s and event %s\n", sdb->st[ns2].n_name, sdb->ev[ne].name );
    return 0;
  }
  if ( fabs(sdb->rec[ne][ns1].dt-sdb->rec[ne][ns2].dt) > .0001 ) {
    fprintf(stderr,"incompatible DT %f ,%f for day %d sta1: %s sta2: %s\n",
           sdb->rec[ne][ns1].dt,sdb->rec[ne][ns2].dt,ne+1,
           sdb->st[ns1].n_name,sdb->st[ns2].n_name);
    return 0;
  }
    return 1;
	
}

/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
 int do_cor( SAC_DB *sdb, int lag , FILE *cv)
/*--------------------------------------------------------------------------*/
{
  int ine, jsta1, jsta2, k, count;
  int len,ns,i,nstack,ix,ll; 


  char filename[200], amp_sac[200], phase_sac[200],str[300];
  float amp[400000], phase[400000], cor[40000];
  float seis_out[400000];
  char loc2[10];

  fprintf(stderr,"sdb->nev %d sdb->nst %d\n",sdb->nev,sdb->nst);
     
  for( jsta1 = 0; jsta1 < sdb->nst; jsta1++ ) {
/*     Buffer initialization                    */
    count = 0;
    for(i=0; i < sdb->nev; i++) Buf[i].n = 0;
    for( ine = 0; ine < sdb->nev; ine++ ) {
      sprintf( amp_sac, "%s.am", sdb->rec[ine][jsta1].ft_fname );
      sprintf( phase_sac, "%s.ph", sdb->rec[ine][jsta1].ft_fname );
      if ( read_sac(amp_sac,&Buf[ine].am[0], &shdamp1, 1000000 )==NULL ) {
        fprintf( stderr,"file %s did not found\n", amp_sac );
        continue;
      }
      if ( read_sac(phase_sac, &Buf[ine].ph[0], &shdph1, 1000000)== NULL ) {
        fprintf( stderr,"file %s did not found\n", phase_sac );
        continue;
      }
      Buf[ine].n = shdamp1.npts;
      count++;
    }
    if( count == 0) continue;
    sprintf(str,"mkdir COR/%s", sdb->st[jsta1].n_name);
    system(str);
    for( jsta2 = (jsta1+1); jsta2 < sdb->nst; jsta2++ ) {

      nstack=0;
      for(i=0; i < (2*lag+1); i++) sig[i]=0.0;

/*-------------------- loop by events -------------------------------*/
      bjday = 2100*1000;
      ejday = 0;
      for( ine = 0; ine < sdb->nev; ine++ )       {
        if (fmod(ine,5)==0)
        fprintf(stderr,"sdb->nev %d\n",ine);
        if(Buf[ine].n > 0){ /*(1) */
          if(sdb->rec[ine][jsta2].n > 0){ /* (2) */
            len = Buf[ine].n;
            sprintf(amp_sac, "%s.am", sdb->rec[ine][jsta2].ft_fname);
            sprintf(phase_sac, "%s.ph", sdb->rec[ine][jsta2].ft_fname);
            fprintf(stderr,"file %s  %s\n",
                 sdb->rec[ine][jsta1].ft_fname,sdb->rec[ine][jsta2].ft_fname );
            if ( read_sac(amp_sac, amp, &shdamp2, 100000) ==NULL ) {
              fprintf(stderr,"file %s did not found\n", amp_sac );
              goto loop1;
              }
            if ( read_sac(phase_sac, phase, &shdph2, 100000)==NULL ) {
              fprintf(stderr,"file %s did not found\n", phase_sac );
              goto loop1;
              }
            len = shdamp2.npts;
            if(!check_info(sdb, ine, jsta1, jsta2 )) { /* (3) */
              fprintf(stderr,"files incompatible\n");
/* MB              return 0;   steps do not coinside   */
              goto loop1;
              }
            else
              {
              strncpy(loc2,shdph2.khole,8);
              dmultifft_(&len, &Buf[ine].am[0],&Buf[ine].ph[0],amp,phase,&lag,seis_out,&ns);
              cor[lag] = seis_out[0];
              for( i = 1; i< (lag+1); i++)
                { 
                cor[lag-i] =  seis_out[i];
                cor[lag+i] =  seis_out[ns-i];
                }
/* add new correlation to previous one                         */
 	        for(k = 0; k < (2*lag+1); k++) sig[k] += cor[k];
                wjday = sdb->ev[ine].yy*1000+sdb->ev[ine].jday;
                if(wjday < bjday) bjday = wjday;
                if(wjday > ejday) ejday = wjday; 
                nstack +=1;  
                fprintf(cv,"COR_%s_%s: %d %d %d %d\n",
                      sdb->st[jsta1].n_name,sdb->st[jsta2].n_name,
                      sdb->ev[ine].yy,sdb->ev[ine].mm,sdb->ev[ine].dd,
                      jday(sdb->ev[ine].yy,sdb->ev[ine].mm,sdb->ev[ine].dd));
                shd_cor = shdamp1;
                ddt = sdb->rec[ine][jsta1].dt;
 	      }   /*loop over check (else) (3) */
            }  /* if jsta2 (2) */
          }  /* if jsta1 (1) */
        loop1:
        ;
        if (ine == sdb->nev-1 && nstack > 0) {   /* last day */
/*  create cor file accordingly                                */
          sprintf(filename, "COR/%s/COR_%s_%s.SAC",
                 sdb->st[jsta1].n_name,sdb->st[jsta1].n_name, sdb->st[jsta2].n_name);
          shd_cor.delta = ddt;
          shd_cor.evla =  sdb->st[jsta1].lat;
          shd_cor.evlo =  sdb->st[jsta1].lon;
          shd_cor.stla =  sdb->st[jsta2].lat;
          shd_cor.stlo =  sdb->st[jsta2].lon;
          strncpy(shd_cor.kevnm,shd_cor.kstnm,8);
          strncpy(&(shd_cor.kevnm[8]),"        ",8);
          ll = strlen(sdb->st[jsta2].name);
          strcpy(shd_cor.kstnm,sdb->st[jsta2].name);
          for(ix=ll; ix < 8; ix++) shd_cor.kstnm[ix] = ' ';
          strncpy(shd_cor.kuser1,sdb->st[jsta2].n_name,8);
          shd_cor.kuser1[7] = 0;
          ll = 0;
          for(ix = 0; ix < 8; ix++) {
            if(shd_cor.kuser1[ix] == '.') ll = 1;
            if(ll == 1) shd_cor.kuser1[ix] = ' ';
          }
/*          strncpy(shd_cor.kuser2,loc2,8);   */
          strncpy(shd_cor.kuser2,shd_cor.kcmpnm,8);
          shd_cor.npts =  2*lag+1;
          shd_cor.b    = -lag*shd_cor.delta;
          shd_cor.e    = lag*shd_cor.delta;
          shd_cor.user0 = nstack;
          sprintf(shd_cor.kt0,"%7d",bjday);
          shd_cor.kt0[7]=' ';
          sprintf(shd_cor.kt1,"%7d",ejday);
          shd_cor.kt1[7]=' ';
          shd_cor.user1 = 24.0*nstack*86400.0/nnpts;
          write_sac (filename, sig, &shd_cor);
          }         
        } /* loop over event */
     }  /* loop over jst2 */
  }  /* loop over jsta1 */
  return 0;
}
