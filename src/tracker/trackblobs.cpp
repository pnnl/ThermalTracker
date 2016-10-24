/*
 *  trackblobs.cpp
 *  ThermalTracker
 *
 *  Created by Shari Matzner on 3/5/14.
 *  Copyright 2014 Pacific Northwest National Laboratory. All rights reserved.
 *
 */

#include <iostream>  // cout, cin, cerr
#include <iomanip>   // setprecision
#include <fstream>   // ifstream, ofstream
#include <string>    // string

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <opencv2/opencv.hpp>

#include "utilities.hpp"       // generate_outpath_str()
#include "thermaltracker.hpp"  // PixelValue, FrameNumber, Blob, Track types
#include "trackfile.hpp"       // file containing tracks output by extracttracks

using namespace std;
using namespace cv;
using namespace boost;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

int trackBlobs(Parameters params) {
	//TO DO:  Check for input file argument.
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
	cout << endl;

	//-------------------------------------------------------------------------------------
    // CREATE CSV FILE
	
	cout << endl;
	
	int N = trkheader.Ntrk;
	for (int k=0; k<N; ++k)
	{
		cout << "track " << d.tracks[k]._id << " with " << d.tracks[k].nblobs() << " blobs" << endl;
		for (int b=0; b<d.tracks[k].nblobs(); ++b)
		{
		//	test_blob_image(d.tracks[k].blobs[b]);
			Mat im_out;
			d.tracks[k].blobs[b].image.convertTo(im_out, CV_16U, 65535, 0);
			stringstream pngfilepath;
			pngfilepath << outpath << "_trk" << setfill('0') << setw(4) << d.tracks[k]._id<< "blob" << setw(3) << b << ".png"; 
			imwrite(pngfilepath.str(),im_out);
		}

			
		
		stringstream ss;
		ss << "-blobs_" << setfill('0') << setw(4) << d.tracks[k]._id << ".csv";
		string csvfilepath(outpath + ss.str());
		ofstream csvfile(csvfilepath.c_str(), ios::out | ios::trunc);
		
		if (!csvfile.is_open())
		{
			cerr << "Error creating output file: " << csvfilepath << endl;
			return (-1);
		}
		d.tracks[k].write_blobs(csvfile);
		csvfile.close();
				 
	}
	
	return 0;

}

