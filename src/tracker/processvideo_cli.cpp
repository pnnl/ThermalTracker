#include <iostream>
#include <boost/program_options.hpp>
#include "processvideo.hpp"

namespace po = boost::program_options;
using namespace boost;
using namespace std;

int main (int argc, char * const argv[]) {
  //--------------------------------------------------------------------------
  // PARSE COMMAND LINE
  // NOTE:  boost::program_options is broken. :(
  po::options_description desc;

  desc.add_options()
    ("help", "print help message")
    //("inputfile,i", po::value<string>()->default_value( "" ),"video file to process")
    ("inputfile,i", po::value<string>()->required(),"video file to process")
    ("outputdir,o", po::value<string>()->default_value( "" ),"output directory path including trailing /")
    ("winsecs,w", po::value<int>()->default_value( 10 ),"window size in seconds")
    ("fps,f", po::value<double>()->default_value( 0.0 ),"frames per second")
    ("view,v", "view image results" );

  po::variables_map options;

  try {
    po::store( po::parse_command_line( argc, argv, desc ), options );
  } catch( const std::exception& e ) {
    cerr << "Sorry, couldn't parse that: " << e.what() << endl;
    cerr << desc << endl;
    return -1;
  }
  

  
  if( options.count( "help" ) > 0 ) {
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
  params.windowSize = options["winsecs"].as<int>();
  params.framesPerSecond = options["fps"].as<double>();

  // If this option was passed at least once, it's true
  params.view = options.count( "view" ) > 0;

  return processVideo(params);
}
