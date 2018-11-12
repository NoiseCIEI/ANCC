# ANCC

DISCLAIMER

This is a ancc package as delivered to IRIS DMC the University of
Colorado by Mikhail Barmine. This is the first release version 1.0
that will provide computation of vertical component of ambient noise
cross-correlations.

The distribution includes all sources needed for compiling for the host
computer.

University of Colorado does not provide any maintenance or support
for public user community, this will be done by IRIS DMC.

This package is distributed as is in the hope that it will be useful,
but WITHOUT ANY WARRANTY.

CONTENTS:

1. PACKAGE CONTENTS
2. INSTALLATION
3. SYSTEM REQUIREMENTS
3. COMPUTATION OF AMBIENT NOISE CROSS-CORRELATIONS
3.1 DATA PREPARATION
3.2 DATA PROCESSING
3.2.1 cv_do_CO.csh program (tcsh script)
3.2.1.1 cv_sa_from_seed_holes_RESP program
3.2.1.2 cut_trans_RESP program
3.2.1.3 filter4 and whiten_rej_phamp programs
3.2.1.4 justCOR_mv_dir program
3.2.2 MERGING DATABASES
3.2.3 POSTPROCESSING
REFERENCES
APPENDIX A


1. PACKAGE CONTENTS
   ================
The contents of the package are as follows:

README		this file
bin		contains executable programs and scripts
doc             contains Data Product Manual
include		contains include files for the program in src directory
SEED		contains SEED volumes data archive
src             contains program source codes
TEST1		contains precomputed example test
TEST2		directory to run your own  installation

1. INSTALLATION
   ================
   Use the tar command to extract distribution contents.

     ...> tar xvf ancc-1.0-0.src.tar

This will yield a top level directory named ancc-1.0-0 with an executable,
along with other miscellaneous files.
Go to ancc-1.0-0/src directory and recompile if necessary all programs.
Go to corresponding subdirectory and recompile it with
   ...> make
   ...> make install

The resulting binary modules will be installed under ancc-1.0-0/bin directory.
In case of problems update Makefile properly.
The ancc-1.0-0/bin directory originally includes binary modules
compiled by gcc/g77 compilers under RHEL 5.8 (RedHat Enterprise Linux 5.8)
Operational System.

The package required installation of the following software:

- rdseed 5.1 - reading SEED volumes;
- sac 104.1  - seismic analysis code;
- fftw 3.2   - libraries for the fast Fourier transformation

rdseed and sac command must be in user's path, and fftw's libraries must be
installed into OS for better performance.
See an example of run in the directory TEST1 and try your own run
in directory TEST2.

2. SYSTEM REQUIREMENTS
   ===================

   - 64-bit or 32-bit OS, 64-bit is preferable
   - Linux Operation System, currently tested on
     RHEL 5.x, 32 and 64 bit version
   - 2-core CPU, 2.0 Ghz or more
   - 4 Gb of RAM or more
   - 256 Gb hard disk or more (depends on situation)
   - gcc/g77 compilers
   - Software: rdseed 5.1, sac 101.4 and fftw 3.2

3. COMPUTATION OF AMBIENT NOISE CROSS-CORRELATIONS
   ================================================

The process of evaluating cross-correlations (CCs) consists of
three basic steps, namely, data preparation, data processing and data
postprocessing described below.

3.1 DATA PREPARATION
    ================

After choosing the area of investigation and the set of seismic stations,
it is necessary to create an archive of continuous waveforms during
desired period of time. Usually, it is a couple of months or years.
Right now, software works only for vertical components of seismic
record (LHZ channel) with sampling step 1 second. The archive consists of
daily IRIS SEED volumes including continuous waveforms for all stations
in the area of investigation that are operating at that day. SEED volume must
include response functions for all waveforms and must be readable by
rdseed v5.1 IRIS program.
The archive itself is a file tree system organized by the year and months.
The root directory, say SEED, includes subdirectory with the years, and
each year includes subdirectory with number of months. See an example
of archive in SEED directory of the distribution set. It includes only two
months of data, Apr 2005 and May 2005. Contents of testing archive is listed
also in APPENDIX A.
The name of SEED file is form of D.yyyy.ddd.M.D.xxxx....
where
    yyyy - year;
    ddd  - day from the beginning of year, starting from 1;
    M    - number of month, one or two characters;
    D    - day in month,one or two characters.

The fragment of typical IRIS "BREAKFAST" waveform request is shown below:

     ......
     ......
.LABEL: D.2005.106.4.16
     ......
     ......
WCN   NN 2005 04 16 00 00 00.0  2005 04 17 00 00 00.0 1 LHZ
109C  TA 2005 04 16 00 00 00.0  2005 04 17 00 00 00.0 1 LHZ
A04A  TA 2005 04 16 00 00 00.0  2005 04 17 00 00 00.0 1 LHZ
     ......
     ......

3.2 DATA PROCESSING
    ===============

When the seismic waveform data archive is ready we may start the data
processing. There are two main procedures.
The first one provides creating one or more monthly cross-correlation database
using tshell script cv_do_CO.csh. The second one provides merging predefined
set of monthly cross-correlation databases into single one (script
do_merge_cor.sh). The total scheme of getting the final cross-correlation
database might be chosen in different ways. For example, at first we do
yearly databases by merging monthly databases, and later do final database
by merging yearly databases.

3.2.1 cv_do_CO.csh program (tcsh script)
      ==================================

Script cv_do_CO.csh is the main procedure which organizes work spaces and
sequential run of five programs written on gcc/g77: cv_sa_from_seed_holes_RESP,
cut_trans_RESP, filter4, whiten_rej_phamp, and justCOR_mv_dir.
cv_do_CO.csh is called from the shell command line as:

...> cv_do_CO.csh year b_month e_month

where,
    year    - processing year;
    b_month - starting month, 1 <= b_month <= 12;
    e_month - ending month,   1 <= e_month <= 12, b_month <= e_month.

For example, to start test example provided in distribution set, go to
TEST2 directory and run it as

...> cd TEST2
...> cv_do_CO.csh 2005 4 5 >& test.log &

As a result script will create two monthly databases for April 2005 and
May 2005.The contents of TEST2 directory should be very close to the
precompiled results in TEST1 directory.
When cv_do_CO.csh starts it immediately creates the work space work1 and
the working directory year/b_month for the program cv_sa_from_seed_holes_RESP.
Also, script prepares input data for the cv_sa_from_seed_holes_RESP
program, namely: creates year/{b_month}_1 directory and copies data of the
first month b_month from archive to year/{b_month}_1 directory, creates
event list (references to daily SEED volumes files) in the working directory
year/b_month, extracts station information from monthly data into shortcut
station list file station.lst and stores it under year/b_month and
year/{b_month}_1 directories, starts cv_sa_from_seed_holes_RESP program.

3.2.1.1 cv_sa_from_seed_holes_RESP program
        ==================================

cv_sa_from_seed_holes_RESP produces from SEED volumes daily waveform segments
(events) for all given days and stations as a set of binary SAC files
(one SAC file per fixed day and station), and retrieve corresponding response
information (ASCII files) in the evalresp compatible format.
cv_sa_from_seed_holes_RESP provides the following additional functionalities:
  - merge multiple waveform segments into single waveform segment
  - analyse possible data gaps and reject data with total gaps exceeding
    treshold value.
  - linearly interpolate gaps
  - create and store on a disk auxiliary Reference Table (RT) that keeps
    events/stations/waveforms parameters and references to waveform
    locations on a disk.
  - analyse and store for further correction in RT possible fraction of
    waveform time, if sample epoch time is not multiple to the sampling step

Usage
=====

    ...>  cv_sa_from_seed_holes_RESP LHZ theshold_gap

where
     LHZ          - channel name, fixed value
     theshold_gap - theshold_gap*100 is maximum allowed data gap
                    in waveforms. Recommended value 0.1

cv_sa_from_seed_holes_RESP must be started in year/b_month directory.

Input data
==========
a) File with the fixed name station.lst . The station.lst file is a plain
   ASCII file given in a tabular form. The file includes shortcut information
   about seismic station. Each line consists of four fields separated by one
   or more spaces: network, sta, lon and lat.
   Here,
         network - two character name of a seismic network
         sta     - seismic station name, up to 6 characters
         lon     - geographic longitude in degrees
         lat     - geographic latitude in degrees
   Format: Unformatted.

Example:

CI   TUQ     -115.923900  35.435800
CI   VES     -119.084700  35.840900
IU   TUC     -110.784700  32.309800
TA   109C    -117.105100  32.888900
TA   A04A    -122.707000  48.719700

b) File with the fixed name input_ev_seed The input_ev_seed file is a plain
   ASCII file given in a tabular form. The file includes information about
   location of a SEED volume file for a given year, month and day.
   The location of a SEED volume is described by two sequential lines.

Line 1: ind, year, month, day, comments
   ind   - fixed text string " PDE", must be placed from the first position
           in line. The first symbol in line must be space.
   year  - four digits year
   month - number of the month of the year
   day   - number of the day of the month
   comments - any text
Format: Unformatted
All fields must be separated by one or more spaces.

Line 2: path
    path - path to the SEED volume file. Text must start from the first
           position in a line.

Example:

 PDE   2005    4 2  0000000000000     63.52 -147.44  11 8.50 Ms GS   9C.G F.S.
../4_in/ALL_2005_4_2
 PDE   2005    4 3  0000000000000     63.52 -147.44  11 8.50 Ms GS   9C.G F.S.
../4_in/ALL_2005_4_3
 PDE   2005    4 4  0000000000000     63.52 -147.44  11 8.50 Ms GS   9C.G F.S.
../4_in/ALL_2005_4_4

Output Data
==========
Waveform directories. For each day cv_sa_from_seed_holes_RESP creates in
the working directory subdirectory form of yyyy_M_D_0_0_0 to store one day
waveforms in binary SAC format (like CI.GRA.LHZ.SAC) and corresponding
instrument response files (like RESP.CI.GRA..LHZ). yyyy M and D are year,
month and day of all data stored in yyyy_M_D_0_0_0 directory.

File sac_db.out. The file sac_db.out is the binary dump of final state of the
auxiliary RT on a disk.

File event_station.tbl. The file event_station.tbl is the ASCII dump of some
fields related to auxiliary RT. This is records date, station name, path
to SAC file, complete start waveform time t0 with possible global time
shift frac in sec, and number of samples.
Example.

2005_4_30_0_0_0  Y22C  2005_4_30_0_0_0/TA.Y22C.LHZ.SAC  t0: 2005/120:0:0:0 frac: 0 s 86401 s of record
2005_4_30_0_0_0  BOZ  2005_4_30_0_0_0/US.BOZ.LHZ.SAC  t0: 2005/120:0:0:1 frac: -0.171 s 86400 s of record
2005_4_30_0_0_0  BW06  2005_4_30_0_0_0/US.BW06.LHZ.SAC  t0: 2005/120:0:0:1 frac: -0.164 s 86400 s of record

3.2.1.2 cut_trans_RESP program
        ======================
When cv_sa_from_seed_holes_RESP is finished cv_do_CO.csh shell script
starts another program cut_trans_RESP on the same working directory.
cut_trans_RESP program removes the mean and the trend from waveforms,
and provides waveform correction for the instrument response by SAC evalresp
function, broadband filtering, and cutting desired segment of data in
the given time range.
cut_trans_RESP writes the global time shift from auxiliary RT into
the header of each SAC output file, field user1.

Usage
=====
     ...> cut_trans_RESP T1 T2 T3 T4 t1 npts
where,
     T1 T2 T3 T4 - corner periods of broadband pass filter in seconds.
                   Corner periods are real numbers, and T1 > T2 > T3 > T4 > 0
                   Corresponding corner frequencies are 1/T1, 1/T1, 1/T1, 1/T4.
     t1          - skip points from the beginning of the waveform up to time
                   t1, where t1 is time in seconds from the beginning of
                   the day. t1 is non negative integer number (t1 >= 0).
     npts        - kept npts seconds in waveform after skipping.
                   npts is positive integer number (npts > 0).

Example

       ...> cut_trans_RESP 170.0 150.0 5.0 4.0 1000 83000

Input/Output data
=================

cut_trans_RESP uses input data from daily directories that had been
created by the previous program cv_sa_from_seed_holes_RESP and stores output
in the same directories, but with different name. To each waveform name
program adds prefix ft_ .
For example, if the file name was AZ.MONP.LHZ.SAC , it is stored
after processing under name ft_AZ.MONP.LHZ.SAC .
cut_trans_RESP  also uses auxiliary RT (read only).

3.2.1.3 filter4 and whiten_rej_phamp programs
        =====================================

filter4 and whiten_rej_phamp programs apply a set of data processing
(filtering) procedures over all individual SAC binary waveforms with the
names ft_xxxxx that were output by cut_trans_RESP program.
Note, that filter4, whiten_rej_phamp and justCOR_mv_dir programs run
in different work space work2 with the new working directory
year/b_month/5to150. To do that main script cv_do_CO.csh creates new
subdirectory 5to150 in current working directory year/b_month and will start
filter4, whiten_rej_phamp and justCOR_mv_dir programs in new working
directory year/b_month/5to150. Script copies station.lst and sac_db.out table
in a new working directory, and runs loop by days. In each loop cycle script
creates a daily waveforms directory yyyy_M_D_0_0_0 and fills out
with ft_xxxx.SAC waveforms from work space work1. After that the script goes
to yyyy_M_D_0_0_0 directory, runs filter4, runs whiten_rej_phamp program,
and returns to working directory.

Let us describe how to run filter4 and whiten_rej_phamp programs.
a) The program filter4 applies broadband filter and the global time
shift correction to SAC waveform files updating it in place.

Usage
=====
    ...> filter4 parameter_file

    The parameter_files ASCII plane file, each line includes the
    following fields separated one or more spaces: T1,T2,T3,T4,dt,npow,name
    where,
    T1,T2,T3,T4 - corner periods in seconds, T1>T2>T3>T4>0, real
    dt          - sampling step in seconds, real
    npow        - power of cosine ends tapering, integer
    name_file   - the name of input/output ASCII binary SAC waveform
                  file in working directory.

Input/Output
============
For each line from the parameter_file filter4 read file with name_file
(ft_xxxx.SAC), makes filtering according to the line parameters, makes
the global time shift correction, and stores result as binary SAC file
by replacing input file by the new one.

b) Another program whiten_rej_phamp applies three data processing procedures
   to every individual files obtained by filter4 program:
   - temporal normalization or one-bit normalization
   - spectral whitening
   - notch correction for 26 sec period

Usage
=====
    ...>  whiten_rej_phamp parameter_file

    The parameter_files ASCII plane file, each line includes the
    following fields separated by one or more spaces:
    T1,T2,T3,T4,dt,npow,nwt,tnorm,fr1,fr2,nsmooth,onebit,patch,freqmin,name
    where,
    T1,T2,T3,T4 - corner periods in seconds, T1>T2>T3>T4>0, spectral whitening,
                   real
    dt          - sampling step in seconds, real,
                  for spectral whitening
    npow        - power of cosine ends tapering, integer,
                  for spectral whitening
    nwt         - half width of smoothing window in samples, integer,
                  for spectral whitening
    tnorm       - one character "Y" or "N", to apply or not apply temporal
                  normalization
    fr1, fr2    - corner frequencies of temporal normalization in Hz , real,
                  used if tnorm = Y
    nsmooth     - half width of smoothing window of temporal normalization
                  in samples, integer, used if tnorm = Y
    onebit      - one character "Y" or "N", to apply or not apply one-bit
                  normalization. We don't recommend to use one-bit
                  normalization.
    notch       - one character "Y" or "N", to apply or not apply notch
                  correction for 26 sec period. Be sure that this effect
                  exists at your area of investigation.
    freqmin     - spectral amplitude damping for the notch correction,
                  0 < freqmin < 1.0. Smaller value of freqmin means stronger
                  damping. freqmin = 0.5 is recommended value.
    name_file   - the name of input ASCII binary SAC waveform file in working
                  directory.
Input/Output
============
For each line whiten_rej_phamp reads name_file (ft_xxxx.SAC ) makes
data processing and stores temporary normalized waveform with the name of
input file. Also, it creates and stores whitened spectra files as a binary
SAC files type of time series files. Real part of spectra has name
ft_xxxx.SAC.am and imaginary part has a name ft_xxxx.SAC.ph.
Example.
    Input file:   ft_TA.N06A.LHZ.SAC
    Output files: ft_TA.N06A.LHZ.SAC  ft_TA.N06A.LHZ.SAC.am
                  ft_TA.N06A.LHZ.SAC.ph

3.2.1.4 justCOR_mv_dir program
        ======================

justCOR_mv_dir is the last program running under cv_do_CO.csh shell script.
The program read auxiliary RT, creates station pairs, computes for
station pairs daily cross-correlations, and, finally, makes monthly
cross-correlation stacking to produce cross-correlation waveform files in
binary SAC format. The resulting cross-correlation data base
is stored under subdirectory COR.

Usage
=====
     ...> justCOR_mv_dir nlag ntps

     where nlag is cross-correlation lag. The total size of cross-correlation
     is 2*nlag+1 and ntps is number of samples per day.

Input
====
    - auxiliary RT sac_db.out
    - directories yyyy_M_D_0_0_0 with spectral information, files
      ft_xxxx.SAC.am and ft_xxxx.SAC.ph
Output
=====
Monthly cross-correlation waveform files data base.
For more details see The Ambient Noise Cross-Correlation Data Product,
User's Manual, Section 5.

3.2.2 MERGING DATABASES

The second procedure. Shell scripts do_merge_cor.sh together with program
do_merge_cor.sh provides merging/stacking of predefined set of
cross-correlation databases into single one. The procedure searches the same
pair of stations over all monthly databases and makes summation of
corresponding waveforms (stacking). All databases have the same organization,
so, it is possible to merge monthly or yearly databases od some mix of monthly
and yearly. The most important things, It should not be data duplications
during merging process. It leads to growing systematic errors.

a) Shell script do_merge_cor.sh
   ============================

Usage
=====
    ...> do_merge_cor.sh param_file dir_cor file_cor

were,
param_file - ASCII plain file. Each line of param_file is absolute
             or relative path to input database
dir_cor    - absolute or relative path to the directory where the merged
             database will be resided
file_cor    - name of merged database

Input/Output
============
do_merge_cor.sh shell script assigns integer number to each database
and creates the path_list file for merge_cor program. Each line of
this file identifies the single cross-correlation and includes two fields
the number of database and path to single cross-correlation.
do_merge_cor.sh forms joined station list and stores it under dir_cor
directory.

b) merge_cor program
   =================
Usage
=====
    ...> merge_cor path_list path_to_base
where
path_list     - ASCII flat file described above
path_to_base  - path to merged database, usually it passed from do_merge_cor.sh
                script and path_to_base = dir_cor/file_cor
Input/Output
According to path_list merge_cor program provides stacking and
output merged file.

3.2.3 POSTPROCESSING
      ==============
When you get the final set of cross-correlations, you may apply
three postprocessing procedures: sac_update.sh shell script and
mkccwfdisc, add_num programs. The first one, sac_update.sh updates SAC
header for each cross-correlation waveform using standard sac program.
This is necessary to do, because, ancc package uses non standard
software for processing SAC binary files and SAC header doesn't include
some fields lile distance, back azimuth etc. You have to run
mkccwfdisc and add_num progams if you want to create ASCII
Cross-correlation descriptor table, see [1] for more details.
Of course, if you need, it is possible to apply postprocessing fr any
intermediate cross-correlation database.

sac_update.sh shell script.
Usage
=====
    ...> sac_update.sh dir_db name_db
Here,
dir_db  - directory containing cross-correlations database;
name_db - directory, the name of database

mkccwfdisc program

mkccwfdisc program creates Cross-correlation descriptor table.
Usage
=====
    ...> mkccwfdisc  dir_db name_db > file_descriptor

dir_db and name_db are the same as for sac_update.sh script.
mkccwfdisc outputs descriptor table on stdout, you have to redirect
program output to pipe or file.

add_num program

add_num numerates descriptor table file (field wfid) starting from 1 by 1.
Usage
=====
    ...> add_num file_descriptor dir_db name_db

See an example of postprocessing in the RUN script under TEST1 directory.

REFERENCES
=========

[1] The Ambient Noise Cross-Correlation Data Product, User's Manual.
    See. CCManual.pdf in doc directory.
[2] Bensen, G.D., M.H. Ritzwoller, M.P. Barmin, A.L. Levshin, F. Lin,
    M.P. Moschetti, N.M. Shapiro, and Y. Yang, Processing seismic ambient
    noise data to obtain reliable broad-band surface wave dispersion
    measurements, Geophys. J. Int., 169, 1239-1260,
    doi: 10.1111/j.1365-246X.2007.03374.x, 2007.

APPENDIX A
==========
[ancc-1.0-0.src]$ ls -R SEED
SEED:
2005  2006

SEED/2005:
4  5

SEED/2005/4:
D.2005.091.4.1.11171.seed   D.2005.106.4.16.15424.seed
D.2005.092.4.2.2442.seed    D.2005.107.4.17.10511.seed
D.2005.093.4.3.19257.seed   D.2005.108.4.18.2421.seed
D.2005.094.4.4.29868.seed   D.2005.109.4.19.22142.seed
D.2005.095.4.5.26776.seed   D.2005.110.4.20.18199.seed
D.2005.096.4.6.12310.seed   D.2005.111.4.21.25227.seed
D.2005.097.4.7.3930.seed    D.2005.112.4.22.7919.seed
D.2005.098.4.8.6479.seed    D.2005.113.4.23.7979.seed
D.2005.099.4.9.6539.seed    D.2005.114.4.24.7861.seed
D.2005.100.4.10.8461.seed   D.2005.115.4.25.28334.seed
D.2005.101.4.11.9712.seed   D.2005.116.4.26.5552.seed
D.2005.102.4.12.8403.seed   D.2005.117.4.27.17178.seed
D.2005.103.4.13.10375.seed  D.2005.118.4.28.13750.seed
D.2005.104.4.14.23676.seed  D.2005.119.4.29.3657.seed
D.2005.105.4.15.20581.seed  D.2005.120.4.30.5022.seed

SEED/2005/5:
D.2005.121.5.1.22097.seed   D.2005.137.5.17.5296.seed
D.2005.122.5.2.20254.seed   D.2005.138.5.18.14463.seed
D.2005.123.5.3.20308.seed   D.2005.139.5.19.23614.seed
D.2005.124.5.4.27359.seed   D.2005.140.5.20.25716.seed
D.2005.125.5.5.93.seed      D.2005.141.5.21.18220.seed
D.2005.126.5.6.16013.seed   D.2005.142.5.22.18345.seed
D.2005.127.5.7.10092.seed   D.2005.143.5.23.29828.seed
D.2005.128.5.8.10023.seed   D.2005.144.5.24.27333.seed
D.2005.129.5.9.11343.seed   D.2005.145.5.25.21129.seed
D.2005.130.5.10.8515.seed   D.2005.146.5.26.28384.seed
D.2005.131.5.11.6006.seed   D.2005.147.5.27.22149.seed
D.2005.132.5.12.17548.seed  D.2005.148.5.28.25169.seed
D.2005.133.5.13.2510.seed   D.2005.149.5.29.23233.seed
D.2005.134.5.14.1433.seed   D.2005.150.5.30.15941.seed
D.2005.135.5.15.13452.seed  D.2005.151.5.31.4719.seed
D.2005.136.5.16.14225.seed

SEED/2006:
1  10  11  12  2  3  4  5  6  7  8  9

SEED/2006/1:

SEED/2006/10:

SEED/2006/11:

SEED/2006/12:

SEED/2006/2:

SEED/2006/3:

SEED/2006/4:

SEED/2006/5:

SEED/2006/6:

SEED/2006/7:

SEED/2006/8:

SEED/2006/9:
