#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Finction prorotypes */

void filter4_(double *f1,double *f2,double *f3,double *f4,int *npow,
              double *dt,int *n, double *frac, float seis_in[],
              float seis_out[]);
void swapn(unsigned char *b, int N, int n);

/* ============================================================
  Program filter4 applies broadband filter to SAC waveform file
  updating it in place.
  Usage: filter4 parameter_file
  Contents of parameter_file:
  Line fields: T1, T2, T3, T4, dt, npow, name
  T1,T2,T3,T4 - corner periods in seconds, T1>T2>T3>T4>0
  dt          - sampling step in seconds
  npow        - power of cosine ends tapering, integer
  name        - path to ASCII binary SAC waveform file
   ============================================================ */

int main (int argc, char *argv[])
{
static int n, npow;
static double f1, f2, f3, f4, dt, frac;
static float seis_in[400000],seis_out[400000];

double delta, t1,t2,t3,t4;
float hed[158];
char  name[160], name1[160];
FILE  *in, *out, *fd;
int   i, j, nn, *nsamples,iswap,*ih;

/* input command line arguments treatment      */
  if(argc != 2) {
      printf("Usage: filter4 parameter_file\n");
      exit(-1);
  }
/* open and read parameter file param.dat      */
  if((in = fopen(argv[1],"r")) == NULL) {
      printf("Can not find file %s.\n",argv[1]);
      exit(1);
  }
  while((nn = fscanf(in,"%lf %lf %lf %lf %lf %d %s",&t1,&t2,&t3,&t4,
            &dt,&npow,name)) != EOF) { /* start main loop  */
      if(nn == 0 || nn != 7) break;
      printf("Corners periods. Low: %f - %f, High: %f - %f\n",t1, t2, t3, t4);
      printf("Step: %f, Cosine power: %d\n",dt, npow);
/* remove quotes from name            */
      j = 0;
      for(i = 0; i < strlen(name); i++) {
          if(name[i] == '\'' || name[i] == '\"') continue;
          name[j] = name[i]; j++;
      }
      name[j] = '\0';

/*
 * Read SAC header and seismogram
 */
      iswap = 0;
      if((fd = fopen(name,"r")) == NULL) {
          printf("Can not find file %s.\n",name);
          exit(1);
      }
/*       The header                         */
      fread(hed,sizeof(float),158,fd);
      ih = (int *)(&hed[76]);
      if(*ih > 100 || *ih < -100) iswap = 1;
      if(iswap) swapn((unsigned char *)hed,(int)(sizeof(float)),158);
      dt = hed[0];
      delta = hed[50];
      nsamples = (int *)(&hed[79]);
      frac = hed[41];
      printf("F4_FRAQ: %lf STEP: %lf \n", frac,dt);
/*       The body                           */
      fread(seis_in,sizeof(float),*nsamples,fd);
      fclose(fd);
      if(iswap) swapn((unsigned char *)seis_in,(int)(sizeof(float)),*nsamples);

/* seismogram is ready                      */

      printf("Delta= %f, Dt= %f, Nsamples= %d\n",delta,dt,*nsamples);
      n = *nsamples;
/* ==========================================================
 * Parameters for filter4 function:
 * Input parameters:
 * f1,f2   - low corner frequences, f2 > f1, Hz, (double)
 * f3,f4   - high corner frequences, f4 > f3, Hz, (double)
 * npow    - power of cosine tapering,  (int)
 * dt      - sampling rate of seismogram in seconds, (double)
 * n       - number of input samples, (int)
 * seis_in - input array length of n, (float)
 * Output parameters:
 * seis_out - output array length of n, (float)
 * ==========================================================
 */
/* Call filter4 function                 */

  f1 = 1.0/t1; f2 = 1.0/t2; f3 = 1.0/t3; f4 = 1.0/t4;
  filter4_(&f1,&f2,&f3,&f4,&npow,&dt,&n,&frac,seis_in,seis_out);

/*  hed[41] = 0.0                              */
/*  write results to SAC file                  */
/*      name[strlen(name)-3] = 0;              */
      strcpy(name1,name);
/*      strcat(name1,"F4.SAC");                */
      printf("In: %s, Out: %s\n", name,name1);
      if((out = fopen(name1,"w")) == NULL) {
          printf("Can not open file %s.\n",name1);
          exit(1);
      }
      if(iswap) swapn((unsigned char *)hed,(int)(sizeof(float)),158);
      fwrite(hed,sizeof(float),158,out);
      if(iswap) swapn((unsigned char *)seis_out,(int)(sizeof(float)),n);
      fwrite(seis_out,sizeof(float),n,out);
      fclose(out);
   } /* end of main loop */
   fclose(in);
   return 0;
  }
