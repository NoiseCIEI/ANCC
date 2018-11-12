#ifndef _ETIME_H
#define _ETIME_H       1

/* Function prototypes                                  */

void epochtoh(double t, int *year,int *doy,int *hour,int *miin,double *sec);
double htoepoch(int year,int doy,int hour,int miin,double sec);
int mtodoy(int year,int mon,int day);
void doytom(int year,int doy,int *mon,int *day);

#endif /* etime.h  */
