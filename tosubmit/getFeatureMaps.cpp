/*****************************************************************************
** DARWIN: A FRAMEWORK FOR MACHINE LEARNING RESEARCH AND DEVELOPMENT
** Distributed under the terms of the BSD license (see the LICENSE file)
** Copyright (c) 2007-2013, Stephen Gould
** All rights reserved.
**
******************************************************************************
** @file getFeatureMap.cpp
** @version 1.0
** @since 2013-05-16
** @author Jimmy Lin (u5223173) - u5223173@uds.anu.edu.au
**
** Edited by MacVim
** Info auto-generated by Snippet 
*****************************************************************************/

// c++ standard headers
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>

// eigen matrix library headers
#include "Eigen/Core"

// opencv library headers
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

// darwin library headers
#include "drwnBase.h"
#include "drwnIO.h"
#include "drwnML.h"
#include "drwnVision.h"
#include "features.h"

using namespace std;
using namespace Eigen;

// usage ---------------------------------------------------------------------

// copied from Stephen Gould's trainCOMP3130Model.cpp 2013 version
void usage() {
    cerr << DRWN_USAGE_HEADER << endl;
    cerr << "USAGE: ./getFeatureMap <mode> <imgDir> <outputDir>\n";
    cerr << "OPTIONS:\n"
         << "  -x                :: visualize\n"
         << DRWN_STANDARD_OPTIONS_USAGE
	 << endl;
}

// ---------------------------------------------------------------------------
int main (int argc, char * argv[]) {

    // Set default value for optional command line arguments.
    const char *modelFile = NULL;
    bool bVisualize = false;
    bool pyramidDisplay = false;

    DRWN_BEGIN_CMDLINE_PROCESSING(argc, argv)
        DRWN_CMDLINE_STR_OPTION("-o", modelFile)
        DRWN_CMDLINE_BOOL_OPTION("-x", bVisualize)
        DRWN_CMDLINE_BOOL_OPTION("-p", pyramidDisplay)
    DRWN_END_CMDLINE_PROCESSING(usage());

    // Check for the correct number of required arguments
    if (DRWN_CMDLINE_ARGC != 3) {
        usage();
        return -1;
    }

    /* Check that the image directory and labels directory exist. All
     * images with a ".jpg" extension will be used for training the
     * model. It is assumed that the labels directory contains files
     * with the same base as the image directory, but with extension
     * ".txt". 
     */
    const char *modeSwitch = DRWN_CMDLINE_ARGV[0]; // msc, csh, csd
    const char *imgDir = DRWN_CMDLINE_ARGV[1];
    const char *outputDir = DRWN_CMDLINE_ARGV[2];

    // intepret which feature to extract
    string mode;
    if (string(modeSwitch).compare("msc") == 0 ) {
        mode = "Multiscale Contrast";
    } else if (string(modeSwitch).compare("csh") == 0 ) {
        mode = "Center Surround Histogram";
    } else if (string(modeSwitch).compare("csd") == 0 ) {
        mode = "Color Spatial Distribution";
    }

    // Check the existence of the given directory
    DRWN_ASSERT_MSG(drwnDirExists(imgDir), "image directory " << imgDir << " does not exist");
    DRWN_ASSERT_MSG(drwnDirExists(outputDir), "output directory " << outputDir << " does not exist");

    // Get a list of images from the image directory.
    vector<string> baseNames = drwnDirectoryListing(imgDir, ".jpg", false, false);
    DRWN_LOG_MESSAGE("Loading " << baseNames.size() << " images and labels...");

    /* Build a dataset by loading images and labels. For each image,
     find the salient area using the labels and then compute the set of features
     that determine this saliency.  
    */
    drwnClassifierDataset dataset;
    //  MAP FROM FILENAME TO RECTANGLE

    for (unsigned i = 0; i < baseNames.size(); i++) {
        String processedImage = baseNames[i] + ".jpg";
        DRWN_LOG_STATUS("...processing image " << baseNames[i]);
        // read the image and draw the rectangle of labels of training data
        cv::Mat img = cv::imread(string(imgDir) + DRWN_DIRSEP + processedImage);

        // show the image and feature maps 
        if (bVisualize) { // draw the current image comparison
            //drwnDrawRegionBoundaries and drwnShowDebuggingImage use OpenCV 1.0 C API
            IplImage cvimg = (IplImage)img;
            IplImage *canvas = cvCloneImage(&cvimg);
            drwnShowDebuggingImage(canvas, "image", false);
            cvReleaseImage(&canvas);
        }
        // get processed by feature.h
        cv::Mat cdi;   
        if (string(modeSwitch).compare("msc") == 0 ) {
            // halfwindowSize:5 , pyramid level: 6
            MultiScaleContrast mscObj = getMultiScaleContrast(img, 5, 6);
            cdi = mscObj.featureMap;  
            // output the pyramid
            if (pyramidDisplay) {
                for (int p = 0; p < mscObj.nPyLevel; p ++) {
                    cv::imwrite(string(outputDir) + baseNames[i] + "_p" + toString(p) + ".jpg", mscObj.PyContrastMaps[p]);
                }
            }
        } else if (string(modeSwitch).compare("csh") == 0 ) {
            cdi = getCenterSurround(img); 
        } else if (string(modeSwitch).compare("csd") == 0 ) {
            cdi = getSpatialDistribution(img);
        }
        cv::Mat pres (img.rows, img.cols, CV_8UC3);
        double grayscale;
        for (int y = 0 ; y < cdi.rows; y ++) {
            for (int x = 0 ; x < cdi.cols; x ++) {
                grayscale = cdi.at<double>(y,x);
                pres.at<Vec3b>(y,x) = Vec3b(grayscale*255, grayscale*255, grayscale*255);
            }
        }
        IplImage pcvimg = (IplImage) pres;
        IplImage *present = cvCloneImage(&pcvimg);
        cv::imwrite(string(outputDir) + baseNames[i] + ".jpg", pres);
        if (bVisualize) { // draw the processed feature map and display it on the screen
            drwnShowDebuggingImage(present, mode.c_str(), false);
            cvReleaseImage(&present);
        }
    }

}

