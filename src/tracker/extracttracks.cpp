/*
 *  extracttracks.cpp
 *
 *  Created by Shari Matzner on 12/28/12.
 *  Copyright 2012 Pacific Northwest National Laboratory. All rights reserved.
 *
 */

#include <iostream> // cout, cin, cerr
#include <fstream>  // ofstream
#include <string>   // for strings
#include <iomanip>  // setw, right, left
#include <boost/unordered_set.hpp>

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "utilities.hpp"
#include "thermaltracker.hpp"
#include "vpsfile.hpp"
#include "trackfile.hpp"
#include "visualization.hpp"

using namespace std;
using boost::unordered_set;
using namespace cv;
using namespace boost;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

int extractTracks(Parameters params) {
  bool VIEW = params.view;
	// TO DO: Print out parameters
	
    //-------------------------------------------------------------------------------------
    // READ VPS FILE
    
    fs::path vfilepath(params.inputFile());

    string outpath;
    if (params.outputDir() != "") {
      outpath = params.outputDir() + vfilepath.stem().string();
    } else {
      generate_outpath_str(vfilepath.make_preferred().string(), outpath);
    }

    fs::path vpsfilepath(outpath + ".vps");
	ifstream vpsfile(vpsfilepath.string().c_str(), ios::in | ios::binary);
	if (!vpsfile.is_open())
	{
		cerr << "Error opening VPS file: " << vpsfilepath << endl;
		cerr << "Run ProcessVideo to generate vps file." << endl;
		return (-1);
	}
    
	cout << endl;
	cout << "reading file " << vpsfilepath << endl;
	
	VPS_FileHeader header;
	VPS_Data d;
	
	read_vps_file(vpsfile, header, d);
	vpsfile.close();
	
	cout << endl << header << endl;
	
    //-------------------------------------------------------------------------------------
    // DECLARATIONS
    
	unsigned int frameHeight = header.height;
	unsigned int frameWidth = header.width;
	double numPixels = frameHeight*frameWidth;
	int Nvps = header.Nvps;
    int halfFrames = header.vpsWindowFrames/2;
    
    WindowText wtxt(string(), Point2i(20,10), FONT_HERSHEY_PLAIN, 1);
    
    double 	maxDist = 0.5*sqrt(pow(params.fracImageX*frameWidth,2)+pow(params.fracImageY*frameHeight,2));
    cout << endl;
	cout << "max distance between groups in track is " << maxDist << " pixels" << endl;
    
    std::vector< std::vector<Track> >    vpsTracks(Nvps, std::vector<Track>());   // list of tracks in each VPS window
    int Ntrk = 0; // total number of tracks
	
	std::vector<Track> active_tracks;   // tracks that may have additional blobs added
	std::vector<Track> complete_tracks; // tracks no longer being updated
    int id = 0; // track id
	    
    //-------------------------------------------------------------------------------------
    // PROCESS VPS IMAGES
 	for (int iWin=0; iWin<Nvps; ++iWin)
	{
        cout << endl << "----------------------------" << endl;
		cout << "processing window " << iWin+1 << " of " << Nvps
             << ", from " << elapsedVideoTime(d.vpsFrameStart[iWin],header.fps)
             << " to " << elapsedVideoTime(d.vpsFrameEnd[iWin],header.fps)<< endl;
        
		Mat im((int)frameHeight, (int)frameWidth, CV_32FC1, &(d.vpsPeak[iWin][0])); // needed to store blob images
        //int firstNewFrame = d.vpsFrameStart[iWin] + halfFrames;
        int firstNewFrame = iWin>0 ? d.vpsFrameStart[iWin] + halfFrames : d.vpsFrameStart[iWin];
        if(params.view) VIEW = true; // reset VIEW flag
        
		//-------------------------------------------------------------------------------------
		// GROUP PIXELS INTO OBJECTS
		cout << "-----grouping pixels into objects-----" << endl;
		
		// Get a list of all the pixels above threshold.
        long Npix = d.pixListIdx[iWin].size();
		cout << "pixels above threshold: " << Npix << "(" << round(Npix/numPixels * 100.0) << "%)" << endl;

		// Statistics calculated as groups are formed.
        int maxW = 0, minW = frameWidth, maxH = 0, minH = frameHeight;
        double minI = 1;
        double maxI = 0;
		
		unordered_set<unsigned> ungroupedIdx(d.pixListIdx[iWin].begin(),d.pixListIdx[iWin].end());  // ungrouped bright pixels
		std::vector<char> ungrouped(d.vpsPeak[iWin].size(),1); 
		std::vector<unsigned>  ineighbors;     // list of pixel neighbors
		std::vector<Blob>      blobs;          // groups of pixels
		std::vector< std::vector<Point2i> >  blobcontours; // contours of groups
		int               igrp = 0;
		int               ipix;           // pixel index
		
        int border = 50;
		std::vector<PixelValue> vps(frameHeight*frameWidth); 
		vps.assign(d.vpsPeak[iWin].begin(),d.vpsPeak[iWin].end());
        
        Viewer v(frameHeight, frameWidth, &(vps[0]), border);
        if (VIEW)
		{
 			stringstream ss;
			ss << "Window " << iWin;
            wtxt.text = ss.str();
            v.ShowImage(wtxt, 0);
 		}
		
		while (!ungroupedIdx.empty()) // while ungrouped pixels
		{
			// start a new group            
			stack<unsigned>   pixReady;  // set of candidate pixels
			std::vector<unsigned>  groupPix;  // pixels in group
			
			// Pick an initial pixel, mark it as found and put it in the Ready list.
			pixReady.push(*ungroupedIdx.begin());
			ungroupedIdx.erase(ungroupedIdx.begin());

            
            // TO DO:  check frame here to limit search to new blobs.
			
			if (VIEW)
			{
				stringstream ss;
				ss << "Window " << iWin << ": " << elapsedVideoTime(d.vpsFrameStart[iWin],header.fps) << " to " << elapsedVideoTime(d.vpsFrameEnd[iWin],header.fps);
                wtxt.text = ss.str();
                v.Refresh();
                v.ShowZoom(pixReady.top(), 5, wtxt,10);
                
			}
			
			while (!pixReady.empty())
			{
				ipix = pixReady.top();  // pick a pixel in the Ready list
				pixReady.pop();         // remove it from Ready
				
				if (VIEW)
                {
                    // show current pixel
                    v.ShowPixelZoom(ipix, CV_RGB(0,0,255), 100);
                }
                
				// add it to the current group
				groupPix.push_back(ipix);
				ungrouped[ipix] = 0; // mark as grouped
                
				// find all its neighbors, spatial and temporal
				getNeighbors(ipix,frameHeight,frameWidth,ineighbors);
				
				for (int j=0;j<ineighbors.size();j++)
				{
                    if (d.vpsPeak[iWin][ineighbors[j]]>0 && d.vpsPkFrame[iWin][ineighbors[j]]==d.vpsPkFrame[iWin][ipix])
					{
						// mark neighbor as found
						if (ungrouped[ineighbors[j]]>0)
                        {
							ungroupedIdx.erase(ineighbors[j]);
                            // add neighbor to the Ready list
                            pixReady.push(ineighbors[j]);
                            if (VIEW)
                            {
                                // show neighbor pixel
                                v.ShowPixelZoom(ipix, CV_RGB(0,0,255), 100);
                            } // if VIEW
                        } // if ungrouped
					} // if neighbor valid
				} // for each neighbor
                if (VIEW)
                {
                    // show current pixel in group
                    int c = v.ShowPixelZoom(ipix, CV_RGB(0,255,0), 0);
					if ( (char)c == 27 ) return 0; // exit
					if ( (char)c == 'g' || (char)c == 'G' ) VIEW=false; // continue without viewing
                } // if VIEW
  				                
			} // pixels ready
			// Only keep groups greater than minimum size
            if (groupPix.size() >= params.minObjectPix)
			{
				igrp++;
				//cout << "new group " << igrp << " with " << groupPix.size() << " pixels" << endl;
                /*
				for (int j=0;j<groupPix.size();++j)
				{
					Point2i pnt = idx2pnt(groupPix[j],frameWidth);
 				}
                */
                Blob newblob(d.vpsPkFrame[iWin][groupPix[0]] + d.vpsFrameStart[iWin], // frame number
                             groupPix, (Mat_<PixelValue>)im);
				// TO DO:  This is kludgy.  Fix blob contructor.
				if (!newblob.contour.empty())
				{
					blobs.push_back(newblob);
					blobcontours.push_back(newblob.contour);
				}
				
                maxW = std::max(maxW, newblob.rect.width);
                minW = std::min(minW, newblob.rect.width);
                maxH = std::max(maxH, newblob.rect.height);
                minH = std::min(minH, newblob.rect.height);
				maxI = std::max(maxI, newblob.meanI);
				minI = std::min(minI, newblob.meanI);
			}
			/*
            else
            {
                for (int j=0;j<groupPix.size();++j)
                {
					bw.at(groupPix[j]) = 0;
                }
            }
			*/
		} // ungrouped pixels
		
		//-------------------------------------------------------------------------------------
        if(params.view) VIEW = true; // reset VIEW flag
		
		if (VIEW)
		{
			// show groups
            v.HideZoom();
            int c = v.ShowContours(blobcontours, CV_RGB(0,128,0), 0);
			if ( (char)c == 27 ) return 0; // exit
			if ( (char)c == 'g' || (char)c == 'G' ) VIEW=false; // continue without viewing
		}
		cout << endl << "calculating object statistics" << endl;
        cout << "intensity ranges from " << minI << " to " << maxI << endl;
		cout << "blob width ranges from " << minW << " to " << maxW << endl;
		cout << "blob height ranges from " << minH << " to " << maxH << endl;
		
		//-------------------------------------------------------------------------------------
		// FORM TRACKS
		cout << endl << "-----forming tracks-----" << endl;
        
        // de-activate tracks that are old
        std::vector<Track>::iterator iter = active_tracks.begin();
        //cout << "checking active tracks:  firstNewFrame = " << firstNewFrame << ", maxFrameDiff = " << params.maxFrameDiff << endl;
        while ( iter != active_tracks.end() )
        {
            //if ( active_tracks[k].frameEnd() < (firstNewFrame - p.maxFrameDiff) )
            if ( iter->frameEnd() < (firstNewFrame - params.maxFrameDiff) )
            {
                //cout << "deactivating track id " << iter->id() << " with last frame " << iter->frameEnd() << endl;
                if (iter->nblobs() >= params.minTrackObjects)
                    complete_tracks.push_back(*iter);
                iter = active_tracks.erase(iter);
            }
            else
                ++iter;
        }
       //FrameNumber curFrame; // frame number of current step in track
 		v.Refresh();
        
		// 1. Mark all blobs as not assigned.
        // sort blobs from latest to earliest
        std::vector<size_t> blobSortedIndex = sortedIndexDescending<Blob>(blobs); // indices of sorted blobs
        
        while ( !blobSortedIndex.empty() ) //
		{
			// get the next blob
            igrp = blobSortedIndex.back();
            blobSortedIndex.pop_back();
			if ( blobs[igrp].frame < firstNewFrame ) 
				continue;
            FrameNumber curFrame = blobs[igrp].frame;
            //cout << endl << "current blob is " << igrp << ", current frame is " << curFrame << endl;
				
            Point2i center = Point2i(blobs[igrp].rect.x + blobs[igrp].centroidx, blobs[igrp].rect.y + blobs[igrp].centroidy);
                 
                if (VIEW)
                {
					stringstream ss;
					ss << "Window " << iWin << ": " << elapsedVideoTime(d.vpsFrameStart[iWin],header.fps) << " to " << elapsedVideoTime(d.vpsFrameEnd[iWin],header.fps);
                     wtxt.text = ss.str();
                     v.ShowZoom(center, 5, wtxt, 1);
                     // show current blob
                    v.ShowContourZoom(blobs[igrp].contour, CV_RGB(0,0,255),1);
                    int c = v.ShowBlobZoom(blobs[igrp].pixIdx, CV_RGB(0,0,255));
                    if ( (char)c == 27 ) return 0; // exit
                    if ( (char)c == 'g' || (char)c == 'G' ) VIEW=false; // continue without viewing
                }
				
				// find candidate tracks to assign blob
                if ( active_tracks.empty() )
                {
                    // start a new track
                    //cout << "starting new track" << endl;
                    ++id;
                    active_tracks.push_back(Track(id,blobs[igrp]));
                    continue;
                }
                int Nactive = active_tracks.size();
                //cout << Nactive << " active tracks" << endl;
				std::vector<size_t> candidatesIdx;
				std::vector<double> blobDist;      // distance of blob from last blob in track
                std::vector<int> blobFrameDiff;    // difference in frames from last blob in track
                std::vector<double> spaceTimeDist; // total distance, spacial and temporal
				std::vector<size_t>::iterator it = blobSortedIndex.end()-1;
            
                //cout << "distance from tracks: " << endl;
                for (int k=0; k<Nactive; ++k)
				{
                    //cout << "    id " << active_tracks[k].id() << "frameEnd=" << active_tracks[k].frameEnd();
                    if ( (active_tracks[k].frameEnd() <= curFrame)
                        && (curFrame < (active_tracks[k].frameEnd() + params.maxFrameDiff)) )
					{
						// check distance
						double dist = maxDist;
                        //for (int j=0;j<active_tracks[k].blobs.back().contour.size();++j)
                        for (int j=0;j<active_tracks[k].back().contour.size();++j)
                        {
							// positive (inside), negative (outside), zero (on contour)
							dist = std::min(dist,
                                        -pointPolygonTest(blobs[igrp].contour, active_tracks[k].back().contour[j],true));
                            // NOTE:  Ignoring overlap (negative distance) is bad.  Overlap blobs should be preferred.
                        } // for each contour point
                        //cout << " dist=" << dist;
                       
						if (dist < maxDist)
						{
							candidatesIdx.push_back(k);
							blobDist.push_back(dist);
                            blobFrameDiff.push_back(curFrame-active_tracks[k].frameEnd());
                            spaceTimeDist.push_back(dist/maxDist + (double)blobFrameDiff.back()/(double)params.maxFrameDiff);
                           // cout << "    id " << active_tracks[k].id() << "," << blobDist.back() << "," << blobFrameDiff.back() << "," << spaceTimeDist.back() << endl;
                           // cout << " score=" << spaceTimeDist.back();
						} // dist < maxDist
					} // frameDiff < maxFrameDiff
                    cout << endl;
				} // for each active track
                cout << endl;
				int Nfound = candidatesIdx.size();
				//cout << "found " << Nfound << " candidate tracks" << endl;
                
				if (Nfound==0)
                {
                    // start a new track
                   // cout << "starting new track id " << (id+1) << endl;
                    ++id;
                    active_tracks.push_back(Track(id, blobs[igrp]));
                    continue;
                }
            
                // index of closest track
                int minIdx = distance(spaceTimeDist.begin(),min_element(spaceTimeDist.begin(),spaceTimeDist.end()));
                //cout << "best match is candidate " << minIdx << " with score " << spaceTimeDist[minIdx] << endl;
				//cout << "best match is id " << active_tracks[candidatesIdx[minIdx]].id() << ", with score " << spaceTimeDist[minIdx] << endl;
				// add to track
				active_tracks[candidatesIdx[minIdx]].push_back(blobs[igrp]);
                if (VIEW)
                {
                    // show best match
                    int c = v.ShowTrack(active_tracks[candidatesIdx[minIdx]], CV_RGB(128,0,255));
                    if ( (char)c == 27 ) return 0; // exit
                    if ( (char)c == 'g' || (char)c == 'G' ) VIEW=false; // continue without viewing
                } // if VIEW

 		} // not all blobs assigned
        if(params.view) VIEW = true; // reset VIEW flag
		cout << "all blobs assigned to tracks" << endl;
		cout << active_tracks.size() << " active tracks" << endl;
		cout << complete_tracks.size() << " complete tracks" << endl;
		  
		// save vps window image with tracks labeled
		
		stringstream ss;
		Size txtsize;
		ss << "Window " << iWin << ": " << elapsedVideoTime(d.vpsFrameStart[iWin],header.fps) << " to " << elapsedVideoTime(d.vpsFrameEnd[iWin],header.fps);
		wtxt.text = ss.str();    
		
		Mat imc_out;
		std::vector<Track> all_tracks;
		for (int k=0; k<complete_tracks.size(); ++k)
			if (complete_tracks[k].frameEnd() > d.vpsFrameStart[iWin])
				all_tracks.push_back(complete_tracks[k]);		
		for (int k=0; k<active_tracks.size(); ++k)
            if (active_tracks[k].nblobs() >= params.minTrackObjects)
				all_tracks.push_back(active_tracks[k]);
			
		if (0<all_tracks.size()) {
			v.MakeOutputImage(imc_out, wtxt, all_tracks, d.vpsFrameStart[iWin], d.vpsFrameEnd[iWin]);
            std::vector< std::vector<Point2i> >  trackblobs;
            for (int k=0; k<all_tracks.size(); ++k)
            {
                for (int j=0; j<all_tracks[k].nblobs(); ++j)
                {
                    trackblobs.push_back(all_tracks[k].blobs[j].contour);
                }
            }
            drawContours(imc_out, trackblobs, -1, CV_RGB(0,128,0), 1, 8, noArray(), INT_MAX, Point2i(0, 0));
			string fname = outpath + "_" + elapsedVideoTime2(d.vpsFrameStart[iWin],header.fps) + "_trks.png"; 
			imwrite(fname,imc_out);
		}
		/*
		if (VIEW) 
		{
			//imshow(WIN_ZOOM,imcb);
			imshow(WIN_VPS,imc);
			int c = waitKey(0);
			if ( (char)c == 27 ) return 0; // exit
			if ( (char)c == 'g' || (char)c == 'G' ) VIEW=false; // continue without viewing
		} // VIEW
		*/


	} // for each vps window
	
    
    for (int k=0; k<active_tracks.size(); ++k)
        if (active_tracks[k].nblobs() >= params.minTrackObjects)
			complete_tracks.push_back(active_tracks[k]);
    
	//-------------------------------------------------------------------------------------
	// SAVE RESULTS
	filesystem::path outfilepath(vpsfilepath);
	outfilepath.replace_extension(".trk");
	cout << "saving data to " << outfilepath << endl;
	
	TRK_FileHeader trkheader;
    memcpy(trkheader.videoName,header.videoName,sizeof(trkheader.videoName));
    trkheader.width = header.width;
    trkheader.height = header.height;
    trkheader.fps = header.fps;
    trkheader.params = params;
	trkheader.framesperwindow = header.vpsWindowFrames;
	trkheader.Ntrk = complete_tracks.size();
	
	TRK_Data trkdata(complete_tracks);
	
	ofstream outfile(outfilepath.string().c_str(),ios::out | ios::binary);
	if (!outfile.is_open())
	{
		cerr << "Error opening output file: " << outfilepath.string() << endl;
		return (-1);
	}
    write_trackfile(outfile, trkheader, trkdata);
	outfile.close();
	
	// for testing
	cout << endl << "Saved to TRK file:" << endl;
	cout << trkheader << endl;
	for (int k=0;k<complete_tracks.size();++k)
		cout << "track id " << complete_tracks[k].id() << " with " << complete_tracks[k].nblobs() << " blobs" << endl;
	cout << endl; 
	return 0;
}




