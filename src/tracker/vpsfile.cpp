/*
 *  vpsfile.cpp
 *  ThermalTrackerX
 *
 *  Created by Shari Matzner on 2/6/13.
 *  Copyright 2013 Pacific Northwest National Laboratory. All rights reserved.
 *
 */
#include <iostream>

#include "vpsfile.hpp"

using namespace std;

ostream& operator<<(std::ostream& strm, const VPS_FileHeader& fh)
{
	cout << "videoName       = " << std::string(fh.videoName).c_str() << endl;
	cout << "width           = " << fh.width << endl;
	cout << "height          = " << fh.height << endl;
	cout << "fps             = " << fh.fps << endl;
	cout << "bitDepth        = " << fh.bitDepth << endl;
	cout << "Nframes         = " << fh.Nframes << endl;
	cout << "vpsWindowFrames = " << fh.vpsWindowFrames << endl;
	cout << "Nvps            = " << fh.Nvps << endl;
	
	return strm;
}


void read_vps_file(ifstream& invps, VPS_FileHeader& fh, VPS_Data& d)
{
	invps.read((char*)&fh,sizeof(VPS_FileHeader));
	int numpixels = fh.height*fh.width;

	// allocate memory
	d.assign(fh.Nvps, numpixels);
	invps.read((char*)d.vpsFrameStart.data(), fh.Nvps*sizeof(FrameNumber));
	invps.read((char*)d.vpsFrameEnd.data(), fh.Nvps*sizeof(FrameNumber));
    size_t sizeIdx;
	for (int k=0; k<fh.Nvps; ++k)
	{
		invps.read((char*)d.vpsPeak[k].data(),sizeof(PixelValue)*numpixels);
		invps.read((char*)d.vpsPkFrame[k].data(),sizeof(FrameNumber)*numpixels);
        invps.read((char*)(&sizeIdx),sizeof(sizeIdx));
        d.pixListIdx[k].assign(sizeIdx,0);
        invps.read((char*)d.pixListIdx[k].data(),sizeof(unsigned)*sizeIdx);
	}
		
	
}

void write_vps_file(ofstream& outvps, const VPS_FileHeader& fh, const VPS_Data& d)
{
	// write header
	outvps.write((char*)&fh, sizeof(fh));
	int numpixels = fh.height*fh.width;
	
	// write data
	// NOTE:  Vector data is guaranteed to be stored in contiguously in the same order 
	//        as represented by the vector.
	outvps.write((char*)d.vpsFrameStart.data(), sizeof(FrameNumber)*fh.Nvps);
	outvps.write((char*)d.vpsFrameEnd.data(), sizeof(FrameNumber)*fh.Nvps);
    size_t sizeIdx;
	for (int k=0; k<fh.Nvps; ++k) 
	{
		outvps.write((char*)d.vpsPeak[k].data(),sizeof(PixelValue)*numpixels);
		outvps.write((char*)d.vpsPkFrame[k].data(),sizeof(FrameNumber)*numpixels);
        sizeIdx = d.pixListIdx[k].size();
        outvps.write((char*)(&sizeIdx),sizeof(sizeIdx));
        outvps.write((char*)d.pixListIdx[k].data(),sizeof(unsigned)*sizeIdx);
	}		
	
}
