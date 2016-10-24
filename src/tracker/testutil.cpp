/*
 *  testutil.cpp
 *  
 *
 *  Created by Shari Matzner on 1/3/13.
 *  Copyright 2013 Pacific Northwest National Laboratory. All rights reserved.
 *
 */
#include <iostream> // cout, cin, cerr

#include "utilities.hpp"
#include "thermaltracker.hpp"

using namespace std;

int main (int argc, char * const argv[]) {
	// TO DO:  make function to test an input argument.
	
	//--------------------------------------------------------------------------------------
	// Test find
	cout << "testing find" << endl;
	Mat tester = Mat::eye(4,4,CV_32F);
	std::vector<unsigned> findResult;
	long Nfound;
	
	Nfound = find(tester, findResult);
	cout << "nonzero elements of 4x4 identity matrix" << endl;
	cout << "found: " << Nfound << ", expected: 4" << endl;
	cout << "indices of nonzero elements: " << endl;
	for (int i=0; i<Nfound; i++)
	{
		cout << findResult[i] << endl;
	}
	cout << endl;
	
	//--------------------------------------------------------------------------------------
	// Test intersect.
	cout << "testing intersect" << endl;
	std::vector<int> v1;  
	std::vector<int> v2;
	std::vector<unsigned> i1, i2;
	
	//6     8    10    12    14    16    18    20    22    24    26
	for (int j=0; j<10; j++)
		v1.push_back(2*j+6);
	//3     6     9    12    15    18    21    24    27
	for (int j=0; j<8; j++)
		v2.push_back(3*j+3);
	
	cout << "v1 = ";
	for (int j=0; j<v1.size(); ++j)
		cout << v1[j] << " " ;
	cout << endl;
	cout << "v2 = ";
	
	cout << "expecting: 6 12 18 24" << endl;
	
	for (int j=0; j<v2.size(); ++j)
		cout << v2[j] << " " ;
	cout << endl;
	
	int N = intersect<int>(v1,v2, i1, i2);
	
	cout << "intersection has " << N << " elements:  " ;
	for (int j=0; j<N; ++j)
		cout << v1[i1[j]] << " " << v2[i2[j]] << " " ;
	cout << endl;
	
	
	cout << endl;
	
	//--------------------------------------------------------------------------------------
	// Test idx2pnt.
	cout << "testing indx2pnt" << endl;
	Point2i pnt;
	
	pnt = idx2pnt(10, 3);
	cout << "idx = 10, cols = 3 : expected [1,3], got [" << pnt.x << "," << pnt.y << "]" << endl;
	pnt = idx2pnt(10, 5);
	cout << "idx = 10, cols = 5 : expected [0,2], got [" << pnt.x << "," << pnt.y << "]" << endl;
	cout << endl;

	//--------------------------------------------------------------------------------------
	// Test pnt2idx.
	cout << "testing pnt2idx" << endl;
	int idx;
	
	idx = pnt2idx(Point2i(1,3), 3);
	cout << "point = [1,3], cols = 3 : expected 10, got " << idx << endl;
	idx = pnt2idx(Point2i(0,2), 5);
	cout << "point = [0,2], cols = 5 : expected 10, got " << idx << endl;
	cout << endl;
	
	//--------------------------------------------------------------------------------------
	// Test getNeighbors.
	cout << "testing getNeighbors" << endl;
	int j,k;
	int mrows = 3;
	int mcols = 4;
	std::vector<unsigned> outidx;
        Mat testmat = Mat::zeros(mrows, mcols, CV_32SC1);
	int *dptr = testmat.ptr<int>(0);
	for (j=0;j<testmat.total();j++)
	{
		dptr[j] = j;
	}
	cout << "test matrix (" << testmat.rows << " x " << testmat.cols << "):" << endl;
	for (j=0;j<mrows;j++) 
	{
		for (k=0;k<mcols;k++)
		{
			cout << "element (" << j << "," << k << "): " << testmat.at<int>(j,k) << " " << endl;
		}
		cout << endl;
	}

	// non-edge pixel
	idx = 5;
	getNeighbors(idx, mrows, mcols, outidx);
	cout << "non-edge pixel 5 neighbors, expected (0 1 2 4 6 8 9 10) :" << endl;
	cout << "got: ";
	for (int j=0; j<outidx.size(); j++)
	{
		cout << outidx[j] << " " ;
	}
	cout << endl;
	
	// non-edge pixel
	idx = 6;
	getNeighbors(idx, mrows, mcols, outidx);
	cout << "non-edge pixel 6 neighbors, expected (1 2 3 5 7 9 10 11) :" << endl;
	cout << "got: ";
	for (int j=0; j<outidx.size(); j++)
	{
		cout << outidx[j] << " " ;
	}
	cout << endl;
	
	// left-edge pixel
	idx = 4;
	getNeighbors(idx, mrows, mcols, outidx);
	cout << "left-edge pixel 4 neighbors, expected (0 1 5 8 9) :" << endl;
	cout << "got: ";
	for (int j=0; j<outidx.size(); j++)
	{
		cout << outidx[j] << " " ;
	}
	cout << endl;
	
	// right-edge pixel
	idx = 7;
	getNeighbors(idx, mrows, mcols, outidx);
	cout << "right-edge pixel 7 neighbors, expected (2 3 6 10 11) :" << endl;
	cout << "got: ";
	for (int j=0; j<outidx.size(); j++)
	{
		cout << outidx[j] << " " ;
	}
	cout << endl;
	
	// top-edge pixel
	idx = 2;
	getNeighbors(idx, mrows, mcols, outidx);
	cout << "top-edge pixel 2 neighbors, expected (1 3 5 6 7) :" << endl;
	cout << "got: ";
	for (int j=0; j<outidx.size(); j++)
	{
		cout << outidx[j] << " " ;
	}
	cout << endl;
	
	// bottom-edge pixel
	idx = 9;
	getNeighbors(idx, mrows, mcols, outidx);
	cout << "bottom-edge pixel 9 neighbors, expected (4 5 6 8 10) :" << endl;
	cout << "got: ";
	for (int j=0; j<outidx.size(); j++)
	{
		cout << outidx[j] << " " ;
	}
	cout << endl;
	
	// top-left corner pixel
	idx = 0;
	getNeighbors(idx, mrows, mcols, outidx);
	cout << "top-left corner pixel 0 neighbors, expected (1 4 5) :" << endl;
	cout << "got: ";
	for (int j=0; j<outidx.size(); j++)
	{
		cout << outidx[j] << " " ;
	}
	cout << endl;
	
	// bottom-left corner pixel
	idx = 8;
	getNeighbors(idx, mrows, mcols, outidx);
	cout << "bottom-left corner pixel 8 neighbors, expected (4 5 9) :" << endl;
	cout << "got: ";
	for (int j=0; j<outidx.size(); j++)
	{
		cout << outidx[j] << " " ;
	}
	cout << endl;
	
	// top-right corner pixel
	idx = 3;
	getNeighbors(idx, mrows, mcols, outidx);
	cout << "top-right corner pixel 3 neighbors, expected (2 6 7) :" << endl;
	cout << "got: ";
	for (int j=0; j<outidx.size(); j++)
	{
		cout << outidx[j] << " " ;
	}
	cout << endl;
	
	// bottom-right corner pixel
	idx = 11;
	getNeighbors(idx, mrows, mcols, outidx);
	cout << "bottom-right corner pixel 11 neighbors, expected (6 7 10) :" << endl;
	cout << "got: ";
	for (int j=0; j<outidx.size(); j++)
	{
		cout << outidx[j] << " " ;
	}
	cout << endl;

    //--------------------------------------------------------------------------------------
	// Test sortIndex
	cout << endl;
	cout << "Testing sortIndex" << endl;
    
    std::vector<Blob> blobs;
    
    Blob newerblob;
    
    newerblob.frame = 4;
    newerblob.pixIdx.assign(7,1);
    blobs.push_back(newerblob);
    
    newerblob.frame = 1;
    newerblob.pixIdx.assign(7,1);
    blobs.push_back(newerblob);

    newerblob.frame = 5;
    newerblob.pixIdx.assign(7,1);
    blobs.push_back(newerblob);

    newerblob.frame = 3;
    newerblob.pixIdx.assign(7,1);
    blobs.push_back(newerblob);

    newerblob.frame = 4;
    newerblob.pixIdx.assign(10,1);
    blobs.push_back(newerblob);


    std::vector<size_t> sortOrder = sortedIndex<Blob>(blobs);
    cout << "sorted order (1 3 4 0 2): ";
    for (int j=0; j<sortOrder.size();++j)
    {
        cout << sortOrder[j] << " ";
    }
    cout << endl;
    cout << endl;
    
    
    /*
	//--------------------------------------------------------------------------------------
	// Test compBlob
	cout << endl;
	cout << "Testing compblob" << endl;
	
	Blob blob1, blob2;
	
	// case: different frames
	blob1.frame = 1; 
	blob2.frame = 2;
	blob1.pixIdx.assign(7,1); // 7 pixels
	blob2.pixIdx.assign(10,2); // 10 pixels
	
	cout << "blob1 frame " << blob1.frame  << ", " << blob1.pixIdx.size() << " pixels" << endl;
	cout << "blob2 frame " << blob2.frame  << ", " << blob2.pixIdx.size() << " pixels" << endl;
	cout << "blob1 <= blob2 ? " << compBlob(blob1,blob2) << endl;
	cout << "blob2 <= blob1 ? " << compBlob(blob2,blob1) << endl;
	
	// case: same frames, different number of pixels
	blob2.frame = 1;
	cout << "blob1 frame " << blob1.frame  << ", " << blob1.pixIdx.size() << " pixels" << endl;
	cout << "blob2 frame " << blob2.frame  << ", " << blob2.pixIdx.size() << " pixels" << endl;
	cout << "blob1 <= blob2 ? " << compBlob(blob1,blob2) << endl;
	cout << "blob2 <= blob1 ? " << compBlob(blob2,blob1) << endl;
	
	// case: same frames, same number of pixels
	blob2.pixIdx.assign(7,2);
	cout << "blob1 frame " << blob1.frame  << ", " << blob1.pixIdx.size() << " pixels" << endl;
	cout << "blob2 frame " << blob2.frame  << ", " << blob2.pixIdx.size() << " pixels" << endl;
	cout << "blob1 <= blob2 ? " << compBlob(blob1,blob2) << endl;
	cout << "blob2 <= blob1 ? " << compBlob(blob2,blob1) << endl;
	
	cout << endl;
	return 0;
*/
		//--------------------------------------------------------------------------------------
		// Test elapsedVideoTime
		cout << endl;
		cout << "Testing elapsedVideoTime" << endl;
		
		double fps = 30.0;
		
		for (int f=0; f<300; f+=26)
		{
			cout << "frame " << f << " = " << elapsedVideoTime(f,fps) << endl;
		}
	
	// Test elapsedVideoTime2
	cout << endl;
	cout << "Testing elapsedVideoTime2" << endl;
	
	int fpm = fps * 60;
	int fph = fpm * 60;
	
	for (int f=0; f<2*fph; f+=15*fpm+7)
	{
		cout << "frame " << f << " = " << elapsedVideoTime2(f,fps) << endl;
	}
	
		return 0;													
	
}
