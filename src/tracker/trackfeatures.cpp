/*
 *  trackfeatures.cpp
 *  ThermalTracker
 *
 *  Created by Shari Matzner on 2/4/13.
 *  Copyright 2013 Pacific Northwest National Laboratory. All rights reserved.
 *
 */


#include <iostream>  // cout, cin, cerr
#include <iomanip>   // setprecision
#include <fstream>   // ifstream, ofstream
#include <string>    // string
#include <algorithm> // find_if_not
#include <math.h> // atan2

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "utilities.hpp"
#include "thermaltracker.hpp"  // PixelValue, FrameNumber, Blob, Track types
#include "trackfile.hpp"       // file containing tracks output by extracttracks

using namespace std;
using namespace cv;
using namespace boost;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

#define PI 3.14159265

string xrelstr(double xrel)
{
	string str;
	if (xrel < 0.35)
        str = "left";
    else if (0.65 < xrel)
        str = "right";
    else
        str = "center";
	
	return str;
	
} // xrelstr

string yrelstr(double yrel)
{
	string str;
	if (yrel < 0.35)
        str = "top";
    else if (0.65 < yrel)
        str = "bottom";
    else
        str = "middle";
	
	return str;
	
} // xrelstr

struct Track_Statistics {
    int id;
    double start_x;
    double start_y;
    double start_xrel;
    double start_yrel;
    FrameNumber start_frame;
    double start_time_sec;
	string start_time_str;
    
    double end_x;
    double end_y;
    double end_xrel;
    double end_yrel;
    FrameNumber end_frame;
    double end_time_sec;
	string end_time_str;
    
    int frames_visible;
    int frame_span;
    double total_distance_pix;
    double sinuosity;
    
    double mean_speed;
    double min_speed;
    double max_speed;
    double std_speed;

    double mean_npoints;
    double mean_width;
    double mean_height;
    double mean_meanI;
    
    int min_npoints;
    int max_npoints;
    int min_width;
    int max_width;
    int min_height;
    int max_height;
    double min_meanI;
    double max_meanI;
    
    double median_npoints;
    double median_width;
    double median_height;
    double median_meanI;
    
    int Q1_npoints;
    int Q3_npoints;
    int Q1_width;
    int Q3_width;
    int Q1_height;
    int Q3_height;
    double Q1_meanI;
    double Q3_meanI;
    
    double std_npoints;
    double std_width;
    double std_height;
    double std_meanI;
    
    double mad_npoints;
    double mad_width;
    double mad_height;
    double mad_meanI;

	// calculate statistics
	void calc(const Track& trk, float fps, int imwidth, int imheight)
	{
		//cout << "Track_Statistics::calc() " << endl;
		id = trk._id;
		cout << "     start" << endl;
		start_x     = trk.blobs[0].rect.x + trk.blobs[0].centroidx;
		start_y     = trk.blobs[0].rect.y + trk.blobs[0].centroidy;
		start_xrel = start_x / imwidth;
		start_yrel = start_y / imheight;
		start_frame = trk.blobs[0].frame;
		start_time_sec = start_frame / fps;
		start_time_str = elapsedVideoTime(start_frame, fps);
		
		//cout << "     end" << endl;
		end_x    = trk.blobs.back().rect.x + trk.blobs.back().centroidx;;
		end_y    = trk.blobs.back().rect.y + trk.blobs.back().centroidy;
		end_xrel = end_x / imwidth;
		end_yrel = end_y / imheight;
		end_frame = trk.blobs.back().frame;
		end_time_sec = end_frame / fps;
		end_time_str = elapsedVideoTime(end_frame, fps);

		//cout << "     frames" << endl;
		frame_span = end_frame - start_frame;
		frames_visible = trk.blobs.size();

		//cout << "     declarations" << endl;
		std::vector<double> speed;
		std::vector<int> npoints;
		std::vector<int> width;
		std::vector<int> height;
		std::vector<double> meanI;
		
		//cout << "     total pix" << endl;
		total_distance_pix = 0.0;
		cout << "     more dec" << endl;
		int sum_npoints = 0;
		int sum_width = 0;
		int sum_height = 0;
		double sum_meanI = 0.0;
		double sum_speed = 0.0;
		
		// first pass to accumulate values in vectors, sum, mean
		//cout << "     first pass  ";
		int n = trk.blobs.size();
		for (int b=0; b<n; ++b)
		{
			if (0<b)
			{
				float  x1 = trk.blobs[b-1].rect.x + trk.blobs[b-1].centroidx;
				float  y1 = trk.blobs[b-1].rect.y + trk.blobs[b-1].centroidy;
				float  x2 = trk.blobs[b].rect.x + trk.blobs[b].centroidx;
				float  y2 = trk.blobs[b].rect.y + trk.blobs[b].centroidy;
				float  dist = sqrt( pow(x2 - x1, 2) + pow(y2 - y1, 2) );
				double  t = (double)(trk.blobs[b].frame - trk.blobs[b-1].frame) / fps;
				if (t>0)
				{
					speed.push_back(dist/t);
					sum_speed += speed.back();
				}
				total_distance_pix += dist;
				
				if (b<(n-1))
				{
					float x3 = trk.blobs[b+1].rect.x + trk.blobs[b+1].centroidx;
					float y3 = trk.blobs[b+1].rect.y + trk.blobs[b+1].centroidy;
					std::vector<Point2f> points;
					points.push_back(Point2f(x1,y1));
					points.push_back(Point2f(x2,y2));
					points.push_back(Point2f(x3,y3));
					Vec4f line_fit;
					cout << "fitting line..." ;
          InputArray arr(points);
					fitLine(points,line_fit, CV_DIST_L2, 0.0, 0.01, 0.01);
					// TODO:  Determine correct direction?  will angle be 0 to 360?
					double flight_dir = atan2(line_fit[1],line_fit[0]) * 180/PI;
					cout << " angle is " << flight_dir << " deg." << endl;
				}
			}
			
			npoints.push_back(trk.blobs[b].npoints);
			width.push_back(trk.blobs[b].rect.width);
			height.push_back(trk.blobs[b].rect.height);
			meanI.push_back(trk.blobs[b].meanI);
			
			sum_npoints += trk.blobs[b].npoints;
			sum_width += trk.blobs[b].rect.width;
			sum_height += trk.blobs[b].rect.height;
			sum_meanI += trk.blobs[b].meanI;
			
		}
		double x1 = trk.blobs[0].rect.x + trk.blobs[0].centroidx;
		double y1 = trk.blobs[0].rect.y + trk.blobs[0].centroidy;
		double x2 = trk.blobs.back().rect.x + trk.blobs.back().centroidx;
		double y2 = trk.blobs.back().rect.y + trk.blobs.back().centroidy;
		double straight_dist = sqrt( pow(x2 - x1, 2) + pow(y2 - y1, 2) );
		sinuosity = total_distance_pix / straight_dist;
		double total_time = frame_span/fps;
		//mean_speed = total_distance_pix/total_time;
		mean_speed = sum_speed/speed.size();
		mean_npoints = sum_npoints/n;
		mean_width = sum_width/n;
		mean_height = sum_height/n;
		mean_meanI = sum_meanI/n;
		
		// sort the vectors for median, min, max, quartiles
		std::sort(npoints.begin(), npoints.end());
		std::sort(width.begin(), width.end());
		std::sort(height.begin(), height.end());
		std::sort(meanI.begin(), meanI.end());
		
    if(speed.size() > 0) {
      min_speed = *min_element(speed.begin(),speed.end());
      max_speed = *max_element(speed.begin(),speed.end());
    }
		min_npoints = npoints.front();
		max_npoints = npoints.back();
		min_width = width.front();
		max_width = width.back();
		min_height = height.front();
		max_height = height.back();
		min_meanI = meanI.front();
		max_meanI = meanI.back();
		
		int ind1, ind2;
		if (n%2 == 0) // even
		{
			ind1 = n/2;
			ind2 = ind1+1;
		} else // odd
		{
			ind1 = (n+1)/2;
			ind2 = ind1;
		}
		median_npoints = (npoints[ind1]+npoints[ind2])/2;
		median_width = (width[ind1]+width[ind2])/2;
		median_height = (height[ind1]+height[ind2])/2;
		median_meanI = (meanI[ind1]+meanI[ind2])/2;
		
		int indL = ceil(0.25*(n+1));
		int indU = floor(0.75*(n+1));
		
		Q1_npoints = npoints[indL];
		Q3_npoints = npoints[indU];
		Q1_width = width[indL];
		Q3_width = width[indU];
		Q1_height = height[indL];
		Q3_height = height[indU];
		Q1_meanI = meanI[indL];
		Q3_meanI = meanI[indU];
		
		// second pass for std, mad
		//cout << "     second pass" << endl;
		sum_speed = 0;
		for (std::vector<double>::iterator it = speed.begin() ; it != speed.end(); ++it)
			sum_speed += pow((*it)-mean_speed,2);
		std_speed = sqrt(sum_speed/(speed.size()-1));

		sum_npoints = 0;
		sum_width = 0;
		sum_height = 0;
		sum_meanI = 0.0;
		for (int b=0; b<n; ++b)
		{
			sum_npoints += pow(trk.blobs[b].npoints-mean_npoints,2);
			sum_width += pow(trk.blobs[b].rect.width-mean_width,2);
			sum_height += pow(trk.blobs[b].rect.height-mean_height,2);
			sum_meanI += pow(trk.blobs[b].meanI-mean_meanI,2);
			
			npoints[b] = abs(npoints[b]-median_npoints);
			width[b] = abs(width[b]-median_width);
			height[b] = abs(height[b]-median_height);
			meanI[b] = abs(meanI[b]-median_meanI);
		}
		std_npoints = sqrt(sum_npoints/n);
		std_width = sqrt(sum_width/n);
		std_height = sqrt(sum_height/n);
		std_meanI = sqrt(sum_meanI/n);
		
		std::sort(npoints.begin(), npoints.end());
		std::sort(width.begin(), width.end());
		std::sort(height.begin(), height.end());
		std::sort(meanI.begin(), meanI.end());
		
		mad_npoints = (npoints[ind1]+npoints[ind2])/2;
		mad_width = (width[ind1]+width[ind2])/2;
		mad_height = (height[ind1]+height[ind2])/2;
		mad_meanI = (meanI[ind1]+meanI[ind2])/2;
		
	}; // calc
}; //Track_Statistics
    


void write_feature_labels(std::ostream& strm)
{
	strm
    << "ID" << ","
    << "Start" << ",,,,,,"
    << "End" << ",,,,,,"
    << "Frames visible" << ","
    << "Frame span" << ","
    << "Sinuosity" << ","
    << "Total distance (pixels)" << ","
    << "Speed (pixels/sec)" << ",,,,"
	<< "Number of points per blob" << ",,,,,,,,"
	<< "Blob width (pixels)" << ",,,,,,,,"
	<< "Blob height (pixels)" << ",,,,,,,,"
	<< "Blob intensity" << ",,,,,,,,"
	<< endl;
	
    strm
   // << ",video time, frame, x, y, 0=left 1=right, 0=top 1=bottom, video time, frame, x, y, 0=left 1=right, 0=top 1=bottom, ,,,, mean, min, max, std,";
    << ",video time, frame, x, y, top/bottom, left/right, video time, frame, x, y, top/bottom, left/right, ,,,, mean, min, max, std,";
    
	for (int j=0;j<3;++j)
		strm << "mean, min, max, std, median, Q1, Q3, MAD, ";
	strm << "mean, min, max, std, median, Q1, Q3, MAD" << endl;
}

void write_stats(std::ostream& strm, const Track_Statistics& stats)
{
 strm << fixed << setprecision(2)
   << stats.id << ","
   
   << stats.start_time_str << ","
   << stats.start_frame << ","
   << stats.start_x << ","
   << stats.start_y << ","
   << yrelstr(stats.start_yrel) << ","
   << xrelstr(stats.start_xrel) << ","
   
   << stats.end_time_str << ","
   << stats.end_frame << ","
   << stats.end_x << ","
   << stats.end_y << ","
   << yrelstr(stats.end_yrel) << ","
   << xrelstr(stats.end_xrel) << ","
   
   << stats.frames_visible << ","
   << stats.frame_span << ","
   << stats.sinuosity << ","
   << stats.total_distance_pix << ","
   
   << stats.mean_speed << ","
   << stats.min_speed << ","
   << stats.max_speed << ","
   << stats.std_speed << ","

 << stats.mean_npoints << ", "
 << stats.min_npoints << ", "
 << stats.max_npoints << ", "
 << stats.std_npoints << ", "
 << stats.median_npoints << ", "
 << stats.Q1_npoints << ", "
 << stats.Q3_npoints << ", "
 << stats.mad_npoints << ", "
 
 << stats.mean_width << ", "
 << stats.min_width << ", "
 << stats.max_width << ", "
 << stats.std_width << ", "
 << stats.median_width << ", "
 << stats.Q1_width << ", "
 << stats.Q3_width << ", "
 << stats.mad_width << ", "
 
 << stats.mean_height << ", "
 << stats.min_height << ", "
 << stats.max_height << ", "
 << stats.std_height << ", "
 << stats.median_height << ", "
 << stats.Q1_height << ", "
 << stats.Q3_height << ", "
 << stats.mad_height << ", " 
 
 << stats.mean_meanI << ", "
 << stats.min_meanI << ", "
 << stats.max_meanI << ", "
 << stats.std_meanI << ", "
 << stats.median_meanI << ", "
 << stats.Q1_meanI << ", "
 << stats.Q3_meanI << ", "
 << stats.mad_meanI 
 << endl;
 
}

int trackFeatures(Parameters params) {
    fs::path vfilepath(params.inputFile());

    string outpath;
    if (params.outputDir() != "") {
      outpath = params.outputDir() + vfilepath.stem().string();
    } else {
      generate_outpath_str(vfilepath.make_preferred().string(), outpath);
    }

    //-------------------------------------------------------------------------------------
    // READ TRK FILE
	
	string trkfilepath(outpath + ".trk");
    ifstream trkfile(trkfilepath.c_str(), ios::in | ios::binary);
	
	if (!trkfile.is_open())
	{
		cerr << "Error opening track file: " << trkfilepath << endl;
		cerr << "Run ExtractTracks to generate track file." << endl;
		return (-1);
	}
	
	cout << endl;
	cout << "reading file " << trkfilepath << endl;
    
    
	TRK_FileHeader trkheader;
	TRK_Data d;
	read_trackfile(trkfile, trkheader, d);
	trkfile.close();
	cout <<trkheader << endl;
	cout << endl << "read " << d.tracks.size() << " tracks" << endl;
    

    //-------------------------------------------------------------------------------------
    // CREATE CSV FILE
	
	string csvfilepath(outpath + ".csv");
    ofstream csvfile(csvfilepath.c_str(), ios::out | ios::trunc);
	
	if (!csvfile.is_open())
	{
		cerr << "Error creating output file: " << csvfilepath << endl;
		return (-1);
	}
	write_feature_labels(csvfile);
	cout << endl << "calculating statistics" << endl;
    
	for (int k=0; k<trkheader.Ntrk; ++k)
	{
		cout << "track " << d.tracks[k]._id << " with " << d.tracks[k].nblobs() << " blobs" << endl;
		Track_Statistics stats;
		//cout << "new statistics" << endl;
		//cout << "frame start " << d.tracks[k].frameStart() << ", frame end " << d.tracks[k].frameEnd() << endl;
		stats.calc(d.tracks[k], trkheader.fps, trkheader.width, trkheader.height);
		write_stats(csvfile, stats);
	}
	
	csvfile << endl;
	csvfile << trkheader << endl;
    csvfile.close();
    cout << endl;
	return 0;
          
}
