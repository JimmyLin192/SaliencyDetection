/*****************************************************************************
** DARWIN: A FRAMEWORK FOR MACHINE LEARNING RESEARCH AND DEVELOPMENT
** Distributed under the terms of the BSD license (see the LICENSE file)
** Copyright (c) 2007-2013, Stephen Gould
** All rights reserved.
**
******************************************************************************
** FILENAME:    2550Common.h
** AUTHOR(S):   Stephen Gould <stephen.gould@anu.edu.au>
**              Jimmy Lin <u5223173@uds.anu.edu.anu>
**              Chris Claoue-Long <u5183532@anu.edu.au>
*****************************************************************************/
#include <cmath>
#include <set>
using namespace std;
using namespace Eigen;

// feature extraction algorithms -----------------------------------------------

// Average intensity of an image (for contrast mapping) TODO NOT QUITE FINISHED.
float avgintensity(cv::Mat img){
    float avgint = 0.0;
    
}


// Get the contrast of the image
cv::Mat contrast(cv::Mat img){
    cv::Mat contrasted = img; // initialised to the same thing to begin with
    float avgint = avgintensity(img);
    float normaliser = 1/(img.rows * img.cols);
    
    double intpix;
    Vec3b intensity;
    for (int y = 0; y < img.rows; y++){
        for (int x = 0; x < seg.cols; x++){
        
            intensity = img.at<Vec3b>(y,x);
            intpix = 0; // reset to 0, new pixel
            for(int i = 0; i < 3; i++){
                intpix += intensity.val[i];
            }
            intpix = abs(intpix - avgint);
        }
    }

    return contrasted;
}

// Get the multiscale contrast map of the image (to 6 scales) 
// TODO NOT QUITE FINISHED.
cv::Mat multiScaleContrast(cv::Mat img){
    cv::Mat msc = img; // the return matrix, initialised to the input by default
    cv::Mat cont;
    cv::Mat tmp;
    cv::Mat dst;
    vector<cv::Mat> pyramid;
    pyramid.resize(6); // 6 images in this gaussian pyramid
    
    cont = contrast(img);
    
    pyramid[0] = cont; // the original contrasted image, base of the gaussian pyramid
    tmp = cont;
    dst = tmp; // initialised
    for(int i = 1; i < 6; i++{
        pyrDown(tmp, dst, Size(tmp.cols/2, tmp.rows/2) );
        pyramid[i] = tmp;
        tmp = dst; // to perform gaussian modelling again
    }
    
    // run the multiscale contrast thingummy to flatten the image, put into msc
    return msc;   
}

// Get the value from a center-surround histogram
struct CentreSurround {
    double dist;
    vector<int> rect;
};

// rect1 is (xval, yval, widthtoright, heightdown)
CentreSurround centreSurround(cv::Mat img, vector<int> rect1, int bins){
    CentreSurround csv; // centre-surround distance and appropriate rectangle
    vector<vector<int>> histogramBins1(bins);
    vector<vector<int>> histogramBins2(bins);
    int binDelimiter=255/bins;
    Vec3b pixColour;
    
    // initialise to 0 for all bins in the histogram
    for(long i = 0; i < bins; i++){
        histogramBins1.at(i).resize(3);
        histogramBins1.at(i)[0] = 0;
        histogramBins1.at(i)[1] = 0;
        histogramBins1.at(i)[2] = 0;
        histogramBins2.at(i).resize(3);
        histogramBins2.at(i)[0] = 0;
        histogramBins2.at(i)[1] = 0;
        histogramBins2.at(i)[2] = 0;
    }
    
    // create a rectangle around rect1 such that its area - area of rect1 is equal to that of rect1
    // for the moment, a really approximate method so we have something working.
    // we can maybe modify this to do away with correct aspect ratio choice by some clever formula too...?
    
    vector<int> rect2(4);
    rect2.at(0)=rect1.at(0)-(rect1.at(2)/2);
    if(rect2.at(0)<0) { rect2.at(0)=0; } // ensure positive values only
    rect2.at(1)=rect1.at(1)-(rect1.at(3)/2);
    if(rect2.at(1)<0) { rect2.at(1)=0; }
    rect2.at(2)=rect1.at(2)*2;
    rect2.at(3)=rect1.at(3)*2;
    
    // perform RGB histogram calculation
    for(int y = 0; y < img.rows; y++){
        for(int x = 0; x < img.cols; x++){
            if (y < rect2.at(1) | y > rect2.at(1)+rect2.at(3) | x < rect2.at(0) | x > rect2.at(0)+rect2.at(2) ){ continue; }
            pixColour=img.at<Vec3b>(y,x);
            // TODO convert pixColour into correct bin delimiters for its B, G and R values

            if (y >= rect1.at(1) & y <= rect1.at(1)+rect1.at(3) & x >= rect1.at(0) & x <= rect1.at(2) ) { // pixel in rect1
                histogram1.at(pixColour[0])[0]++;
                histogram1.at(pixColour[1])[1]++;
                histogram1.at(pixColour[2])[2]++;
            } else { // pixel in rect2
                histogram2.at(pixColour[0])[0]++;
                histogram2.at(pixColour[1])[1]++;
                histogram2.at(pixColour[2])[2]++;
            }   
        }

    }
    
    // calculate the chi-squared value
    float sum = 0.0;
    for(int i = 0; i < bins; i++){
        for(int j = 0; j < 3; j++{
            sum+= (pow((histogram1.at(i)[j]-histogram2.at(i)[j]), 2))/(histogram1.at(i)[j]+histogram2.at(i)[j])
        }
    }
    csv.dist = sum/2 // calculated 1/2*sum_i[(histR1_i-histR2_i)^2/(histR1_i+histR2_i)]
    csv.rect = rect2;
    return csv;
}

// Get the colour spatial distribution as a gaussian mixture model
cv::Mat colourDist(cv::Mat img){
    cv::Mat cdi;
    
    vector<vector<double> > features = getFeatures(img);
    drwnGaussianMixture gmm(features[0].size(), 10); // 10? mixture components
    gmm.train(features); // train the mixture model on the features given
    
    // generate 10 samples from the model
    vector<double> s;
    for (int i = 0; i < 10; i++) {
        gmm.sample(s);
        DRWN_LOG_MESSAGE("sample " << (i + 1) << " is " << toString(s));
    }
    
    return cdi;
}

vector<vector<double> > getFeatures(cv::Mat img){
    vector<vector<double> > featureList;
    featureList.resize(img.rows*img.cols);
    for(int y = 0; y < img.rows; y++){
        for(int x = 0; x < img.cols; x++{
            // CV library feature calculations on pixels?
        }
    }
    
    return featureList;
}