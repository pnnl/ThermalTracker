/*
 *  visualization.hpp
 *  ThermalTrackerX
 *
 *  Created by Shari Matzner on 6/26/13.
 *  Copyright 2013 Pacific Northwest National Laboratory. All rights reserved.
 *
 */

#ifndef PNNL_TRACKER_VISUALIZATION_HPP
#define PNNL_TRACKER_VISUALIZATION_HPP

#include <string>
#include <opencv2/opencv.hpp>

#include "thermaltracker.hpp"

using namespace std;
using namespace cv;

struct WindowText {
    string  text;
    Point2i origin;
    int     font;
    double  font_scale;
    
    WindowText(const string& init_txt, Point2i init_origin, int init_font, double init_font_scale)
    : text(init_txt)
    , origin(init_origin)
    , font(init_font)
    , font_scale(init_font_scale) {};
};

class Viewer {

public:
	
	// Constructors
	Viewer(int height, int width, void* data, int border);
    
	// Methods
    void Refresh();
    int ShowImage(const WindowText& wt, int delay=0);
    int ShowZoom(int icenter, double zoom_scale, const WindowText& wt, int delay=0 );
    int ShowZoom(Point2i center, double zoom_scale, const WindowText& wt, int delay=0 );
    int ShowPixelZoom(int ipix, const Scalar& color, int delay=0);
    void HideZoom();
    int ShowContours(InputArrayOfArrays contours, const Scalar& color, int delay=0);
    int ShowBlobZoom(std::vector<unsigned>& ipix, const Scalar& color, int delay=0);
    int ShowContourZoom(const vector<Point2i>& contour, const Scalar& color, int delay=0);
    int DrawLineZoom(const Point2i& p1, const Point2i& p2, const Scalar& color, int delay=0);
    int DrawLine(const Point2i& p1, const Point2i& p2, const Scalar& color, int delay=0);
    int ShowTrack(const Track& track, const Scalar& color, int delay=0);
    int ShowTrackZoom(const Track& track, const Scalar& color, int delay=0);
    void MakeOutputImage(OutputArray im_out, const WindowText& wt, const vector<Track>& tracks, int frame1, int frame2);


private:
	
	// Constants
	static const Scalar kLineColors[];
	
    // TO DO:  all instances will use these same windows
    // need unique names, if want multiple viewers
	static const char *kWinMain;
	static const char *kWinZoom;
    
    // Functions
    
    // Variables
    int _im_height;
    int _im_width;
    int _border;
    
    Point2i _offset;
    double _zoom_scale;
    Mat _imc;     // original image
    Mat _imcb;    // original image with border
    Mat _imctemp0; // updated image
    Mat _imctemp; // created from _imctemp0 in ShowZoom(), updated by ShowPixelZoom()
    Mat _imroi;   // zoomed in region, set in ShowZoom()
    
}; // class Viewer

    
    


#endif // PNNL_TRACKER_VISUALIZATION_HPP