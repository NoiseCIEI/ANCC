#!/bin/tcsh
# Usage: sac_update.sh db_path db_name
if($#argv != 2) then
echo "Usage: sac_update.sh db_path db_name"
exit
endif
set dbpath=$1/$2
cd $dbpath
foreach f (`ls`)
echo "Working with $f ......"
cd  $f
ls *.SAC | awk '{print "r",$1; print "ch idep IUNKN";print "wh",$1;}END{print "quit"}' > tmp.1
sac < tmp.1
rm -f tmp.1
cd ..
end
exit
