/*
 * MINEOS version 1.0 by Guy Masters, John Woodhouse, and Freeman Gilbert
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ***************************************************************************
 *
 * time conversation routines
 *
 *************************************************************************** */
/* 
 *  convert epoch time to human time
 */
#include <stdio.h>
void epochtoh(double t, int *year,int *doy,int *hour,int *miin,double *sec)
{
  double fsec;
  int    idate, itime, irest,iysupp,idsupp;

/*...  date parts                                   */
      idate = t/86400.0;
      iysupp = idate/365;
      idsupp = iysupp*365+(iysupp+1)/4+(iysupp+369)/400-(iysupp+69)/100;
      if(idate < idsupp) iysupp -= 1;
      idsupp = iysupp*365+(iysupp+1)/4+(iysupp+369)/400-(iysupp+69)/100;
/*... extract year                                   */
      *year = 1970+iysupp;
/*... extract doy                                    */
      *doy = idate - idsupp+1;
/*... time part                                      */
      fsec = t-idate*86400.0;
      irest = fsec;
      fsec = fsec - irest;
      itime = irest;
      irest = itime/60;
/*... extract seconds                                 */
      *sec = itime - irest*60 + fsec;
      itime = irest;
      irest = itime/60;
/*... extract mininutes                               */
      *miin = itime - irest*60;
/*... extract hours                                   */
      *hour = irest;
} /* end  epochtoh */
/*
 * convert human time to epoch time
 */
double htoepoch(int year,int doy,int hour,int miin,double sec)
{
int    vis, ydelay;
double t;
      ydelay = year - 1970;
      vis = (ydelay+1)/4+(ydelay+369)/400-(ydelay+69)/100;
      t = (365.0*ydelay+vis+doy-1)*86400.0+
             (hour*60.0+miin)*60.0+sec;
      return t;
}  /* end  htoepoch */
/*
 * ... convert month and day to day of year - doy
 */
int mtodoy(int year,int mon,int day)
{
int i,jday;
int days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
      
      days[1] = 29;
      if((year % 4) != 0 || ((year % 100) == 0 &&
         (year % 400) != 0)) days[1] = 28;
      jday = 0;
      if(mon > 1) {
        for(i = 0;i <= mon-2; i++) {
            jday = jday+days[i];
        }      
      }
      return jday+day;
} /* end mtodoy  */
/* 
 * ... convert day of year to month and day
 */
void doytom(int year,int doy,int *mon,int *day)
{
int i;
int days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

      days[1] = 29;
      if((year % 4) != 0 || ((year % 100) == 0 &&
         (year % 400) != 0)) days[1] = 28;
      *mon = 1;
      *day = doy;
      for( i = 0; i <= 10; i++) {
        if(*day <= days[i]) break;
        *mon = *mon+1;
        *day = *day-days[i];
      }
}  /* end doytom  */
/* c
c get local time in form mm/dd/yy-hh:mm:ss
c
c     character*17 function loctime()
c     implicit none
c     integer*4 itime,ia(9),time,i
c     itime=time()
c     call ltime(itime,ia)
c     write(loctime,1000) ia(5)+1,ia(4),mod(1900+ia(6),100),
c    +                    ia(3),ia(2),ia(1)
c     do i = 1,17
c       if(loctime(i:i).eq.' ') loctime(i:i) = '0'
c     enddo
c     return
c1000 format(i2,'/',i2,'/',i2,'-',i2,':',i2,':',i2)
c     end */
