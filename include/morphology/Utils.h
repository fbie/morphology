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

#ifndef __MORPHOLOGY_UTILS_H
#define __MORPHOLOGY_UTILS_H

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

#include <opencv2/core/core.hpp>

namespace morphology
{
    /**
     * @returns true if one of the neighbors (union of raster
     * and anti-raster direction) of p is zero.
     */
    inline bool isBoundary(const uchar* p, const uchar step)
    {
        const uchar neighbors[] = {*(p - step - 1), *(p - step), *(p - step + 1),
                                   *(p - 1), *(p + 1),
                                   *(p + step - 1), *(p + step), *(p + step + 1)};
        for (int i = 0; i < 8; i++) {
            if (neighbors[i] == 0) {
                return true;
            }
        }
        return false;
    }

    /**
     * @returns the negative of an image.
     */
    inline cv::Mat negative(const cv::Mat& src)
    {
        return (cv::Mat::ones(src.size(), src.type()) * 255) - src;
    }

    inline cv::Mat& negative(cv::Mat& dst)
    {
        const uchar step = dst.step[0] / dst.step[1];

        for (int y = 0; y < dst.rows - 1; y++) {
            uchar* p = dst.ptr(y);
            const uchar* p_end = p+ dst.cols;

            while (p != p_end) {
                *p = 255 - *p;
                p++;
            }
        }
        return dst;
    }

    /**
     * @returns the radius of this area.
     */
    inline int to_radius(unsigned int area)
    {
        return static_cast<int>(sqrt(static_cast<double>(area) / M_PI) + 0.5);
    }

    /**
     * @returns the area of this radius.
     */
    inline int to_area(unsigned int radius)
    {
        return static_cast<int>(M_PI * radius * radius + 0.5);
    }
}

#endif // __MORPHOLOGY_UTILS_H
