/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 Florian Biermann, fbie@itu.dk
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __MORPHOLOGY_RECONSTRUCTION_H
#define __MORPHOLOGY_RECONSTRUCTION_H

#include "config.h"

#include <opencv2/core/core.hpp>

namespace morphology
{
    /**
     * Grey-scale reconstruction algorithms implemented after
     * L. Vincent, "Morphological grayscale reconstruction in
     * image analysis: applications and efficient algorithms",
     * IEEE Transactions on Image Processing, 1993.
     */

    cv::Mat MORPHOLOGY_EXPORT parallelReconstruct(const cv::Mat& marker, const cv::Mat& mask);

    /**
     * Sequential grey-scale reconstruction.
     *
     * The reconstruction scans repeatedly over the image, in and
     * against raster direction, successively reconstructing it
     * from a given marker image until the number of changes in
     * an iteration hits zero.
     *
     * @returns The reconstructed image.
     */
    cv::Mat MORPHOLOGY_EXPORT sequentialReconstruct(const cv::Mat& marker, const cv::Mat& mask);

    /**
     * Fifo-queue based grey-scale reconstruction.
     *
     * The reconstruction uses a fifo-queue to manage pixels instead
     * of scanning the entire image.
     *
     * @returns The reconstructed image.
     */
    cv::Mat MORPHOLOGY_EXPORT queueReconstruct(const cv::Mat& marker, const cv::Mat& mask);

    /**
     * Hybrid grey-scale reconstruction.
     *
     * Uses sequential reconstruction and queue reconstruction for
     * optimal performance.
     *
     * @returns The reconstructed image.
     */
    cv::Mat MORPHOLOGY_EXPORT hybridReconstruct(const cv::Mat& marker, const cv::Mat& mask);


    /**
     * These functions compute the h-domes or -basins, respectively,
     * which are equivalent to regional maxima and minima.
     */
    cv::Mat MORPHOLOGY_EXPORT computeHDomes(const cv::Mat& src, uchar h);
    cv::Mat MORPHOLOGY_EXPORT computeHBasins(const cv::Mat& src, uchar h);
}

#endif // __MORPHOLOGY_RECONSTRUCTION_H
