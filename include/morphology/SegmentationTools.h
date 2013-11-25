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

#ifndef __MORPHOLOGY_SEGMENTATION_TOOLS_H
#define __MORPHOLOGY_SEGMENTATION_TOOLS_H

#include <vector>

#include <opencv2/core/core.hpp>

#include <morphology/Attributes.h>
#include <morphology/AttributeFilter.h>
#include <morphology/Timer.h>

namespace morphology
{
    const int LAMBDA = 5000;

    /**
     * Computes attribute granulometry for a given
     * attribute on a given image up to lambda.
     */
    template <typename A>
    std::vector<int> computeGranulometry(const cv::Mat& img, unsigned int lambda)
    {
        AttributePatternSpectrum<A> aps;
        std::vector<int> attribute_spectrum;
        {
            Timer t("attribute granulometry");
            attribute_spectrum = aps.close(img, lambda);
        }
        return attribute_spectrum;
    }

    /**
     * Computes the peak of the granulometry
     */
    template <typename A>
    int ultimateAttribute(const cv::Mat& img)
    {
        std::vector<int> spectrum = computeGranulometry<Area>(img, LAMBDA);
        const std::vector<int>::iterator max_deflection = max_element(spectrum.begin(), spectrum.end());
        return distance(spectrum.begin(), max_deflection);
    }

    /**
     * Computes an ultimate attribute opening for
     * given image. Other than image, this operator
     * is parameter-free unless the caller specifies
     * alpha and epsilon.
     */
    template <typename A>
    cv::Mat ultimateAttributeClosing(const cv::Mat& img, const double alpha = 1.0, const double epsilon = 0.0)
    {
        Timer t("ultimate attribute closing");

        // Estimate ultimate attribute.
        const int attribute = ultimateAttribute<A>(img);

        AttributeFilter<A> attribute_filter;

        // Remove grain and dirt from the image and
        // separate cells from each other.
        cv::Mat i;
        {
            Timer t("attribute closing");
            i = attribute_filter.close(img, attribute * alpha - epsilon);
        }

        // Close the entire image. This is the
        // background model.
        cv::Mat i_bg;
        {
            Timer t("background model");
            i_bg = attribute_filter.close(img, 2 * LAMBDA);
        }

        // Cells are darker than background, so
        // remove i from the closed background.
        return i_bg - i;
    }
}
#endif // __MORPHOLOGY_SEGMENTATION_TOOLS_H
