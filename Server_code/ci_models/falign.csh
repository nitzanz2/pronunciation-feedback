#!/bin/csh -f

if ($?PBS_O_WORKDIR) then
    cd $PBS_O_WORKDIR
endif

if ($#argv != 2) then
    echo "USAGE: $0  part npart"
    exit 0
endif

set part = $1
set npart = $2


set basedir = ./

set listbasedir = ./
set ctlfn = XXX.ctl
set inlsnfn = XXX.trans

set dictfn = XXX.dict
set fillerdictfn = XXX.fillerdict

set cepdir = path/mfc/
set cepext = 80-7200_40filts.mfc
set name = somename

if (! -e $basedir/falignout) mkdir $basedir/falignout
set outlsnfn = $basedir/falignout/$name.faligned-$part.trans
set outctlfn = $basedir/falignout/$name.faligned-$part.ctl
set logfn = $basedir/falignout/$name.faligned-$part.log

#ci models are best for falignment to segment out phonemes and words
set modeldir = ./ads.80-7200-40f.1-3.ci_continuous.8gau
set mdeffn = ./ads.80-7200-40f.1-3.ci.mdef

set nlines = `wc $ctlfn | awk '{print $1}'`
set ctloffset ctlcount
@ ctloffset = ( ( $nlines * ( $part - 1 ) ) / $npart )
@ ctlcount = ( ( $nlines * $part ) / $npart ) - $ctloffset
echo "Doing $ctlcount segments starting at number $ctloffset"

#goto a
set phsegdir = phonesegs
#Make stateseg directories
foreach x (`cat $ctlfn | awk '{print $1;}'`)
   set d = $x:h
if(! -e $phsegdir/$d)   mkdir -p $phsegdir/$d
end
a:

~/src/s3decoder/decoderlatest/bin/linux/s3align \
-logbase 1.0001  \
-mdeffn $mdeffn \
-senmgaufn .cont.  \
-meanfn $modeldir/means.meskimen2.multiclass \
-varfn $modeldir/variances \
-mixwfn $modeldir/mixture_weights \
-tmatfn $modeldir/transition_matrices \
-feat 1s_c_d_dd \
-topn 32  \
-beam 1e-80  \
-dictfn $dictfn \
-fdictfn $fillerdictfn \
-ctlfn $ctlfn \
-ctloffset $ctloffset \
-ctlcount  $ctlcount \
-cepdir $cepdir \
-cepext $cepext  \
-ceplen 13 \
-agc none  \
-cmn current \
-phsegdir $phsegdir,CTL \
#-stsegdir $phsegdir,CTL \
-wdsegdir $phsegdir,CTL \
-insentfn $inlsnfn \
-outsentfn $outlsnfn \
-outctlfn $outctlfn \
-logfn $logfn

# -phsegdir $phsegdir,CTL \
#>&! $logfn


exit 0


