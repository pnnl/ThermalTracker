#!/bin/bash

# Extract features, blobs, positions from processed video in data dir. 
# If any features/blobs/positions output files already exist,
# they will be deleted.
bindir=../build
datadir=../data
logdir=../results

datetime=$(date +"%Y-%m-%d_%H%M%S")
featureslog="$logdir/trackfeatures_$datetime.log"
echo -n "svnversion " > $featureslog
#svnversion >> $featureslog

for vfile in "$datadir"/*.mp4
do
    echo
	echo $vfile
	# delete old files
	nchars=$((${#vfile}-4))
	outdir=${vfile:0:$nchars}_out # output dir for vfile 
	echo "cleaning $outdir"
	rm $outdir/*_trks.png # output from extracttracks
	rm $outdir/*_trk????blob???.png # output from trackblobs
	rm $outdir/*-blobs_????.csv # output from trackblobs
	# run feature extractors
	$bindir/trackfeatures -i $vfile | tee -a $featureslog
	$bindir/trackblobs -i $vfile 
	$bindir/trackpositions -i $vfile 
	
done

exit
