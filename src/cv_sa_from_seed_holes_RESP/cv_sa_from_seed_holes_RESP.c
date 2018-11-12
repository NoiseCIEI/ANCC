/*======================================================================
Name: cv_sa_from_seed_holes_RESP
      read and convert daily SEED volumes to SAC format.
      Create auxilary table including enent/station attributes
      and referencies to SAC binary filesi on a disk. 
      Saved auxilary table on a disk to pass it other programs.
Usage: cv_sa_from_seed_holes_RESP LHZ mholes
       where
       LHZ - chanel name of waveforms;
       mholes - maximum allowed data gap per day (in %)
========================================================================*/
#define MAIN

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "mysac.h"
#include "sac_db.h"
#include "etime.h"

/*   Function prototypes                                                  */
SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, int nmax);
void write_sac (char *fname, float *sig, SAC_HD *SHD);
void sac_db_write_to_asc ( SAC_DB *sdb, char *fname );
int merge_sac(char *sta, char *chan, double *t0, float *dt, int *nrec);
void mk_one_rec (SAC_DB *sdb, int ne, int ns, char *nseed, char *ch);
void  fill_one_sta (STATION *st, char *buff );
void fill_one_event (EVENT *ev, char *buff );
int change_sac_file(char *buff1, double *frac,int *nfrac);

#define NPTSMAX 1000000
float sig0[NPTSMAX], sig1[NPTSMAX];
SAC_DB sdb;
char str[1000];
char buff[300];
char fname[10000][3000];
double t1[10000], t2[10000], T1, T2;
SAC_HD sd[10000], s0;
int nf;
float pholes;

/*////////////////////////////////////////////////////////////////////////*/
/*------------------------------------------------------------------------*/
 int main (int argc, char *argv[] )
/*------------------------------------------------------------------------*/
{
  int ist, iev;
  FILE *ff;
  char channel[3];
  if ( argc < 3 ) {
    fprintf(stderr, "Usage: cv_sa_from_seed_holes_RESP LHZ mholes\n");
    return 1;
  }
  sscanf(argv[1],"%s",channel);             /* channel      */
  pholes = (float)atof(argv[2]);            /* gap fraction */
  for ( iev = 0; iev < NEVENTS; iev++ ) 
    for ( ist = 0; ist < NSTATION; ist++ ) sdb.rec[iev][ist].n = 0;

  fprintf(stderr,"initializing DB ok\n");


  ff = fopen("station.lst","r");

  for ( ist = 0; ; ist++ )
    {
      if ( !fgets(buff,300,ff) ) break;

/*      puts(buff); */

      fill_one_sta (&(sdb.st[ist]), buff );

      fprintf(stderr,"filling station %s %f %f\n", sdb.st[ist].n_name,  sdb.st[ist].lon, sdb.st[ist].lat );
    }

  sdb.nst = ist;

  fclose(ff);

  fflush(NULL);

  /*fprintf(stderr,"stations filled\n");
    scanf("%*1d");*/



  ff = fopen("input_ev_seed","r");

  for ( iev = 0;; )
    {
      if ( !fgets(buff,300,ff) ) break;

      if ( !strncmp(" PDE", buff, 4 ) )
	{
	  fill_one_event (&(sdb.ev[iev]), buff );

	  iev++;
	}

      else
	{
          if(access("rdtmp", F_OK) == 0) {
            sprintf(str,"/bin/rm -f rdtmp/* ");
            system(str);
          } else {
            sprintf(str,"/bin/mkdir rdtmp");
            system(str);
          }
          sprintf(str,"cd rdtmp; rdseed -R -f ../%s",buff);
          system(str);
          sprintf(str,"cd rdtmp; rdseed -o 1 -d -f ../%s",buff);
          system(str);
	  for ( ist = 0; ist < sdb.nst; ist++ )
	    {
	      /*fprintf(stderr,"starting rdseed ev %d  st %d\n", iev-1, ist );
		scanf("%*1d");*/

	      mk_one_rec (&sdb, iev-1, ist, buff, channel);

	      /*fprintf(stderr,"finishing rdseed ev %d  st %d\n", iev-1, ist );
		scanf("%*1d");*/
	    }
	}
    }

  sdb.nev = iev;

  fclose(ff);

  fflush(NULL);

  fprintf(stderr,"sdb.nst: %d  sdb.nev: %d\n", sdb.nst, sdb.nev);
  ff = fopen("sac_db.out","wb");
  fwrite(&sdb, sizeof(SAC_DB), 1, ff );
  fclose(ff);
  
  sac_db_write_to_asc ( &sdb, "event_station.tbl" );
  sprintf(str,"/bin/rm -rf rdtmp ");
  system(str);
  return 0;
}
/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
        int isign(double f)
/*--------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
{
/*..........................................................................*/
        if (f < 0.)     return -1;
        else            return 1;
}
/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
        int nint(double f)
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
{
        int i;
        double df;
/*..........................................................................*/
                           i=(int)f;
        df=f-(double)i;
        if (fabs(df) > .5) i=i+isign(df);

        return i;
}

/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
	SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, int nmax)
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
{
 FILE *fsac;
/*..........................................................................*/
	fsac = fopen(fname, "rb");
	if ( !fsac )
	{
	 fclose (fsac);
	 return NULL;
	}

	if ( !SHD ) SHD = &SAC_HEADER;

	 fread(SHD,sizeof(SAC_HD),1,fsac);

	 if ( SHD->npts > nmax )
	 {
	   /*fprintf(stderr,
	     "ATTENTION !!! dans le fichier %s npts est limite a %d",fname,nmax);*/

	  SHD->npts = nmax;
	 }

	 fread(sig,sizeof(float),(int)(SHD->npts),fsac);

	fclose (fsac);

   /*-------------  calcule de t0  ----------------*/
   {
	int eh, em ,i;
	float fes;
	char koo[9];

	for ( i = 0; i < 8; i++ ) koo[i] = SHD->ko[i];
	koo[8] = 0;

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
 
   for ( i = 0; i < SHD->npts ; i++ )
   {
    if ( SHD->depmin > sig[i] ) SHD->depmin = sig[i];
    if ( SHD->depmax < sig[i] ) SHD->depmax = sig[i];
   }

	 fwrite(SHD,sizeof(SAC_HD),1,fsac);

	 fwrite(sig,sizeof(float),(int)(SHD->npts),fsac);


	fclose (fsac);
}


/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
	void sac_db_write_to_asc ( SAC_DB *sdb, char *fname )
/*--------------------------------------------------------------------------*/
{
  int ie, is;
  FILE *fi, *ff;
  static SAC_HD shd;

  ff = fopen(fname,"w");
  fprintf(ff,"%d %d \n",sdb->nst, sdb->nev);
  for ( ie = 0; ie < sdb->nev; ie++ ) for ( is = 0; is < sdb->nst; is++ )
    {
      fprintf(ff,"%s  %s  ", sdb->ev[ie].name, sdb->st[is].name );

      if ( sdb->rec[ie][is].n <= 0 ) fprintf(ff,"NO DATA\n");

      else 
	{
	  fi = fopen(sdb->rec[ie][is].fname,"rb");
	  fread(&shd, sizeof(SAC_HD), 1, fi );
	  fclose(fi);

	  fprintf(ff,"%s  t0: %d/%d:%d:%d:%g frac: %g s %g s of record\n", sdb->rec[ie][is].fname, 
		   shd.nzyear, shd.nzjday, shd.nzhour, shd.nzmin, 
		   (shd.nzsec + 0.001*shd.nzmsec), sdb->rec[ie][is].frac,
                   shd.delta*shd.npts );
	}
    } 

  fclose(ff);
}

/*/////////////////////////////////////////////////////////////////////////*/
/*-------------------------------------------------------------------------*/
 int merge_sac(char *sta, char *chan, double *t0, float *dt, int *nrec)
/*-------------------------------------------------------------------------*/
{
  FILE *fi;
  int i, j, N, nfirst=0, Nholes;  /* MB added =0 */
  int k,ib,ie,ist;

  T1 = 1.e25;
  T2 = -100.;
  
/* MB  sprintf(str,"/bin/rm *.10.LHZ.Q.SAC *.AZ.PFO..LHZ.Q.SAC RESP.*.10.LHZ RESP.AZ.PFO..LHZ"); */
  sprintf(str,"/bin/rm *.10.LHZ.M.SAC RESP.*.10.LHZ");
  system(str);

  sprintf(str,"ls *%s*%s*SAC > list_sac", sta, chan);
  system(str);

  fi = fopen("list_sac","r");

  if ( !fi )
    {
      fprintf(stderr,"no list_sac\n");

      sprintf(str,"/bin/rm *%s*%s*SAC", sta, chan);
      system(str);
      system("/bin/rm list_sac");

      fclose(fi);
      return 0;
    }

  if ( fscanf(fi,"%s", fname[0] ) == EOF )  /* MB &(fname[0]) */
    {
      fprintf(stderr,"void list_sac\n" );

      sprintf(str,"/bin/rm *%s*%s*SAC", sta, chan);
      system(str);
      system("/bin/rm list_sac");

      fclose(fi);
      return 0;
    }

  fclose(fi);


  fi = fopen("list_sac","r");

  for ( i = 0; ; )
    {
      if ( fscanf(fi,"%s", fname[i] ) == EOF ) break; /* MB  &(fname[i]) */

      if ( !read_sac ( fname[i], sig1, &(sd[i]), NPTSMAX) )
	{
	  fprintf(stderr,"file %s not found\n", fname[i] );
	  continue;
	}

      t1[i] = htoepoch (sd[i].nzyear, sd[i].nzjday, sd[i].nzhour, 
              sd[i].nzmin, (double)sd[i].nzsec+(double)sd[i].nzmsec/1000.0 );
      t2[i] = t1[i] + sd[i].npts*sd[i].delta;

      if ( t1[i] < T1 )
	{
	  T1 = t1[i];
	  nfirst = i;
	}

      if ( t2[i] > T2 ) T2 = t2[i];

      i++;
    }

  fclose(fi);


  memcpy(&s0, &(sd[nfirst]), sizeof(SAC_HD) );

  N = nint((T2-T1)/s0.delta);

  if ( N > NPTSMAX ) N = NPTSMAX;

  s0.npts = N;

  *t0 = T1;

  *dt = s0.delta;

  *nrec = s0.npts;


  for ( j = 0; j < N; j++ ) sig0[j] = 1.e30;


  fi = fopen("list_sac","r");

  for ( i = 0; ; )
    {
      int nb;
      double ti;

      if ( fscanf(fi,"%s", fname[i] ) == EOF ) break; /* MB  &(fname[i]) */

      if ( !read_sac ( fname[i], sig1, &(sd[i]), NPTSMAX) )
	{
	  fprintf(stderr,"file %s not found\n", fname[i] );
	  continue;
	}

      if ( fabs(sd[i].delta-s0.delta) > .0001 )
	{
	  fprintf(stderr,"incompatible dt in file file %s\n", fname[i] );
	  continue;
	}

      ti = htoepoch (sd[i].nzyear, sd[i].nzjday, sd[i].nzhour, 
           sd[i].nzmin, (double)sd[i].nzsec+(double)sd[i].nzmsec/1000.0 );

      nb = nint((ti-T1)/s0.delta);

      for ( j = 0; j < sd[i].npts; j++ )
	{
	  int jj = nb+j;

	  if ( sig0[jj] > 1.e29 ) sig0[jj] = sig1[j];
	}

      i++;
    }

  fclose(fi);


  Nholes = 0;

  for ( j = 0; j < N; j++ ) if ( sig0[j] > 1.e29 ) Nholes++;

  if ( (float)Nholes/(float)N > pholes ) /* > 0.1 */
    {
      fprintf(stderr,"too many holes\n");
      sprintf(str,"/bin/rm *%s*%s*SAC", sta, chan);
      system(str);
      system("/bin/rm list_sac");

      return 0;
    }
/* linear interpolation of gaps */
  ist = 0;
  ib = -1;
  for(j =0; j < N; j++) {
    if(sig0[j] < 1.e29) {
      if(ist == 0) {
        ib = j; continue;
      } else {
        ie = j;
/* interpolation */
        if (ib > -1) {
          for(k=ib; k <= ie; k++) {
            sig0[k] = (sig0[ie]-sig0[ib])/(ie-ib)*(k-ib)+sig0[ib];
          }
        } else {
          for(k=0; k <= ie; k++) sig0[k] = sig0[ie];
        }
        ist = 0;
        ib =j;
        continue;
      }
    } else {
      ist =1;
      continue;
    }
  }
  if(ist == 1) {
    for(k=ib; k <= N; k++) sig0[k] = sig0[ib];
  }

  write_sac ("merged.sac", sig0, &s0);


  sprintf(str,"/bin/rm *%s*%s*SAC", sta, chan);
  system(str);
  system("/bin/rm list_sac");

  return 1;
}
/*////////////////////////////////////////////////////////////////////////*/
/*------------------------------------------------------------------------*/
 void mk_one_rec (SAC_DB *sdb, int ne, int ns, char *nseed, char *ch)
/*------------------------------------------------------------------------*/
{
  FILE *ff,*fi;
  static char resp_name[150];
  double frac,sfr;
  int    nfr,nfrac;

  if ( sdb->rec[ne][ns].n > 0 ) return;

/*------------------- get sac data ---------------------------------*/
  sprintf(str,"/bin/mv rdtmp/*%s.*%s* .",sdb->st[ns].n_name,ch);
  system(str);
/*---------------change every sac file -----------------------------*/
  sprintf(str,"ls *%s.*%s.*SAC>list_sac1",sdb->st[ns].n_name,ch);
  system(str);
  int i=0;
  nfr = 0;
  sfr = 0.0;
  fi=fopen("list_sac1","r");
  sdb->rec[ne][ns].frac = 0.0;
  
  if(fi)
     for (i=0;;)
      {
      if(fscanf(fi,"%s",fname[i])!=EOF)
        {
/*        fputs(fname[i],stderr); */
        change_sac_file(fname[i],&frac,&nfrac);
        nfr += nfrac;
        sfr += frac*nfrac;
        i++;
        
        }
       else break;
       }
       if(nfr != 0) {
          sfr /= nfr;
          sdb->rec[ne][ns].frac = sfr;
          fprintf(stderr,"FRAQ_AVE: %lf \n",sfr);
       }
  fclose(fi);
/*---------------------------over----------------------------------*/





  if ( !merge_sac(sdb->st[ns].n_name, ch, &(sdb->rec[ne][ns].t0), &(sdb->rec[ne][ns].dt), &(sdb->rec[ne][ns].n) ) )
    {
      sdb->rec[ne][ns].n = 0;
      sprintf(str,"/bin/rm RESP*%s*%s*",sdb->st[ns].n_name,  ch);
      system(str);
      return;
    }

  /*---------- response file -----------*/
  sprintf(str,"ls RESP*%s.*%s* > list_resp",  sdb->st[ns].n_name,  ch);
  system(str);

  ff = fopen("list_resp","r");

  if ( fscanf(ff,"%s", resp_name ) == EOF )
    {
      sdb->rec[ne][ns].n = 0;
      return;
    }

  fclose(ff);

  sprintf(sdb->rec[ne][ns].resp_fname,"%s/%s", sdb->ev[ne].name, resp_name);
  sprintf(str,"/bin/mv %s %s", resp_name, sdb->rec[ne][ns].resp_fname);
  system(str);

  system("/bin/rm list_resp");
/*  system("/bin/rm SAC_PZs_*");                */


  /*------------- mooving sac file -------*/
  sprintf(str,"/bin/mv merged.sac %s/%s.%s.SAC", sdb->ev[ne].name, sdb->st[ns].n_name, ch);
  system(str);
  
  sprintf(sdb->rec[ne][ns].fname,"%s/%s.%s.SAC", sdb->ev[ne].name, sdb->st[ns].n_name, ch);
  sprintf(sdb->rec[ne][ns].ft_fname,"%s/ft_%s.%s.SAC", sdb->ev[ne].name, sdb->st[ns].n_name, ch);

  sprintf(sdb->rec[ne][ns].chan,"%s", ch );
}

/*////////////////////////////////////////////////////////////////////////*/
/*------------------------------------------------------------------------*/
 void  fill_one_sta (STATION *st, char *buff )
/*------------------------------------------------------------------------*/
{
  char snet[10], sname[12];

/*  for ( i = 0; i < 5; i++ )
    {
      if ( buff[i] == ' ' ) break;
      else st->name[i] = buff[i];
    } 

  st->name[i] = '\0'; */

  /*fprintf(stderr,"station name %s\n", st->name );*/

  sscanf(buff,"%s %s %f %f", snet,sname,&(st->lon), &(st->lat) );
  strcpy(st->name,sname);
  strcpy(st->n_name,snet);
  strcat(st->n_name,".");
  strcat(st->n_name,sname);
}

/*////////////////////////////////////////////////////////////////////////*/
/*------------------------------------------------------------------------*/
  void fill_one_event (EVENT *ev, char *buff )
/*------------------------------------------------------------------------*/
{
  sscanf(&(buff[7]),"%d %d %d", &(ev->yy),  &(ev->mm),  &(ev->dd));
/* sscanf(&(buff[7]),"%d", &(ev->yy) );
  sscanf(&(buff[14]),"%d", &(ev->mm) );
  sscanf(&(buff[17]),"%d", &(ev->dd) );
  sscanf(&(buff[20]),"%2d", &(ev->h) );
  sscanf(&(buff[22]),"%2d", &(ev->m) );
  sscanf(&(buff[24]),"%2d", &(ev->s) );
  sscanf(&(buff[27]),"%2d", &(ev->ms) ); */

  ev->h  = 0;
  ev->m  = 0;
  ev->s  = 0;
  ev->ms = 0;

  ev->jday = mtodoy( ev->yy, ev->mm, ev->dd );

  ev->t0 = htoepoch (ev->yy, ev->jday, ev->h, ev->m, (double)ev->s+(double)ev->ms/1000.0 );
  sprintf(ev->name,"%d_%d_%d_%d_%d_%d",ev->yy, ev->mm, ev->dd, ev->h, ev->m, ev->s );

  sprintf(str,"mkdir %s", ev->name );
  system(str);
}
/*========================================================================*/

int change_sac_file(char *buff1, double *frac, int *nfrac)
{
SAC_HD shd;
int ierr,nf;
double t,sec,frac1;

  ierr = 0;
  *frac = 0.0;
  shd = sac_null;
  if (read_sac(buff1,sig1,&shd,1000000))
  {
    *nfrac = shd.npts;
    t = htoepoch(shd.nzyear,shd.nzjday,shd.nzhour,shd.nzmin,
                (double)shd.nzsec+(double)shd.nzmsec/1000.0)+shd.b;
fprintf(stderr,"FRAC: %s\n",buff1);
fprintf(stderr,"FRAC: %d %d %d %d %d %d %f\n",shd.nzyear,shd.nzjday,shd.nzhour,shd.nzmin,shd.nzsec,shd.nzmsec,shd.b);

    frac1 = t-floor(t);
    nf = frac1/shd.delta;
    *frac = t-(floor(t)+nf*shd.delta);
    t = floor(t)+nf*shd.delta;
    if(*frac > 0.5*shd.delta) {
      t+=shd.delta;
      *frac = *frac - shd.delta; 
    }
    epochtoh(t,&shd.nzyear,&shd.nzjday,&shd.nzhour,&shd.nzmin,&sec);
    shd.nzsec = sec;
    shd.nzmsec = (sec-shd.nzsec)*1000.0;
    shd.b = 0.0;
fprintf(stderr,"FRAC: %d %d %d %d %d %d %f %lf\n",shd.nzyear,shd.nzjday,shd.nzhour,shd.nzmin,shd.nzsec,shd.nzmsec,shd.b,*frac);
    write_sac(buff1,sig1,&shd);
    return 0;
  } else {
    ierr = 1;
  }
  return ierr;
}
/*==================================================================*/
