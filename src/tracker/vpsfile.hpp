/*
 *  vpsfile.hpp
 *  ThermalTrackerX
 *
 *  Created by Shari Matzner on 2/6/13.
 *  Copyright 2013 Pacific Northwest National Laboratory. All rights reserved.
 *
 */

#ifndef __PNNL_VPSFILE_HPP__
#define __PNNL_VPSFILE_HPP__

#include <fstream>  // ofstream
#include <string>   // for strings
#include <vector>

#include "thermaltracker.hpp"

// VPS File written by ProcessVideo, read by ExtractTracks.
struct VPS_FileHeader {
	char  videoName[256];
	unsigned int width;     // width of frame in pixels
	unsigned int height;    // height of frame in pixels
	float        fps;       // frame rate (frames per second)
	unsigned int bitDepth;  // bits per pixel
	unsigned int Nframes;   // total number of frames
	unsigned int vpsWindowFrames; // frames per VPS window
	unsigned int Nvps;      // total number of VPS windows
	// TO DO:  add version number to header
};

struct VPS_Data {
	std::vector<FrameNumber>  vpsFrameStart; // start frame of each VPS window
	std::vector<FrameNumber>  vpsFrameEnd;   // end frame of each VPS window
    std::vector< std::vector<PixelValue> >  vpsPeak;       // VPS window images
    std::vector< std::vector<FrameNumber> > vpsPkFrame;    // frame number of each peak in VPS images
    std::vector< std::vector<unsigned> > pixListIdx;    // index of non-zero pixels in VPS image
	void assign(int numvps, int pixelsperframe)
	{
		// allocate memory
		vpsFrameStart.assign(numvps, 0);
		vpsFrameEnd.assign(numvps, 0); // may have extra/less frames in last window
		vpsPeak.assign(numvps, std::vector<PixelValue>(pixelsperframe,0));
		vpsPkFrame.assign(numvps, std::vector<FrameNumber>(pixelsperframe,0));
        pixListIdx.assign(numvps, std::vector<unsigned>());
	}
};

std::ostream& operator<<(std::ostream& strm, const VPS_FileHeader& fh);

void read_vps_file(std::ifstream& invps, VPS_FileHeader& fh, VPS_Data& d);
void write_vps_file(std::ofstream& outvps, const VPS_FileHeader& fh, const VPS_Data& d);


#endif
