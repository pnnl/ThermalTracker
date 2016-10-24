
/*
 *  visualization.cpp
 *  ThermalTrackerX
 *
 *  Created by Shari Matzner on 6/26/13.
 *  Copyright 2013 Pacific Northwest National Laboratory. All rights reserved.
 *
 */

#include "visualization.hpp"
#include "utilities.hpp"

const Scalar Viewer::kLineColors[] = {
	/*
	// from colorbrewer2.org
	// 12 labels (only using 11) , qualitative
    CV_RGB(141,211,199),   // 0
    CV_RGB(255, 255, 179), // 1
    CV_RGB(190, 186, 218), // 2
    CV_RGB(251, 128, 114), // 3
    CV_RGB(128, 177, 211), // 4
    CV_RGB(253, 180, 98),  // 5
    CV_RGB(179, 222, 105), // 6
    CV_RGB(252, 205, 229), // 7
    CV_RGB(188, 128, 189), // 8
    CV_RGB(204, 235, 197), // 9
    CV_RGB(255, 237, 111)  //10
	*/
	// made up by me
	// from http://www.colorschemer.com/online.html
    CV_RGB(245,0,61),   // 0
    CV_RGB(51, 255, 102), // 1
    CV_RGB(51, 102, 255), // 2
    CV_RGB(184, 0, 46), // 3
    CV_RGB(0, 184, 138), // 4
    CV_RGB(51, 204, 255),  // 5
    CV_RGB(255, 51, 204), // 6
    CV_RGB(204, 255, 51), // 7
    CV_RGB(102, 51, 255), // 8
    CV_RGB(255, 102, 51), // 9
    CV_RGB(51, 255, 204)  //10
	
};  //

const char* Viewer::kWinMain   = "Image";
const char* Viewer::kWinZoom  = "Zoomed-in Region";


Viewer::Viewer(int height, int width, void* data, int border)
: _im_height(height)
, _im_width(width)
, _border(border)
{
    // grayscale image from original data
    Mat temp(_im_height, _im_width, CV_32FC1, data);
	// create a blank grayscale image same size as temp + borders
    _imcb.create(_im_height+_border*2, _im_width+_border*2,temp.depth());
	// create a matrix header for inner region of bordered image
    _imc = _imcb(Range(_border,_border+_im_height),Range(_border,_border+_im_width));
	// convert grayscale to color in the center of the bordered image
    cvtColor(temp, _imc, CV_GRAY2RGB);
	// initialize border to 0 (black)
    copyMakeBorder(_imc, _imcb, _border, _border, _border, _border, BORDER_CONSTANT, Scalar::all(0));
    _offset = Point2i(_border, _border);
    _imcb.copyTo(_imctemp0);
} // Viewer::Viewer()

void Viewer::Refresh()
{
    //init_image();
    _imcb.copyTo(_imctemp0);
}

int Viewer::ShowImage(const WindowText& wt, int delay)
{
	putText(_imc, wt.text, wt.origin, wt.font, wt.font_scale, CV_RGB(255,255,255));
    namedWindow(kWinMain, CV_WINDOW_AUTOSIZE );
	imshow(kWinMain,_imcb);
	return waitKey(delay) ;
    
} // Viewer::ShowImage()


int Viewer::ShowZoom(int icenter, double zoom_scale, const WindowText& wt, int delay)
{
    Point2i center = idx2pnt(icenter, _im_width);
    return ShowZoom(center, zoom_scale, wt, delay) ;
}

int Viewer::ShowZoom(Point2i center, double zoom_scale, const WindowText& wt, int delay)
{
    center += _offset;
    _zoom_scale = zoom_scale;
    _imctemp0.copyTo(_imctemp);
    _imroi = Mat( _imctemp,
              Range(center.y-_border+1,center.y+_border-1),
              Range(center.x-_border+1,center.x+_border-1) );
    
    putText(_imctemp, wt.text, wt.origin, wt.font, wt.font_scale, CV_RGB(255,255,255));
    rectangle(_imctemp, Rect(center.x-_border,center.y-_border,_border*2,_border*2),CV_RGB(255,255,0));
    imshow(kWinMain,_imctemp);
    
    return waitKey(delay) ;
    
} // Viewer::ShowZoom()

	
	
int Viewer::ShowPixelZoom(int ipix, const Scalar& color, int delay)
{
    Point2i pnt = idx2pnt(ipix,_im_width) + _offset;
    _imctemp.at<Vec3f>(pnt)[0]=color[0];
    _imctemp.at<Vec3f>(pnt)[1]=color[1];
    _imctemp.at<Vec3f>(pnt)[2]=color[2];
    
    Mat imzoom = _imroi;
    resize( _imroi,imzoom,Size(),_zoom_scale,_zoom_scale );
    namedWindow(kWinZoom, CV_WINDOW_AUTOSIZE );
    imshow(kWinZoom,imzoom);
    
    return waitKey(delay) ;

} // Viewer::ShowPixel()

void Viewer::HideZoom()
{
    destroyWindow(kWinZoom);
}
	
int Viewer::ShowContours(InputArrayOfArrays contours, const Scalar& color, int delay)
{
    drawContours(_imctemp0, contours, -1, color, 1, 8, noArray(), INT_MAX, _offset);
    imshow(kWinMain,_imctemp0);
		return waitKey(delay) ;
    
} // Viewer::ShowContours()
	

int Viewer::ShowBlobZoom(std::vector<unsigned>& ipix, const Scalar& color, int delay)
{
    for (int k=0; k< ipix.size(); ++k)
    {
        Point2i pnt = idx2pnt(ipix[k],_im_width) + _offset;
        _imctemp.at<Vec3f>(pnt)[0]=color[0];
        _imctemp.at<Vec3f>(pnt)[1]=color[1];
        _imctemp.at<Vec3f>(pnt)[2]=color[2];
    }
    Mat imzoom = _imroi;
    resize( _imroi,imzoom,Size(),_zoom_scale,_zoom_scale );
    namedWindow(kWinZoom, CV_WINDOW_AUTOSIZE );
    imshow(kWinZoom,imzoom);
    return waitKey(delay) ;
}

int Viewer::ShowContourZoom(const vector<Point2i>& contour, const Scalar& color, int delay)
{
    vector< vector<Point2i> > contour_list;
    contour_list.push_back(contour);
    drawContours(_imctemp, contour_list, -1, color, 1, 8, noArray(), INT_MAX, _offset);
    
    Mat imzoom = _imroi;
    resize( _imroi,imzoom,Size(),_zoom_scale,_zoom_scale );
    namedWindow(kWinZoom, CV_WINDOW_AUTOSIZE );
    imshow(kWinZoom,imzoom);
    return waitKey(delay) ;
} // Viewer::ShowContourZoom()


int Viewer::DrawLineZoom(const Point2i& p1, const Point2i& p2, const Scalar& color, int delay)
{
    line(_imctemp,Point(p1+_offset),Point(p2+_offset),color);
    
    Mat imzoom = _imroi;
    resize( _imroi,imzoom,Size(),_zoom_scale,_zoom_scale );
    namedWindow(kWinZoom, CV_WINDOW_AUTOSIZE );
    imshow(kWinZoom,imzoom);
    return waitKey(delay) ;

} // Viewer::DrawLineZoom()

int Viewer::DrawLine(const Point2i& p1, const Point2i& p2, const Scalar& color, int delay)
{
    line(_imctemp0,Point(p1+_offset),Point(p2+_offset),color);
    
    imshow(kWinMain,_imctemp0);
    return waitKey(delay) ;
    
} // Viewer::DrawLine()

int Viewer::ShowTrack(const Track& track, const Scalar& color, int delay)
{
    Point2i cntr(track.blobs[0].rect.x + track.blobs[0].centroidx,
                 track.blobs[0].rect.y + track.blobs[0].centroidy);
    for (int k=1;k<track.blobs.size();++k)
    {
        Point2i newcntr(track.blobs[k].rect.x + track.blobs[k].centroidx,
                        track.blobs[k].rect.y + track.blobs[k].centroidy);
        line(_imctemp0,Point(cntr+_offset),Point(newcntr+_offset),color);
        cntr = newcntr;
    }
    imshow(kWinMain,_imctemp0);
    return waitKey(delay) ;
    
}

int Viewer::ShowTrackZoom(const Track& track, const Scalar& color, int delay)
{
    Point2i cntr(track.blobs[0].rect.x + track.blobs[0].centroidx,
                 track.blobs[0].rect.y + track.blobs[0].centroidy);
    for (int k=1;k<track.blobs.size();++k)
    {
        Point2i newcntr(track.blobs[k].rect.x + track.blobs[k].centroidx,
                        track.blobs[k].rect.y + track.blobs[k].centroidy);
        line(_imctemp,Point(cntr+_offset),Point(newcntr+_offset),color);
        cntr = newcntr;
    }

    Mat imzoom = _imroi;
    resize( _imroi,imzoom,Size(),_zoom_scale,_zoom_scale );
    namedWindow(kWinZoom, CV_WINDOW_AUTOSIZE );
    imshow(kWinZoom,imzoom);
    return waitKey(delay) ;
}

void Viewer::MakeOutputImage(OutputArray im_out, const WindowText& wt, const vector<Track>& tracks, int frame1, int frame2)
{
	
    putText(_imc, wt.text, wt.origin, wt.font, wt.font_scale, CV_RGB(255,255,255));
	
	Point2i winTxtTrackOrg(wt.origin.x,wt.origin.y+wt.origin.y+4);
	stringstream ss;
	Size txtsize;
	int baseline;
	ss << "Tracks: ";
	txtsize = getTextSize(ss.str(), wt.font, wt.font_scale, 1, &baseline);
	putText(_imc, ss.str(), winTxtTrackOrg, wt.font, wt.font_scale, CV_RGB(255,255,255));
	
	int Ncolors = sizeof(kLineColors) / sizeof(Scalar);
	int Ntracks = tracks.size();
    
	//for (int j=ind1;j<ind2;++j)
    for (int j=0; j<Ntracks; ++j)
	{
        if ( tracks[j].frameEnd() < frame1 || frame2 < tracks[j].frameStart()  )
            continue;
		int iclr = j%Ncolors; // color index
		ss.clear();ss.str("");
		//ss << j;
        ss << tracks[j]._id;
		winTxtTrackOrg += Point2i(txtsize.width + 4, 0);
		putText(_imc, ss.str(), winTxtTrackOrg, wt.font, wt.font_scale, kLineColors[iclr]/255.0);
		txtsize = getTextSize(ss.str(), wt.font, wt.font_scale, 1, &baseline);
		
        int n = 0;
        while ( tracks[j].blobs[n].frame < frame1 )
            ++n;
		Point2i cntr(tracks[j].blobs[n].rect.x + tracks[j].blobs[n].centroidx,
					 tracks[j].blobs[n].rect.y + tracks[j].blobs[n].centroidy);
		for (int k=n+1;k<tracks[j].blobs.size();++k)
		{
			Point2i newcntr(tracks[j].blobs[k].rect.x + tracks[j].blobs[k].centroidx,
							tracks[j].blobs[k].rect.y + tracks[j].blobs[k].centroidy);
			line(_imc, cntr, newcntr, kLineColors[iclr]/255.0, 2);
			cntr = newcntr;
		}
		circle(_imc, cntr, 8, kLineColors[iclr]/255.0, -1); // draw filled circle at the end of the track
	}
	
	Mat imc2;
	_imc.convertTo(imc2, CV_8U, 255, 0);
	cvtColor(imc2, im_out, CV_BGR2RGB);
	
} // Viewer::MakeOutputImage()