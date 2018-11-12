#define MAIN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mysac.h"

/* Function prorotypes */
void filter4_(double *f1,double *f2,double *f3,double *f4,int *npow,
              double *dt,int *nwt, int *n, float seis_in[],float seis_out[],
              float seis_outamp[], float seis_outph[],int *ns,
              double *dom, int *patch,double *freqmin);

void swapn(unsigned char *b, int N, int n);
SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, int nmax);
void write_sac (char *fname, float *sig, SAC_HD *SHD);

 float sig[1000000];
 SAC_HD shd1;


/*c/////////////////////////////////////////////////////////////////////////*/
/* ========================================================================
  Usage:  whiten_rej_phamp parameter_file

    The parameter_files ASCII plane file, each line includes the
    following filelds separated one or more spaces:
    T1,T2,T3,T4,dt,npow,nwt,tnorm,fr1,fr2,nsmooth,onebit,patch,freqmin,name
    where,
    T1,T2,T3,T4 - corner periods in seconds, T1>T2>T3>T4>0, spectral whitening,
                   real
    dt          - sampling step in seconds, spectral whitening, real
    npow        - power of cosine ends tapering, spectral whitening, integer
    nwt         - half width of smoothing window in samples, spectral
                  whitening, integer
    tnorm       - one character "Y" or "N", apply or not apply temporal
                  normalization
    fr1, fr2    - corner frequencies of temporal normalization in Hz , used if
                  tnorm = Y, real values
    nsmooth     - half width of smoothing window of temporal normalization
                  in samples, used if tnorm = Y, integer
    onebit      - one character "Y" or "N", apply or not apply one-bit
                  normalization. We don't recommend to use one-normalization.
    notch       - one character "Y" or "N", apply or not apply notch
                  correction for 26 sec period. Be sure that this effect
                  presents for your area of investigation.
    freqmin     - spectral amplitude damping for the notch correction,
                  0 < freqmin < 1.0. Smaller value of freqmin means stronger
                  damping. freqmin = 0.5 is recommended value.
    name_file   - the name of input ASCII binary SAC waveform file in working
                  directory.
   ======================================================================== */

int main (int argc, char *argv[])
{
static int n, ns,npow;
static double f1, f2, f3, f4, dt,dom;
static float seis_in[400000],seis_out[400000];
static float seis_outamp[400000],seis_outph[400000];
double t1,t2,t3,t4,freqmin;
char  name[160], name1[160];
char  nameamp[160],nameph[160];
int   nwt,nsmooth,tnorm,patch,onebit;
char  stnorm[10],spatch[10],sonebit[10];
char  fr1[20],fr2[20];
FILE  *in, *ff;
int   i, j, nn;


  if( argc != 2) {
      fprintf(stderr,"Usage: whiten_rej_phamp parameter_file\n");
      exit(-1);
  }
  shd1 = sac_null;
/* open and read parameter file param.dat                      */
  if((in = fopen(argv[1],"r")) == NULL) {
      fprintf(stderr,"Can not find file %s.\n",argv[1]);
      exit(1);
  }

  while((nn = fscanf(in,"%lf %lf %lf %lf %lf %d %d %s %s %s %d %s %s %lf %s",
            &t1,&t2,&t3,&t4,
            &dt,&npow,  &nwt,stnorm,fr1,fr2,&nsmooth,sonebit,
            spatch,&freqmin,name)) != EOF) { /* start main loop  */
      tnorm = onebit = patch = 0;
      if(stnorm[0] == 'Y' || stnorm[0] == 'y') tnorm = 1;
      if(sonebit[0] == 'Y' || sonebit[0] == 'y') onebit = 1;
      if(spatch[0] == 'Y' || spatch[0] == 'y') patch = 1;
      if(nn == 0 || nn != 15) break;
      fprintf(stderr,"Corners periods. Low: %f - %f, High: %f - %f\n",t1, t2, t3, t4);
      fprintf(stderr,"Step: %f, Cosine power: %d, nwt: %d\n",dt, npow,nwt);
      fprintf(stderr,"Tnorm: %s:%d %s %s %d\n",stnorm,tnorm,fr1,fr2,nsmooth);
      fprintf(stderr,"Onebit: %s:%d, Patch: %s:%d %lf\n",sonebit,onebit,spatch,patch,freqmin);

/* remove quotes from name                                   */
      j = 0;
      for(i = 0; i < strlen(name); i++) {
          if(name[i] == '\'' || name[i] == '\"') continue;
          name[j] = name[i]; j++;
      }
      name[j] = '\0';

/* do running average before whitening                       */

  ff = fopen("sac_one_cor","w");
  fprintf(ff,"sac << END\n");
/*  fprintf(ff,"/data/ulisse/barmin/sac/bin/sac << END\n"); */
  fprintf(ff,"r %s\n", name);
  if(tnorm == 1) {
  fprintf(ff,"bp co %s %s n 4 p 2\n",fr1,fr2);
  fprintf(ff,"abs\n");
  fprintf(ff,"smooth mean h %d\n",nsmooth);
  fprintf(ff,"w a.avg \n");
  fprintf(ff,"r %s\n",name);
  fprintf(ff,"divf a.avg\n");
}
  fprintf(ff,"w smooth.sac\n");
  fprintf(ff,"quit\n");
  fprintf(ff,"END\n");
  fclose(ff);
/*  system("cat sac_one_cor"); */
  system("sh sac_one_cor");  

  /* end of running average                             */
  /* now do one_bit_whiten                              */

if(onebit == 1) {
   if (! read_sac(name,sig, &shd1, 1000000))  {
      fprintf(stderr,"file %s did not found\n", name );
      return 0;
    }
   for (i =0;i<shd1.npts;i++)
      {
      if(sig[i]>0) sig[i]=1;  
      if(sig[i]<0) sig[i]=-1;  
      }
   write_sac("smooth.sac",sig, &shd1);
}

  if ( !read_sac("smooth.sac", sig, &shd1, 1000000 ) )
    {
      fprintf(stderr,"file %s did not found\n", name1 );
      return 0;
    }

    n  = shd1.npts;
    dt = shd1.delta;
   
     for( i =0; i< n; i++)
     {  
     seis_in[i] = sig[i];  
     /*     printf(" seis_in1  %d %f\n", i,sig[i]);      */
     }

      fprintf(stderr," Dt1= %f, Nsamples1= %d\n",dt, n);


  f1 = 1.0/t1; f2 = 1.0/t2; f3 = 1.0/t3; f4 = 1.0/t4;
  filter4_(&f1,&f2,&f3,&f4,&npow,&dt,&nwt,&n,seis_in,seis_out,seis_outamp,seis_outph,&ns,&dom,&patch,&freqmin);
     
	shd1.npts = n;
	shd1.delta = dt;
        write_sac(name,seis_out, &shd1);

        strcpy(nameamp,name);
        strcpy(nameph,name);
        strcat(nameamp,".am");
        strcat(nameph, ".ph");
	shd1.npts = ns/2 + 1;
	shd1.delta = dom;
	shd1.b = 0;
        shd1.iftype = IXY;
        write_sac(nameamp,seis_outamp, &shd1 );
        write_sac(nameph, seis_outph,  &shd1 );



  }
   fflush(NULL); 
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
	  fprintf(stderr,
	   "ATTENTION !!! dans le fichier %s npts est limite a %d",fname,nmax);

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
