#!/bin/tcsh
#
# count datata into each subdir
if( $#argv != 2)then 
echo "Usage: count.sh dir file"
exit
endif
set cdir=`pwd`
set dir=$2
cd $1/$2
set sdir=`/bin/ls`
if( -f $cdir/tmp.1) rm -f $cdir/tmp.1
touch $cdir/tmp.1
foreach f ($sdir)
cd $f
set nf=`/bin/ls | /usr/bin/wc -l`
echo $f $nf >> $cdir/tmp.1
cd ..
end
awk 'BEGIN{n=0}{n+=$2}END{print n}' $cdir/tmp.1
rm -f $cdir/tmp.1
exit
