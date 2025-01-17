/*******************************************************************
*   main.cpp
*   KORAL
*
*	Author: Kareem Omar
*	kareem.omar@uah.edu
*	https://github.com/komrad36
*
*	Last updated Dec 27, 2016
*******************************************************************/
//
// ## Summary ##
// KORAL is a novel, extremely fast, highly accurate, scale- and
// rotation-invariant, CPU-GPU cooperative detector-descriptor.
//
// Detection is based on the author's custom multi-scale KFAST corner
// detector, with rapid bilinear interpolation performed by the GPU
// asynchronously while the CPU works on KFAST.
//
// ## Usage ##
// Basic use of KORAL is extremely easy, although, of course, for a
// larger high-performance pipeline, users will benefit from
// calling KORAL functions directly and modifying it to suit their needs.
//
// To detect and describe, simply #include "KORAL.h" and
// then do:
//
// 	    KORAL koral(scale_factor, scale_levels);
//      koral.go(image, width, height, KFAST_threshold);
//
// where scale_factor is the factor by which each scale leve
// is reduced from the previous, scale_levels is the total
// number of such scale levels used, image is a pointer to
// uint8_t (grayscale) image data, and KFAST_threshold
// is the threshold supplied to the KFAST feature detector.
//
// After this call, keypoints are avaiable in a vector at 
// koral.kps, while descriptors are available at
// koral.desc.
//
// Portions of KORAL require SSE, AVX, AVX2, and CUDA.
// The author is working on reduced-performance versions
// with lesser requirements, but as the intent of this work
// is primarily novel performance capability, modern
// hardware and this full version are highly recommended.
//
// Description is performed by the GPU using the novel CLATCH
// (CUDA LATCH) binary descriptor kernel.
//
// Rotation invariance is provided by a novel vectorized
// SSE angle weight detector.
//
// All components have been written and carefully tuned by the author
// for maximum performance and have no external dependencies. Some have
// been modified for integration into KORAL,
// but the original standalone projects are all availble on
// the author's GitHub (https://github.com/komrad36).
//
// These individual components are:
// -KFAST        (https://github.com/komrad36/KFAST)
// -CUDALERP     (https://github.com/komrad36/CUDALERP)
// -FeatureAngle (https://github.com/komrad36/FeatureAngle)
// -CLATCH       (https://github.com/komrad36/CLATCH)
//
// In addition, the natural next step of matching descriptors
// is available in the author's currently separate
// project, CUDAK2NN (https://github.com/komrad36/CUDAK2NN).
//
// A key insight responsible for much of the performance of
// this insanely fast system is due to Christopher Parker
// (https://github.com/csp256), to whom I am extremely grateful.
// 
// The file 'main.cpp' is a simple test driver
// illustrating example usage.It requires OpenCV
// for image read and keypoint display.KORAL itself,
// however, does not require OpenCV or any other
// external dependencies.
//
// Note that KORAL is a work in progress.
// Suggestions and improvements are welcomed.
//
// ## License ##
// The FAST detector was created by Edward Rosten and Tom Drummond
// as described in the 2006 paper by Rosten and Drummond:
// "Machine learning for high-speed corner detection"
//         Edward Rosten and Tom Drummond
// https://www.edwardrosten.com/work/rosten_2006_machine.pdf
//
// The FAST detector is BSD licensed:
// 
// Copyright(c) 2006, 2008, 2009, 2010 Edward Rosten
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met :
// 
// 
// *Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 
// *Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and / or other materials provided with the distribution.
// 
// *Neither the name of the University of Cambridge nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO,
// 	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// 	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// 	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING
// 		NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// 	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
//
//
// KORAL is licensed under the MIT License : https://opensource.org/licenses/mit-license.php
// 
// Copyright(c) 2016 Kareem Omar, Christopher Parker
// 
// Permission is hereby granted, free of charge,
// to any person obtaining a copy of this software and associated documentation
// files(the "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish, distribute,
// sublicense, and / or sell copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// 
// Note again that KORAL is a work in progress.
// Suggestions and improvements are welcomed.
//

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

#include "koral/KORAL.h"
#include "koral/Matcher.h"
#include "koral/Detector.h"

int main() {
	using namespace koral;
	
	// -------- Configuration ----------
	constexpr uint8_t KFAST_thresh = 60;
	constexpr char name[] = "test1.jpg";
	constexpr char name2[] = "test2.jpg";
	constexpr float scale_factor = 1.2f;
	constexpr uint8_t scale_levels = 8;
	// --------------------------------


	// ------------- Image Read -------
	cv::Mat image = cv::imread(name, cv::ImreadModes::IMREAD_GRAYSCALE);
	if (!image.data) {
		std::cerr << "ERROR: failed to open image. Aborting." << std::endl;
		return EXIT_FAILURE;
	}

	cv::Mat image2 = cv::imread(name2, cv::ImreadModes::IMREAD_GRAYSCALE);
	if (!image2.data) {
		std::cerr << "ERROR: failed to open image. Aborting." << std::endl;
		return EXIT_FAILURE;
	}
	// --------------------------------


	// ------------- KORAL ------------
	koral::KORAL koral(scale_factor, scale_levels);
	koral.go(image.data, image.cols, image.rows, KFAST_thresh);

	koral::KORAL koral2(scale_factor, scale_levels);
	koral2.go(image2.data, image2.cols, image2.rows, KFAST_thresh);
	// --------------------------------


	// ------------ Output ------------
	std::cout << "KORAL found " << koral.kps.size() << " keypoints and descriptors." << std::endl;
	std::cout << "KORAL2 found " << koral2.kps.size() << " keypoints and descriptors." << std::endl;

	cv::Mat image_with_kps;
	std::vector<cv::KeyPoint> converted_kps;
	for (const auto& kp : koral.kps) {
		// note that KORAL keypoint coordinates are on their native scale level,
		// so if you want to plot them accurately on scale level 0 (the original
		// image), you must multiply both the x- and y-coords by scale_factor^kp.scale,
		// as is done here.
		const float scale = static_cast<float>(std::pow(scale_factor, kp.scale));
		converted_kps.emplace_back(scale*static_cast<float>(kp.x), scale*static_cast<float>(kp.y), 7.0f*scale, 180.0f / 3.1415926535f * kp.angle, static_cast<float>(kp.score));
	}

	cv::Mat image2_with_kps;
	std::vector<cv::KeyPoint> converted_kps2;
	for (const auto& kp : koral2.kps) {
		// note that KORAL keypoint coordinates are on their native scale level,
		// so if you want to plot them accurately on scale level 0 (the original
		// image), you must multiply both the x- and y-coords by scale_factor^kp.scale,
		// as is done here.
		const float scale = static_cast<float>(std::pow(scale_factor, kp.scale));
		converted_kps2.emplace_back(scale*static_cast<float>(kp.x), scale*static_cast<float>(kp.y), 7.0f*scale, 180.0f / 3.1415926535f * kp.angle, static_cast<float>(kp.score));
	}

	cv::drawKeypoints(image, converted_kps, image_with_kps, cv::Scalar::all(-1.0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	cv::drawKeypoints(image2, converted_kps2, image2_with_kps, cv::Scalar::all(-1.0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	cv::namedWindow("KORAL", cv::WINDOW_NORMAL);
	cv::namedWindow("KORAL2", cv::WINDOW_NORMAL);
	cv::imshow("KORAL", image_with_kps);
	cv::imshow("KORAL2", image2_with_kps);
	cv::waitKey(0);

	const unsigned int maxFeatureCount = 50000;
	const uint8_t fastThreshold = 40;
	const uint8_t matchThreshold = 25;

	const unsigned int scaleLevels = 4;
	const float scaleFactor = 1.2;
	koral::FeatureDetector detector(scaleFactor, scaleLevels, 900, 1100, maxFeatureCount, fastThreshold);
	koral::FeatureMatcher matcher(matchThreshold, maxFeatureCount);
	
	cv::Rect crop(1470, 1350, 900, 1100);
	cv::Mat img1, img2;
	img1 = image(crop).clone();
	img2 = image2(crop).clone();

	cv::Mat image_with_kps_L, image_with_kps_R;
	std::vector <cv::KeyPoint> kpsL, kpsR;

	detector.extractFeatures(img1);
	matcher.setTrainingImage(detector.kps, detector.desc);
	cv::drawKeypoints(img1, detector.converted_kps, image_with_kps_L, cv::Scalar::all(-1.0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	kpsL = detector.converted_kps;

	detector.extractFeatures(img2);
	matcher.setQueryImage(detector.kps, detector.desc);
	cv::drawKeypoints(img2, detector.converted_kps, image_with_kps_R, cv::Scalar::all(-1.0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	kpsR = detector.converted_kps;
	
	matcher.matchFeatures();
	cv::Mat image_with_matches;
	cv::drawMatches(image_with_kps_L, kpsL, image_with_kps_R, kpsR, matcher.dmatches, image_with_matches, cv::Scalar::all(-1.0), cv::Scalar::all(-1.0), std::vector<char>(), cv::DrawMatchesFlags::DEFAULT);
	cv::namedWindow("Matches", cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
	cv::imshow("Matches", image_with_matches);
	cv::waitKey(0);
	// --------------------------------

	
	// Descriptors are available in koral.desc
	// as a contiguous block of 512-bit binary
	// LATCH descriptors.
}
