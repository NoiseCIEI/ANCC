#!/bin/tcsh
#
# Monthly cross correlations stacking script
# Usage: cv_do_CO.csh year b_month e_month
#        where
#        year - year of stacking of form "yyyy"
#        b_month - staring mohth in year
#        e_month - ending month in year
#
if($#argv != 3) then
echo "Usage: cv_do_CO.csh year b_month e_month"
exit
endif
set mmm=(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec)
set ddd=(31 29 31 30 31 30 31 31 30 31 30 31)
set year=$1
echo "Stacking for $year from $mmm[$2] to $mmm[$3]"
alias cp cp
alias mv mv
#
set work0=`pwd`
set prog=$work0:h/bin
set seed_data=$work0:h/SEED
set bp = 5to150
if( ! -d $year) mkdir $year
cd $year
set work0=`pwd`
set mlist=`awk '{}END{for(i='$2'; i <= '$3';i++) print i}' /dev/null`
# Loop by months
foreach month ($mlist )
echo "Stacking for $year $mmm[$month]"
set dd=$ddd[$month];
set ndir = $month
mkdir ${month}_in
mkdir  $ndir
cd $ndir   # change work space to work1 = $work0/$month
# generate days, $mday, in current $month
set mday=`awk '{}END{y='$year';m='$month';d='$dd';if(m == 2){if((y % 4 != 0) || ((y % 100 == 0) && (y %400 != 0))) d=28}; for(i=1; i <= d;i++) print i}' /dev/null`
# This loop creates event list input_ev_seed and station list
# from SEED volumes for current month $month

foreach day1 ($mday)
# copy SEED volumes to  $work0/${month}_in dir
cp $seed_data/${year}/${month}/D.$year.???.$month.${day1}.*  ../${month}_in/ALL_${year}_${month}_${day1}
# output date of the SEED volume of a current day
if ( $day1 == 1 ) then
echo " PDE   ${year}    $month $day1  0000000000000     63.52 -147.44  11 8.50 Ms GS   9C.G F.S."  > input_ev_seed
else
echo " PDE   ${year}    $month $day1  0000000000000     63.52 -147.44  11 8.50 Ms GS   9C.G F.S."  >> input_ev_seed
endif
# output path of a SEED volume relative to space work1
echo ../${month}_in/ALL_${year}_${month}_${day1} >> input_ev_seed
end    # END loop by day1

# create station list for current month 
if( -f tmp.1) rm -f tmp.1
touch tmp.1
foreach svol (`ls ../${month}_in/ALL_*`)
if( -f rdseed.stations) rm -f rdseed.stations
rdseed -S -f $svol >& /dev/null
cat  rdseed.stations >> tmp.1
end
# awk '{print $2,$1,$4,$3}' tmp.1 | sort -u +0.0 -2.0 | \
awk '{print $2,$1,$4,$3}' tmp.1 | sort -u -b -k 1,1 -k 2,2 | \
awk '{printf("%-4.4s %-6.6s %12.6f %10.6f\n",$1,$2,$3,$4);}' > station.lst
cp -f station.lst ../${month}_in
rm -f rdseed.stations tmp.1
# cp $seed_data/${year}/${month}/station.list ./station.lst

echo "Stage1: cv_sa_from_seed_holes_RESP started ============== "
date
# Usage: cv_sa_from_seed chan gap
# chan - channel name
# gap  - gap in data, gap*100 in %
$prog/cv_sa_from_seed_holes_RESP LHZ 0.1
echo "Stage2: cut_trans_RESP started ========================== "
date
# Usage: cut_trans T1 T2 T3 T4 t1 npts
# T1 > T2 > T3 > T4 corner periods of transfer (evalresp) function
# t1 - skip t1 points from begin record
# npts - stay npts points after skipping
$prog/cut_trans_RESP 170.0 150.0 5.0 4.0 1000 83000
cp ../event.dat .

mkdir $bp
cd $bp

cp ../*.*  .

echo "Stage3: filter4 & whiten_rej_phamp started ===================== "
date
foreach day ($mday)
set dirday =  ${year}_${month}_${day}_0_0_0
echo "Day: $dirday ==========================="
mkdir  $dirday
cd $dirday
cp ../../${dirday}/ft_* .
ls ft_* > temp.ft
awk '{print "200 150 5 4 1 1  ", $1}' temp.ft > param.dat
# Usage: filter4 param_dat
# param_dat contents: T1 T2 T3 T4 dt npow name
$prog/filter4  param.dat
awk '{print "200 150 5 4 1 1 20 Y 0.02 0.0667 40 N N 0.5 ", $1}' temp.ft > param1.dat
# Usage: whiten_rej_phamp  param_dat
# param_dat contents: T1 T2 T3 T4 dt npow nwt tnorm fr1 fr2 nsmooth onebit notch freqmin  name
$prog/whiten_rej_phamp  param1.dat
# \rm ft_*.SAC
cd ../  # up from dirday
end

cd ../ # up from bp
cp sac_db.out $bp/
cd $bp

mkdir COR

echo "Stage4: justCOR_mv_dir started ===================== "
date
$prog/justCOR_mv_dir  3600 83000
cd .. # up from bp
cd .. # up from ndir
date
end # end loop by months
exit
