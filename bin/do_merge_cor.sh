#!/bin/tcsh 
# Merge multiple databases into single one
if( $#argv != 3) then
echo "Usage: do_merge_cor.sh param_file dir_cor file_cor"
exit
endif
set in_param=$1  # param.dat
set out_dir=$2   # MERGED_COR
set out_name=$3  # COR_2005.4_2005.5
set out_path=$out_dir/$out_name
#
if( ! -d $out_path) mkdir -p $out_path
if( -f list) rm -f list station.list
touch list station.list
@ n = 0
set dir=`pwd`
foreach f (`cat $in_param`)
echo "$f .........."
@ n = $n + 1
cd $f
# foreach ff (`ls`)
# cd $ff
# ls | awk '{print '$n',"'$f'""/""'$ff'""/"$1;}' >> $dir/list
# cd ..
# end
/bin/ls -R | egrep ":|SAC" | sed -e"/^\.:/d" -e"s/^\.//" -e"s/://" | \
awk '{if(substr($0,1,1) == "/"){d=$1;next;}; \
print '$n',"'$f'"d"/"$1}' >> $dir/list
pwd
cat ../../station.lst >> $dir/station.list
cd $dir
end
cat station.list | sort -u > $out_dir/station.lst
../../bin/merge_cor list $out_path
rm -f list list_1 station.list
exit
