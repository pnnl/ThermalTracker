/*
 *  trackfile.hpp
 *  ThermalTrackerX
 *
 *  Created by Shari Matzner on 2/6/13.
 *  Copyright 2013 Pacific Northwest National Laboratory. All rights reserved.
 *
 */

#ifndef __PNNL_TRACKFILE_HPP__
#define __PNNL_TRACKFILE_HPP__

#include <fstream>  // ofstream
#include <vector>

#include "thermaltracker.hpp"

// Track file written by ExtractTracks, read by TrackFeatures
struct TRK_FileHeader {
	// TO DO:  add camera specs
	char  videoName[256];
	unsigned int width;     // width of frame in pixels
	unsigned int height;    // height of frame in pixels
	float        fps;       // frame rate (frames per second)
	Parameters   params; // parameter settings used for track extraction
	unsigned int framesperwindow; // number of frames in each window
	unsigned int Ntrk;   // number of tracks
};
std::ostream& operator<<(std::ostream& strm, const TRK_FileHeader& fh);

struct TRK_Data {
	/*
	std::vector<FrameNumber>           vpsFrameStart; // start frame of each VPS window
	std::vector<FrameNumber>           vpsFrameEnd;   // end frame of each VPS window
	std::vector< std::vector<Track> >  vpsTracks;     // tracks in each VPS window

	void assign(int numvps)
	{
		// allocate memory
		vpsFrameStart.assign(numvps, 0);
		vpsFrameEnd.assign(numvps, 0); // may have extra/less frames in last window
		vpsTracks.assign(numvps, std::vector<Track>());
	}
	 */
	std::vector<Track> tracks;
	
	TRK_Data() {};
	TRK_Data(const std::vector<Track>& initial_tracks) : tracks(initial_tracks) {};
};

int write_trackfile(std::ofstream& outfile, TRK_FileHeader& fh, const TRK_Data& d);
int read_trackfile(std::ifstream& trkfile, TRK_FileHeader& fh, TRK_Data& d);

#endif
