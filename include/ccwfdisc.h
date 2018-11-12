#ifndef _CCWFDISC_H
#define _CCWFDISC_H  1

typedef struct ccwfdisc {
char	sta1[8];
char	net1[8];
char	sta2[8];
char	net2[8];
char	chan1[8];
char	chan2[8];
double	time;
int	wfid;
double	endtime;
int	nsamp;
float	samprate;
float	snrn;
float	snrp;
int	sdate;
int	edate;
int	stdays;
float	range;
double	tsnorm;
char	datatype[4];
char	dir[32];
char	dfile[32];
int	foff;
char	lddate[20];
} CCWFDISC;

static CCWFDISC ccwfdisc_null = {
"_",			/* sta1 */
"_",			/* net1 */
"_",			/* sta2 */
"_",			/* net2 */ 
"_",			/* chan1 */ 
"_",			/* chan2 */ 
-9999999999.999,	/* time */
-1,			/* wfid */
/* -1,			   jdate */
-9999999999.999,	/* endtime */
-1,			/* nsamp */
-1.,			/* samprate */
-1.,			/* snrn */
-1.,			/* snrp */
-1,			/* sdate */
-1,			/* edate */
0,			/* stdays */
-1.,			/* range */
-1.,			/* tsnorm */
"_",			/* datatype */ 
"_",			/* dir */
"_",			/* dfile */
-1,			/* foff */
"_"			/* lddate */
};

#define CCWFDISC_RCS "%s %s %s %s %s %s %lf %d %lf %d %f %f %f %d %d %d %f %lf %2s %s %s %d %s"
#define WFDISC_RVL(SP) \
(SP)->sta1, (SP)->net1, (SP)->sta2, (SP)->net2, (SP)->chan1, \
(SP)->chan2, &(SP)->time, &(SP)->wfid, &(SP)->endtime, \
&(SP)->nsamp, &(SP)->samprate, &(SP)->snrn, &(SP)->snrp, &(SP)->sdate, \
&(SP)->edate, &(SP)->stdays, &(SP)->range, &(SP)->tsnorm, (SP)->datatype, \
(SP)->dir, (SP)->dfile, &(SP)->foff, (SP)->lddate

#define CCWFDISC_WCS "%-6.6s %-8.8s %-6.6s %-8.8s %-8.8s %-8.8s %12.3lf %8d %12.3lf %8d %11.7f %16.6f %16.6f %8d %8d %6d %10.3f %14.4lf %-2.2s %-32.32s %-32.32s %10d %-17.17s\n"
#define CWFDISC_WVL(SP)\
(SP)->sta1, (SP)->net1, (SP)->sta2, (SP)->net2, (SP)->chan1, \
(SP)->chan2, (SP)->time, (SP)->wfid, (SP)->endtime, \
(SP)->nsamp, (SP)->samprate, (SP)->snrn, (SP)->snrp, (SP)->sdate, \
(SP)->edate, (SP)->stdays, (SP)->range, (SP)->tsnorm, (SP)->datatype, \
(SP)->dir, (SP)->dfile, (SP)->foff, (SP)->lddate

#endif /* end of CCWFDISC */
