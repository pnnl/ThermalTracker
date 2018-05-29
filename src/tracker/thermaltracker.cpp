/*
 *  thermaltracker.cpp
 *  ThermalTracker
 *
 *  Created by Shari Matzner on 6/19/2015.
 *  Copyright 2015 Pacific Northwest National Laboratory. All rights reserved.
 *
 */
#include <iostream> // cin, cout, clog, cerr
#include <math.h> // PI, atan2

#include "thermaltracker.hpp"

using namespace std;
using namespace cv;

#define PI 3.14159265


Blob::Blob(const FrameNumber& fn, const std::vector<unsigned>& ipixels, const Mat_<PixelValue>& frameimage)
{
	// NOTE:  frameimage is vpsPeak image, when called from extracttracks main().
	npoints = ipixels.size();
	frame = fn;
	Mat matMask = Mat::zeros(frameimage.size(),CV_8UC1);
	for (int j=0;j<npoints;++j)
	{
		matMask.at<unsigned char>( idx2pnt(ipixels[j],frameimage.cols) ) = 255; //
	}
	
	// NOTE:  This is good. Without morphing, very noisy blob contours.
	Mat newMask;
	//morphologyEx(matMask, newMask, MORPH_CLOSE, Mat());
	//morphologyEx(newMask, matMask, MORPH_OPEN, Mat());
	std::vector< std::vector<Point2i> > contours;
	findContours(matMask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE); 
	if (!contours.empty()) 
	{
		contour = contours[0];
		rect = boundingRect(contour); // get the bounding rectangle
		frameimage(rect).copyTo(image,matMask(rect));
		std::vector<unsigned> idx;
		npoints = find(image>0, pixIdx);
		Point2i offset(rect.x,rect.y);
		for (int j=0; j<npoints; ++j)
		{ 
			points.push_back( idx2pnt(pixIdx[j], image.cols) + offset );
		}
		mom = moments(image); // spatial moments
		meanI = mom.m00/npoints; // mean intensity
		centroidx = mom.m10/mom.m00;
		centroidy = mom.m01/mom.m00;
		Point2i maxLoc;
		minMaxLoc(image, NULL, &maxI, NULL, &maxLoc, noArray());
		max_x = maxLoc.x;
		max_y = maxLoc.y;
	}
	//std::cout << "MAX LOC " << maxLoc.x << ", " << maxLoc.y << std::endl;  
	
}


void Track::write_blobs(std::ostream& strm) const
{
	int Nblobs = blobs.size();
	strm << "track_id, blob, frame, ULx, ULy, ncols, nrows, cent_x, cent_y, max_col, max_row, dir_deg, blob_len, blob_wid" << endl;
	for (int b=0; b<Nblobs; ++b)
	{
		std::vector<Point2f> points;
		if (0<b) points.push_back(blobs[b-1].centroid());
		points.push_back(blobs[b].centroid());
		if (b<(Nblobs-1)) points.push_back(blobs[b+1].centroid());		
			
		Vec4f line_fit;
		//cout << "fitting line..." ;
		fitLine(points,line_fit, CV_DIST_L2, 0.0, 0.01, 0.01);
		double flight_dir = atan2(line_fit[1],line_fit[0]) * 180/PI;
		//cout << " angle is " << flight_dir << " deg." << endl;
		
		// rotate points
		//Mat A(2,2,CV_64F);
		Mat A = getRotationMatrix2D(Point2f(0.0,0.0), flight_dir, 1);
		Mat points_rotated;
		transform(blobs[b].points, points_rotated, A);
		Rect R = boundingRect(points_rotated);
		
		
		
		strm << _id << ","
		<< b << ","
		<< blobs[b].frame << ","
		<< std::fixed << std::setprecision(2)
		<< blobs[b].rect.x << ","
		<< blobs[b].rect.y << ","
		<< blobs[b].rect.width << ","
		<< blobs[b].rect.height << ","
		<< blobs[b].centroidx << ","
		<< blobs[b].centroidy << ","
		<< blobs[b].max_x << ","
		<< blobs[b].max_y << ","
		<< flight_dir << ","
		<< R.width << ","            // Note: this is length in direction of flight
		<< R.height << endl;
		
	}
	
}

void test_blob_image(Blob bob)
{
	std::cout << std::endl;
	std::cout << "Blob size: " << bob.rect.width << " x " << bob.rect.height;
	std::cout << ", image size:  " << bob.image.cols << " x " << bob.image.rows << std::endl;
	std::cout << std::endl;
	
	//resize(bob.image, imzoom, 
	const char *WIN_BLOB="Blob Image";
	namedWindow(WIN_BLOB, CV_WINDOW_AUTOSIZE );
	imshow(WIN_BLOB,bob.image);
	int c = waitKey(0);
	
} // test_blob_image()
