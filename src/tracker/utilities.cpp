/*
 *  utilities.cpp
 *  
 *
 *  Created by Shari Matzner on 1/3/13.
 *  Copyright 2013 Pacific Northwest National Laboratory. All rights reserved.
 *
 */

#include "utilities.hpp"

#include <cstdio>

// Similar to Matlab find function
// src must be a logical array, matrix, or vector.
// dst is a vector of indices of the nonzero elements
// of src.
long find(InputArray src, std::vector<unsigned>& out) {
	
	long Nfound = 0;
	Mat matsrc;
	src.getMat().convertTo(matsrc, CV_8U); // get Mat headers
	Size matsize = matsrc.size();
	out.clear();
	
	// TO DO: Make sure src is a single channel.
	
	if ( matsrc.isContinuous() )
	{
		matsize.width *= matsize.height;
		matsize.height = 1;
	}
	
	for (int i=0; i<matsize.height; i++)
	{
		uchar *dat = matsrc.ptr(0);
		for (long j=0; j<matsize.width; ++j)
		{
			if ( dat[j] ) 
			{
				out.push_back(j);
				++Nfound;
			}
		}
	}
	return(Nfound);
}


// TO DO:  Make array versions of these functions (indx2pnt and pnt2idx).
Point2i idx2pnt(int idx, int mcols) {
	Point2i pnt(-1,-1);
	
	pnt.y = floor(idx/mcols);
	pnt.x = idx - pnt.y*mcols;
	
	return(pnt);
}

int pnt2idx(Point2i pnt, int mcols) {
	int idx = -1;
	
	idx = pnt.y*mcols + pnt.x;
	return(idx);
}

void getNeighbors(int idx, int mrows, int mcols, std::vector<unsigned>& outidx) {
	outidx.clear();
	int y = floor(idx/mcols);
	int x = idx - y*mcols;
	
	if (y>0) 
	{
		if (x>0)       outidx.push_back((y-1)*mcols + x-1); // left, up
					   outidx.push_back((y-1)*mcols + x);   // up
		if (x<mcols-1) outidx.push_back((y-1)*mcols + x+1); // right, up
	}
	
	if (x>0)           outidx.push_back(y*mcols + (x-1));   // left 
	if (x<mcols-1)     outidx.push_back(y*mcols + (x+1));   // right
	
	if (y<mrows-1)
	{	
		if (x>0)       outidx.push_back((y+1)*mcols + x-1); // left, down
		               outidx.push_back((y+1)*mcols + x);   // down
		if (x<mcols-1) outidx.push_back((y+1)*mcols + x+1); // right, down
	}
	
}


std::string elapsedVideoTime(int frame, double fps)
{
	double etimesec = frame/fps;
	int etimemin = floor(etimesec/60);
	
	char buff[32];
	snprintf(buff, 32, "%02d:%06.3f",etimemin, etimesec - etimemin*60);

	
	return std::string(buff);
}

std::string elapsedVideoTime2(int frame, double fps)
{
	double etimesec = frame/fps;
	int etimefrac = floor((etimesec - floor(etimesec))*10);
	int etimemin = floor(etimesec/60);
	int etimehr = floor(etimemin/60);
	
	char buff[32];
	snprintf(buff, 32, "%02dh%02dm%02d-%d", etimehr, etimemin-etimehr*60, (int)floor(etimesec - etimemin*60), etimefrac);
	
	return std::string(buff);
}

void generate_outpath_str(const std::string vfilepath_str, std::string& outpath_str)
{
    unsigned dotpos = vfilepath_str.find_last_of('.');
    unsigned slashpos = vfilepath_str.find_last_of("/\\");
    outpath_str = vfilepath_str.substr(0,dotpos) + "_out/" + vfilepath_str.substr(slashpos+1,dotpos-slashpos-1);
    
    
}
