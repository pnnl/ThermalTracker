#!/bin/bash
# Process videos in the data dir from start to finish.

# assume we are in the scripts dir
bindir=../build/tracker
datadir=../sample_video
logdir=../build

datetime=$(date +"%Y-%m-%d_%H%M%S")
processlog="$logdir/processvideo_$datetime.log"
extractlog="$logdir/extracttracks_$datetime.log"
featureslog="$logdir/trackfeatures_$datetime.log"

for vfile in "$datadir"/*.mp4
do
	echo $vfile
	echo
	echo "Step 1:  ProcessVideo"
	$bindir/processvideo -i $vfile -w 10 | tee -a $processlog
	
	echo
	echo "Step 2:  ExtractTracks"
	$bindir/extracttracks -i $vfile | tee -a $extractlog
	
	echo
	echo "Step 3:  TrackFeatures"
	$bindir/trackfeatures -i $vfile | tee -a $featureslog
        $bindir/trackpositions -i $vfile
        $bindir/trackblobs -i $vfile
done

exit
