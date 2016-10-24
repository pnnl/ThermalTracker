/*
 *  utilities.hpp
 *  
 *
 *  Created by Shari Matzner on 1/3/13.
 *  Copyright 2013 Pacific Northwest National Laboratory. All rights reserved.
 *
 */

#ifndef __PNNL_UTILITIES_HPP__
#define __PNNL_UTILITIES_HPP__

#include <opencv2/opencv.hpp>

//#include "thermaltracker.hpp"

using namespace cv;

// returns -1, 0 or 1 as int
template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

// Similar to Matlab find function
// src must be a logical array, matrix, or vector.
// out is a vector of indices of the nonzero elements
// of src.
long find(InputArray src, std::vector<unsigned>& out);

// similar to Matlab intersect function
// returns the number of elements in the intersection of v1 and v2.
// idx1 and idx2 are set to the indices in each vector of the common elements.
template<typename T>
int intersect(const std::vector<T>& v1, const std::vector<T>& v2, std::vector<unsigned>& ind1, std::vector<unsigned>& ind2)
{
	int Nintersect = 0;
	
	ind1.clear();
	ind2.clear();
	
	int N1 = v1.size();
	int N2 = v2.size();
	int i1 = 0, i2 = 0;
	
	// from std::set_intersection reference
	while (i1<N1 && i2<N2)
	{
		if (v1[i1]<v2[i2]) ++i1;
		else if (v2[i2]<v1[i1]) ++i2;
		else { // the elements are equal
			++Nintersect;
			ind1.push_back(i1);
			ind2.push_back(i2);
			++i1; ++i2;
		}
	}
	return Nintersect;
}

// Similar to Matlab ind2sub function.
Point2i idx2pnt(int idx, int mcols);

// Similar to Matlab sub2ind function.
int pnt2idx(Point2i pnt, int mcols);

// Get a list of the indices of the neighbors of the given pixel.
// A non-edge pixel has 8 neighbors, edge pixels have less.
void getNeighbors(int idx, int mrows, int mcols, std::vector<unsigned>& outidx);


// Return the indices of a vector in sorted order
template<typename T>
class CompareIndicesByVectorValues {
    std::vector<T>* _values;
public:
    CompareIndicesByVectorValues( std::vector<T>* values) : _values(values) {};
public:
    bool operator() (const int& a, const int& b) const { return (*_values)[a] < (*_values)[b]; }
};

template <typename T>
std::vector<size_t> sortedIndex( std::vector<T> &v) {
    
    // initialize original index locations
	std::vector<size_t> idx(v.size());
    for (size_t i = 0; i != idx.size(); ++i) idx[i] = i;
    
    // sort indexes based on comparing values in v
    sort(idx.begin(), idx.end(),
         CompareIndicesByVectorValues<T>(&v));
    
    return idx;
};

template <typename T>
std::vector<size_t> sortedIndexDescending( std::vector<T> &v) {
    
    // initialize original index locations
	std::vector<size_t> idx(v.size());
    for (size_t i = 0; i != idx.size(); ++i) idx[i] = i;
    
    // sort indexes based on comparing values in v
    sort(idx.rbegin(), idx.rend(),
         CompareIndicesByVectorValues<T>(&v));
    
    return idx;
};

std::string elapsedVideoTime(int frame, double fps);
std::string elapsedVideoTime2(int frame, double fps);
void generate_outpath_str(const std::string vfilepath_str, std::string& outpath_str);

#endif // __PNNL_UTILITIES_HPP__

