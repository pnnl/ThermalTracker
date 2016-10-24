#ifndef __PNNL_THERMALTRACKER_HPP__
#define __PNNL_THERMALTRACKER_HPP__

#include <fstream>  // ofstream
#include <iomanip>  // setprecision()
#include <iostream>
#include <opencv2/opencv.hpp>

# include "utilities.hpp" // find()
#include "process_params.hpp"

// TO DO:  put this all in a namespace

typedef  int  FrameNumber; 
typedef float PixelValue;

// Group of connected pixels
struct  Blob {
    unsigned int          npoints; // number of points
	std::vector<unsigned> pixIdx; // index of pixel location in image vector
	std::vector<cv::Point2i>  points; // pixels as 2D points
	FrameNumber           frame;  // frame when pixel peaks occurred
    cv::Rect              rect;   // bounding rectangle
	std::vector<cv::Point2i> contour; // convex hull points
	cv::Moments           mom;    // moments
    cv::Mat               image;  // grayscale image with non-blob pixels set to 0
    double                meanI;  // mean intensity
    double                centroidx; // centroid, horizontal coordinate
    double                centroidy; // centroid, vertical coordinate
	double				  maxI;   // max intensity
	int				      max_x;  // location of max value
	int				      max_y;  // location of max value
    
    Blob(unsigned int n) // allocate memory only
    {
        npoints = n;
        pixIdx.assign(n,0);
        points.assign(n,Point2i(-1,-1));
        frame = -1;
    };
    
	Blob() {};
	
    // construct fully initialized blob
    Blob(const FrameNumber& fn, const std::vector<unsigned>& ipixels, const Mat_<PixelValue>& frameimage);
    
    // weak ordering to sort earliest/largest to latest/smallest
    bool operator<(const Blob& b2) const
    {
        return ((frame<b2.frame)
                || (frame==b2.frame && npoints>b2.npoints));
        
    };
	
	cv::Point2f centroid() const { return cv::Point2f(rect.x + centroidx, rect.y + centroidy); };
    
}; // struct Blob


// Sequence of connected blobs
class Track {
public:
    int _id;
    std::vector<Blob> blobs;
	
    
    // Constructors, Destructors
    Track() {};
    Track(int id, std::vector<Blob> track_blobs) : _id(id), blobs(track_blobs) {};
    Track(int id, Blob first_blob) : _id(id) { blobs.push_back(first_blob); };
    
    // Methods
    inline int  id() const         { return _id; };
    inline int  nblobs() const     { return blobs.size(); };
    FrameNumber frameStart() const { return (blobs.size()>0) ? blobs.front().frame : -1; };
    FrameNumber frameEnd() const   { return (blobs.size()>0) ? blobs.back().frame : -1; };
    inline Blob back() const       { return blobs.back(); };
	void        frameList(std::vector<FrameNumber>& flist) const 
	{ 
		flist.clear(); 
		for (int j=0; j<blobs.size(); j++)
			flist.push_back(blobs[j].frame);
	}
    
    // weak ordering to sort earliest to latest
    bool operator<(const Track& t2) const
    {
        return ((frameStart()<t2.frameStart())
                || (frameStart()==t2.frameStart() && frameEnd()<t2.frameEnd()));
        
    };
    
    // write out comma-separated list of blob centroids
    void write_centroids(std::ostream& strm) const
    {
		// write column labels
		strm << "ID, frame, centroid x, centroid y, max x, max y" << std::endl;
        int n = blobs.size();
        for (int b=0; b<n; ++b)
        {
            strm << _id << "," << blobs[b].frame << ","
			<< std::fixed << std::setprecision(2)
			<< blobs[b].rect.x + blobs[b].centroidx << ","
            << blobs[b].rect.y + blobs[b].centroidy << ",";
 			strm << blobs[b].rect.x + blobs[b].max_x << ","
            << blobs[b].rect.y + blobs[b].max_y << std::endl;
       }
    }

	// write out comma-separated list of blob locations and images
    void write_blobs(std::ostream& strm) const;
	
	// add blob to track
    void push_back(Blob new_blob) { blobs.push_back(new_blob); };
    
}; // class Track

#endif // __PNNL_THERMALTRACKER_HPP__
