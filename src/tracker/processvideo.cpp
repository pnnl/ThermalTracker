/*
*  processvideo.cpp
*
*
*  Created by Shari Matzner on 1/3/13.
*  Copyright 2013 Pacific Northwest National Laboratory. All rights reserved.
*
*/
#include <iostream> // cout, cin, cerr
#include <fstream>  // ofstream
#include <string>   // for strings
#include <iomanip>  // for controlling float print precision
#include <sstream>  // string to number conversion

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>

#include "thermaltracker.hpp"
#include "vpsfile.hpp"
#include "utilities.hpp"

using namespace std;
using namespace cv;
namespace fs = boost::filesystem;

int processVideo(Parameters params) {

if (params.inputFile() == "") {
  cerr << "Must specify a file to analyze" << endl;
  return -1;
}

fs::path vfilepath(params.inputFile());

//If the output path is specified, use it, otherwise use the default _out behavior
//and parse it out of the input file name.
string outpath;
if (params.outputDir() != "") {
  outpath = params.outputDir() + vfilepath.stem().string();
  fs::create_directories(fs::path(outpath).parent_path());
} else {
  generate_outpath_str(vfilepath.make_preferred().string(), outpath);
  fs::create_directory(fs::path(outpath).parent_path());
}

// text on output image
Point2i wintextOrigin(20,10);
int     wintextFont = FONT_HERSHEY_PLAIN;
double  wintextScale = 1;

//************************************************************************** 
// OPEN VIDEO FILE
//************************************************************************** 
VideoCapture capture(vfilepath.string());

if ( !capture.isOpened() )
{
  cerr << "Could not open file " << vfilepath.string() << endl;
  return -1;
}

cout << endl;
cout << endl << "Processing file " << vfilepath.string() << endl;
cout << "with " << params.windowSize << " second windows" << endl;

double frameHeight  = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
double frameWidth   = capture.get(CV_CAP_PROP_FRAME_WIDTH);
int    numFrames    = (int)capture.get(CV_CAP_PROP_FRAME_COUNT);

if ( !(params.framesPerSecond > 0) ) params.framesPerSecond = capture.get(CV_CAP_PROP_FPS); // the FPS property returned is sometimes wrong, so user can specify

cout << endl;
cout << "Width:  " << frameWidth << endl;
cout << "Height: " << frameHeight << endl;
cout << "FPS:    " << params.framesPerSecond << endl;
cout << "Total frames: " << numFrames << endl;

int  iFrame = 0; // frame index
Mat newFrame;
capture >> newFrame;  // 3-channel BGR (openCV default colorspace), all equal intensity because original was grayscale intensity image
++iFrame;

float maxChannelVal=0;
switch (newFrame.depth())
{
  case CV_8U:
    maxChannelVal = 255.0f;
    cout << "frame depth is 8U, channels = " << newFrame.channels() << endl;
    break;
  case CV_16U:
    maxChannelVal = 65535.0f;
    cout << "frame depth is 16U, channels = " << newFrame.channels() << endl;
    break;
  case CV_32F:
    maxChannelVal = 1.0f;
    cout << "frame depth is 32F, channels = " << newFrame.channels() << endl;
    break;
}


//************************************************************************** 
// INITIALIZE BACKGROUND
//************************************************************************** 
// TO DO: Make the length of the window for estimating background a user parameter.
//        Using the whole file takes a int time (but works on 50k+ frames).  

struct Background 
{
  int N; // number of frames for background moving window
  int Npixels; // number of pixels in frame data
  Mat win; // moving window
  Mat mean;
  Mat stdv;
} bg;

int bgWindowSec = 10 * params.windowSize;
bg.N = std::min(bgWindowSec * params.framesPerSecond, (float)numFrames);
cout << endl << "Estimating background over " << bg.N/params.framesPerSecond <<  " seconds (" << bg.N << " frames)..." << endl;
bg.Npixels = newFrame.rows * newFrame.cols;
bg.win.create(bg.N, bg.Npixels, CV_32FC1);

Mat grayFrame;
for (int k=0; k<bg.N; ++k)
{
  capture >> newFrame;  // 3-channel BGR, all equal intensity
  if(newFrame.empty()){
	  cout << "\r" << "Frame at index: " << k << " is empty.";
	  continue;
  }
  cvtColor(newFrame.reshape(0,1),grayFrame,CV_BGR2GRAY);
  grayFrame.convertTo(bg.win.row(k), CV_32F, 1.0f/maxChannelVal, 0);
  
  // the following is nice at command line but not so good
  // in the GUI
  /*
  cout << "\r" << setw(5) << k+1 << ": ";
  int X = ((float)(k+1)/bg.N) *10;
  cout << std::string(X, '|');
  cout << std::string(10-X, '-');
  cout.flush();
  */
}
cout << " done!" << endl;
reduce(bg.win, bg.mean, 0, CV_REDUCE_AVG);
double minVal, maxVal;
minMaxIdx(bg.mean, &minVal, &maxVal, 0, 0);
cout << "background mean intensity from " << minVal << " to " << maxVal << endl;

Mat sq_diff;
pow(bg.win - repeat(bg.mean, bg.N, 1),2.0f,sq_diff);
Mat bg_var;
reduce(sq_diff, bg_var, 0, CV_REDUCE_SUM);
bg_var = bg_var/(bg.N-1);
sqrt(bg_var, bg.stdv);
minMaxIdx(bg.stdv, &minVal, &maxVal, 0, 0);
cout << "background intensity standard deviation from " << minVal << " to " << maxVal << endl;

iFrame += bg.N;  // frame index

int vpsFrames  = ceil((double)params.windowSize * params.framesPerSecond);
vpsFrames = std::min(vpsFrames, numFrames);
vpsFrames = vpsFrames + (vpsFrames % 2); // make even
int halfFrames = vpsFrames/2; // 50% overlap of windows
int Nvps     = floor((double)numFrames/(double)vpsFrames) * 2; // estimated number of VPS windows

cout << "frames per window = " << vpsFrames << ", " << Nvps << " windows" << endl;

// VPS Data
VPS_Data d;
d.assign(Nvps, frameHeight*frameWidth);

const PixelValue  *frame; // for accessing frame pixels
PixelValue peakval;
int peakframe;
int newFrames;
int iWin = 0; // window index

const char *WIN_VPS="VPS Image";
if (params.view) namedWindow(WIN_VPS, CV_WINDOW_AUTOSIZE );
int waitdelay = 0; // default wait for keypress

int framesRemaining;
int ind = 0; // index into background window
//************************************************************************** 
// MAIN LOOP
//************************************************************************** 
//while (framesRemaining>halfFrames || ind < bg.N)
while ( ind < (bg.N - vpsFrames + 1) )
{

  d.vpsFrameStart[iWin] = iFrame - bg.N + ind;
  d.vpsFrameEnd[iWin] = d.vpsFrameStart[iWin] + vpsFrames-1;

  framesRemaining = numFrames - d.vpsFrameEnd[iWin];
  cout << endl << "window " << iWin+1 << " of " << Nvps 
        << ", frames remaining " << framesRemaining << endl;

  //***********************************************************************
  // Video Peak Store
  // Mat frameStack(bg.win.rowRange(bg.N - vpsFrames,bg.N));
  Mat frameStack(bg.win.rowRange(ind, ind + vpsFrames));
 
  
  // get peak values and associated frame for each pixel
  cout << "storing peak values" << endl;
  for (int j=0; j<bg.Npixels; ++j) // for each pixel
  {
    peakval = 0;
    peakframe = 0;
    for (int k=0; k<vpsFrames; ++k) // for each frame in VPS window
    {
        frame = frameStack.ptr<PixelValue>(k);
        peakframe = (frame[j]>peakval) ? k : peakframe;
        peakval = std::max(frame[j],peakval);
    }
    d.vpsPeak[iWin][j] = peakval;
    d.vpsPkFrame[iWin][j] = peakframe;
  }

  // show VPS image
  cout << "frames " << d.vpsFrameStart[iWin] << " to " << d.vpsFrameEnd[iWin] << endl;
  cout << "elapsed video time " 
       << elapsedVideoTime(d.vpsFrameStart[iWin],params.framesPerSecond) 
       << " to " << elapsedVideoTime(d.vpsFrameEnd[iWin],params.framesPerSecond) 
       << endl;

  //***********************************************************************
  // create output VPS image
  Mat vpsImage((int)frameHeight, (int)frameWidth, CV_32FC1, &(d.vpsPeak[iWin][0]));

 
  Mat bw0 = vpsImage > ( (bg.mean.reshape(0,(int)frameHeight) + 3*bg.stdv.reshape(0,(int)frameHeight)) );
  int Npix = find(bw0>0, d.pixListIdx[iWin]);
  cout << Npix << " non-zero pixels" << endl;
  
  Mat im_peaks;
  vpsImage.copyTo(im_peaks, bw0);
  
  Mat im_out;
  im_peaks.convertTo(im_out, CV_16U, 65535, 0);

  stringstream ss;
  ss << "Window " << iWin << ": " 
     << elapsedVideoTime(d.vpsFrameStart[iWin],params.framesPerSecond) 
     << " to " << elapsedVideoTime(d.vpsFrameEnd[iWin],params.framesPerSecond);
  putText(im_out, ss.str(), wintextOrigin, wintextFont, wintextScale, CV_RGB(65535,65535,65535));
  string pngfilepath = outpath + "_" 
                           + elapsedVideoTime2(d.vpsFrameStart[iWin],params.framesPerSecond) 
                           + ".png";
  cout << "writing to " << pngfilepath << endl;
  imwrite(pngfilepath,im_out);
  if (params.view)
  {
    imshow(WIN_VPS,vpsImage);
    int c = waitKey(waitdelay);
    if ( (char)c == 27 ) break; // stop processing, save and exit
    if ( (char)c == 'g' || (char)c == 'G' ) waitdelay = 10; // continue without waiting for keypress
  }

  iWin++;

  if ( (ind + halfFrames) < (bg.N - vpsFrames) )
  {
    ind += halfFrames;
    continue; // main loop
  }

  if (framesRemaining<halfFrames) break; // exit main loop

  //***********************************************************************
  // update background
  cout << "updating background" << endl;
  Mat oldbg(bg.win);
  oldbg.rowRange(halfFrames,bg.N).copyTo(bg.win.rowRange(0,bg.N-halfFrames));
  for (int k=bg.N-halfFrames; k<bg.N; ++k)
  {
    capture >> newFrame;  // 3-channel BGR, all equal intensity
    if(newFrame.empty()){
      cout << "\r" << "Frame at index: " << k << " is empty.";
      continue;
    }
    cvtColor(newFrame.reshape(0,1),grayFrame,CV_BGR2GRAY);
    grayFrame.convertTo(bg.win.row(k), CV_32F, 1.0f/maxChannelVal, 0);
  }
  reduce(bg.win, bg.mean, 0, CV_REDUCE_AVG);

  Mat sq_diff;
  pow(bg.win - repeat(bg.mean, bg.N, 1),2.0f,sq_diff);
  Mat bg_var;
  reduce(sq_diff, bg_var, 0, CV_REDUCE_SUM);
  bg_var = bg_var/(bg.N-1);
  sqrt(bg_var, bg.stdv);

  iFrame += halfFrames;
} // MAIN LOOP

if (params.view) destroyWindow(WIN_VPS);

//--------------------------------------------------------------------------
// SAVE RESULTS

fs::path outfilepath(outpath + ".vps");
cout << endl << "Saving data to " << outfilepath << endl;

VPS_FileHeader header;
strncpy(header.videoName,vfilepath.filename().string().c_str(),vfilepath.filename().string().size());
  header.videoName[vfilepath.filename().string().size()-1] = '\0'; // make sure it's null-terminated
  header.width = frameWidth;        
  header.height = frameHeight;       
  header.fps = params.framesPerSecond;               
  header.bitDepth = newFrame.elemSize1()*8; 
  header.Nframes = numFrames;         
  header.vpsWindowFrames = vpsFrames;         
  header.Nvps = iWin;             

  ofstream outfile(outfilepath.string().c_str(),ios::out | ios::binary);
  if (!outfile.is_open())
  {
    cerr << "Error opening output file: " << outfilepath.string() << endl;
    return (-1);
  }
  write_vps_file(outfile, header, d);
  outfile.close();
  cout << endl;
  return 0;
}
