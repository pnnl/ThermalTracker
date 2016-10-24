/*
 *  extracttracks.cpp
 *
 *  Created by Shari Matzner on 12/28/12.
 *  Copyright 2012 Pacific Northwest National Laboratory. All rights reserved.
 *
 */

#include <iostream> // cout, cin, cerr
#include <string>   // for strings
#include <boost/program_options.hpp>

#include "extracttracks.hpp"
#include "thermaltracker.hpp"

using namespace std;
using namespace boost;
namespace po = boost::program_options;

int main (int argc, char * const argv[]) {
    // process arguments
	po::options_description desc;
	desc.add_options()
	("help", "print help message")
	//("inputfile,i", po::value<string>()->default_value( "" ),"video file to process")
	("inputfile,i", po::value<string>()->required(),"video file to process")
    ("outputdir,o", po::value<string>()->default_value( "" ),"output directory path including trailing /")
	("fracx,x", po::value<float>()->default_value( 0.10 ),"fraction of video width containing consecutive objects in track" )
	("fracy,y", po::value<float>()->default_value( 0.10 ),"fraction of video height containing consecutive objects in track" )
	("pix,p", po::value<unsigned int>()->default_value( 9 ),"minimum number of pixels constituting an object" )
	("obj,o", po::value<unsigned int>()->default_value( 6 ),"minimum number of objects constituting a track" )
	("fdiff,f", po::value<unsigned int>()->default_value( 10 ),"maximum number of frames between consecutive track objects" )
    ("view,v", "view image results" );

	po::variables_map options;
    try
    {
        po::store( po::parse_command_line( argc, argv, desc ), options );
    }
    catch( const std::exception& e )
    {
        cerr << "Sorry, couldn't parse that: " << e.what() << endl;
        cerr << desc << endl;
        return -1;
    }
	
	
    if( options.count( "help" ) > 0 )
    {
        cerr << desc << endl;
        return 0;
    }
    
    if( options.count( "inputfile" ) == 0 ) {
        cerr << "Input file required. Available options: " << endl << desc << endl;
        return -1;
    }
  
    po::notify( options );
    
	Parameters params;
	params.fracImageX         = options["fracx"].as<float>();
	params.fracImageY         = options["fracy"].as<float>();
	params.minTrackObjects    = options["obj"].as<unsigned int>();
	params.minObjectPix       = options["pix"].as<unsigned int>();
	params.maxFrameDiff       = options["fdiff"].as<unsigned int>();
    params.inputFile(options["inputfile"].as<string>());
    params.outputDir(options["outputdir"].as<string>());
    params.view = options.count( "view" ) > 0;

  return extractTracks(params);
}
