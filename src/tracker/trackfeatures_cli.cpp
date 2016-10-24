/*
 *  trackfeatures.cpp
 *  ThermalTracker
 *
 *  Created by Shari Matzner on 2/4/13.
 *  Copyright 2013 Pacific Northwest National Laboratory. All rights reserved.
 *
 */


#include <iostream>  // cout, cin, cerr
#include <string>    // string

#include <boost/program_options.hpp>
#include "trackfeatures.hpp"

using namespace std;
using namespace boost;
namespace po = boost::program_options;

//-------------------------------------------------------------------------------------
//  MAIN
//-------------------------------------------------------------------------------------
int main (int argc, char * const argv[]) {
    // process arguments
	po::options_description desc;
	desc.add_options()
    ("help", "print help message")
    ("inputfile,i", po::value<string>()->required(),"video file to process")
    ("outputdir,o", po::value<string>()->default_value( "" ),"output directory path including trailing /");

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
  params.inputFile(options["inputfile"].as<string>());
  params.outputDir(options["outputdir"].as<string>());

  return trackFeatures(params);
}
