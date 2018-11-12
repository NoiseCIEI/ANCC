#ifndef _SAC_DB_H
#define _SAC_DB_H  1

/* Type definitions          */
typedef struct buf
{
  int n;
  float am[100000];
  float ph[100000];
}
BUF;

typedef struct event
{
  float lat, lon;
  int yy, mm, dd, h, m, s, ms, jday;
  double t0;
  char name[40];
}
EVENT;

typedef struct station
{
  float lat, lon;
  char name[10];
  char n_name[12];
}
STATION;

typedef struct record
{
  double frac;
  char fname[150];
  char ft_fname[150];
  char resp_fname[150];
  char chan[7];
  double t0;
  float dt;
  int n;
}
RECORD;

#define NSTATION 1200
#define NEVENTS 50

typedef struct sac_dbase
{
  EVENT ev[NEVENTS];
  STATION st[NSTATION];
  RECORD rec[NEVENTS][NSTATION];
  int nev, nst;
}
SAC_DB;

typedef struct sac_dbase3
{
  EVENT ev[NEVENTS];
  STATION st[NSTATION];
  RECORD rz[NEVENTS][NSTATION];
  RECORD rn[NEVENTS][NSTATION];
  RECORD re[NEVENTS][NSTATION];
  int nev, nst;
}
SAC_DB3;

#endif /* sac_db.h */
