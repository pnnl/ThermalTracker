/*
 *  trackfile.cpp
 *  ThermalTracker
 *
 *  Created by Shari Matzner on 2/4/13.
 *  Copyright 2013 Pacific Northwest National Laboratory. All rights reserved.
 *
 */
#include <iostream>

#include "trackfile.hpp"

using namespace std;
using namespace cv;

ostream& operator<<(ostream& strm, const TRK_FileHeader& fh)
{
	strm << "videoName       = " << std::string(fh.videoName).c_str() << endl;
	strm << "width           = " << fh.width << endl;
	strm << "height          = " << fh.height << endl;
	strm << "fps             = " << fh.fps << endl;
	strm << "fracImageX      = " << fh.params.fracImageX << endl;
	strm << "fracImageY      = " << fh.params.fracImageY << endl;
	strm << "minTrackObjects = " << fh.params.minTrackObjects << endl;
	strm << "minObjectPix    = " << fh.params.minObjectPix << endl;
	strm << "maxFrameDiff    = " << fh.params.maxFrameDiff << endl;
	strm << "thresholdFactor = " << fh.params.thresholdFactor << endl;
	strm << "Ntrk            = " << fh.Ntrk << endl;
	
	return strm;
}


int write_trackfile(ofstream& outfile, TRK_FileHeader& fh, const TRK_Data& d)
{
	fh.Ntrk = d.tracks.size();
	outfile.write((char*)&fh, sizeof(fh));
	cout << "wrote header" << endl;
	// NOTE:  Vector data is guaranteed to be stored contiguously in the same order
	//        as represented by the vector.
		for (int j=0; j<fh.Ntrk; ++j) // each track
		{
            outfile.write((char*)&d.tracks[j]._id,sizeof(int));
			int ntrkblobs = d.tracks[j].nblobs();
			outfile.write((char*)&ntrkblobs,sizeof(int));
			for (int n=0; n<ntrkblobs; ++n) // each blob in track
			{
				int N = d.tracks[j].blobs[n].npoints;
				outfile.write((char*)&(d.tracks[j].blobs[n].npoints),sizeof(int));
                outfile.write((char*)d.tracks[j].blobs[n].pixIdx.data(),N*sizeof(int));
				outfile.write((char*)d.tracks[j].blobs[n].points.data(),N*sizeof(Point2i));
                
				outfile.write((char*)&(d.tracks[j].blobs[n].frame),sizeof(FrameNumber));
				outfile.write((char*)&(d.tracks[j].blobs[n].rect),sizeof(Rect));
                
				int nconpoints = d.tracks[j].blobs[n].contour.size();
				outfile.write((char*)&(nconpoints),sizeof(int));
				outfile.write((char*)(d.tracks[j].blobs[n].contour.data()),d.tracks[j].blobs[n].contour.size()*sizeof(Point2i));
                
				outfile.write((char*)&(d.tracks[j].blobs[n].mom),sizeof(Moments));
                
				
				outfile.write((char*)&(d.tracks[j].blobs[n].image.rows), sizeof(int));
				outfile.write((char*)&(d.tracks[j].blobs[n].image.cols), sizeof(int));
 				outfile.write((char*)(d.tracks[j].blobs[n].image.ptr(0)),d.tracks[j].blobs[n].image.total()*sizeof(PixelValue));
               
				outfile.write((char*)&(d.tracks[j].blobs[n].meanI),sizeof(double));
				outfile.write((char*)&(d.tracks[j].blobs[n].max_x),sizeof(int));
				outfile.write((char*)&(d.tracks[j].blobs[n].max_y),sizeof(int));
			} // each blob
		}// each track
    return 0; // TO DO:  return number of bytes written
}

int read_trackfile(ifstream& trkfile, TRK_FileHeader& fh, TRK_Data& d)
{
    
    trkfile.read((char*)&fh,sizeof(TRK_FileHeader));
    
        d.tracks.assign(fh.Ntrk,Track());
		for (int j=0; j<fh.Ntrk; ++j) // each track 
		{
            int id;
            trkfile.read((char*)&id,sizeof(int));
            d.tracks[j]._id = id;
			int ntrkblobs;
			trkfile.read((char*)&ntrkblobs,sizeof(int));
            d.tracks[j].blobs.clear();
            //cout << "reading " << ntrkblobs << " blobs for track " << j << endl;
			for (int n=0; n<ntrkblobs; ++n) // each blob in track
			{
 				int N;
				trkfile.read((char*)&N,sizeof(int));
				//cout << N << " points in blob " << n << endl;
                Blob newblob(N);
				trkfile.read((char*)(newblob.pixIdx.data()),N*sizeof(int));
				trkfile.read((char*)(newblob.points.data()),N*sizeof(Point2i));
                
				trkfile.read((char*)&(newblob.frame),sizeof(FrameNumber));
				trkfile.read((char*)&(newblob.rect),sizeof(Rect));
				int nconpoints;
				trkfile.read((char*)&(nconpoints),sizeof(int));
				//cout << nconpoints << " points in contour of blob " << n << endl;
                newblob.contour.assign(nconpoints,Point2i(0,0));
				trkfile.read((char*)(newblob.contour.data()),nconpoints*sizeof(Point2i));
                
				trkfile.read((char*)&(newblob.mom),sizeof(Moments));
                
				int nrows, ncols;
				trkfile.read((char*)&nrows, sizeof(int));
				trkfile.read((char*)&ncols, sizeof(int));
				//cout << "blob image size: " << ncols << " x " << nrows << endl;
                newblob.image.create(nrows, ncols, CV_32FC1);
				PixelValue* pImage = newblob.image.ptr<PixelValue>(0);
				for (int p=0; p<newblob.image.total(); ++p)
				{
					PixelValue val;
					trkfile.read((char*)&val,sizeof(val));
					pImage[p] = val;
				}
				//trkfile.read((char*)(newblob.image.ptr<PixelValue>(0)),nrows*ncols*sizeof(PixelValue));
                
				trkfile.read((char*)&(newblob.meanI),sizeof(double));
				//cout << "blob mean intensity is " << newblob.meanI << endl;
				
                newblob.centroidx = newblob.mom.m10/newblob.mom.m00;
                newblob.centroidy = newblob.mom.m01/newblob.mom.m00;
				
				trkfile.read((char*)&(newblob.max_x),sizeof(int));
				trkfile.read((char*)&(newblob.max_y),sizeof(int));

				d.tracks[j].blobs.push_back(newblob);
				//cout << "----------" << endl;
				
			} // each blob
		} // each track
    return 0;
}
