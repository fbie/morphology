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

#include <morphology/Reconstruction.h>

#include <queue>
#include <omp.h>

#include <morphology/Utils.h>

using namespace cv;

namespace morphology
{
    namespace
    {
        /**
         * Pairs a pixel with a non-const pointer to another pixel
         * and stores their respective coordinates.
         */
        struct PixelPair
        {
            const uchar* p_i;
            uchar* p_j;
            cv::Point coords;

            PixelPair () : p_i(0), p_j(0) {}

            PixelPair(const uchar* _p_i, uchar* _p_j, const int _x, const int _y) :
                p_i(_p_i), p_j(_p_j), coords(_x, _y) {}
        };

        /**
         * @returns a queue of pixels representing the
         * boundaries of the markers on image j. Keeps
         * a one-pixel margin around the image.
         */
        inline std::queue<PixelPair> initPixelQueue(const Mat& mask, Mat& marker)
        {
            std::queue<PixelPair> fifo;
            const uchar step = marker.step[0] / marker.step[1];

            for (int y = 1; y < marker.rows - 1; y++) {
                const uchar* p_mask = mask.ptr(y) + 1;
                uchar* p_marker = marker.ptr(y) + 1;
                const uchar* p_marker_end = p_marker + marker.cols - 1;

                int x = 1;
                while (p_marker != p_marker_end) {
                    // A boundary masks defined as a pixel that itself
                    // is not zero and that has at least one neighbor
                    // with value zero.
                    if (*p_marker > 0 && isBoundary(p_marker, step)) {
                        fifo.push(PixelPair(p_mask, p_marker, x, y));
                    }
                    p_mask++;
                    p_marker++;
                    x++;
                }
            }
            return fifo;
        }

        /**
         * @returns the maximum value of the neighborhood
         * in given raster direction.
         */
        inline uchar computeMaxNeighbor(const uchar* p, const uchar step, const int direction)
        {
            const int direction_step = - direction * step;
            uchar neighbors[] = {*(p + direction),
                    *(p - 1 + direction_step),
                    *(p     + direction_step),
                    *(p + 1 + direction_step)};
            uchar max = neighbors[0];
            for (int i = 1; i < 4; i++) {
                if (neighbors[i] > max) {
                    max = neighbors[i];
                }
            }
            return max;
        }

        inline uchar computeMaxNeighbor(const uchar* p, const uchar step)
        {
            return std::max(computeMaxNeighbor(p, step, 1), computeMaxNeighbor(p, step, -1));
        }

        /**
         * Scans in given raster direction over the image,
         * performing one reconstruction step. The function
         * spares out the margin pixels, i.e. there is a
         * one-pixel border around the image which is not
         * being reconstructed.
         *
         * @returns the number of changes made in this scan
         */
        inline void rasterReconstruct(const int direction, const Mat& i, Mat& j)
        {
            CV_Assert(direction == 1 || direction == -1);

            const uchar step = j.step[0] / j.step[1];

            // Start at bottom if direction is negative
            const int start_y = direction < 0 ? j.rows - 1 : 1;
            const int stop_y = direction < 0 ? 1 : j.rows - 1;

            for (int y = start_y; y != stop_y; y += direction) {
                const uchar* p_i = i.ptr(y) + 1;
                uchar* p_j = j.ptr(y) + 1;
                const uchar* p_j_end = p_j;

                // Determine start pointers
                if (direction < 0) {
                    p_i += i.cols - 1;
                    p_j += j.cols - 1;
                    p_j_end += 1;
                } else {
                    p_j_end += j.cols - 1;
                }

                while (p_j != p_j_end) {
                    uchar value = std::min(std::max(computeMaxNeighbor(p_j, step, direction), *p_j), *p_i);
                    if (value != *p_j) {
                        *p_j = value;
                    }
                    p_i += direction;
                    p_j += direction;
                }
            }
        }

    } // namespace

    Mat parallelReconstruct(const Mat& marker, const Mat& mask)
    {
        CV_Assert(marker.type() == CV_8U && mask.type() == CV_8U);

        // Images must be of the same size
        CV_Assert(marker.size == mask.size);
        CV_Assert(sum(marker)[0] < sum(mask)[0]);

        Mat j = marker.clone();
        Mat k = Mat::zeros(marker.size(), marker.type());

        const uchar step = j.step[0] / j.step[1];

        Scalar stability;
        while (stability != sum(j)) {
            stability = sum(j);

            // Dilation step
#pragma omp for
            for (int y = 1; y < k.rows; y++) {
                const uchar* p_j = j.ptr(y) + 1;
                uchar* p_k = k.ptr(y) + 1;
                const uchar* p_k_end = p_k + k.cols - 1;

                while (p_k != p_k_end) {
                    *p_k = std::max(computeMaxNeighbor(p_j, step), *p_j);
                    p_j++;
                    p_k++;
                }
            }

            // Point-wise minimum
#pragma omp for
            for (int y = 1; y < j.rows; y++) {
                const uchar* p_i = mask.ptr(y) + 1;
                uchar* p_j = j.ptr(y) + 1;
                const uchar* p_k = k.ptr(y) + 1;
                const uchar* p_j_end = p_j + j.cols - 1;

                while (p_j != p_j_end) {
                    *p_j = std::min(*p_i, *p_k);
                    p_i++;
                    p_j++;
                    p_k++;
                }
            }
        }
        return j;
    }

    Mat sequentialReconstruct(const Mat& marker, const Mat& mask)
    {
        CV_Assert(marker.type() == CV_8U && mask.type() == CV_8U);

        // Images must be of the same size
        CV_Assert(marker.size == mask.size);

        Mat j = marker.clone();

        // Scan back and forth over the image
        // until no changes made.
        Scalar stability;
        while (stability != sum(j)){
            stability = sum(j);
            rasterReconstruct(1, mask, j);
            rasterReconstruct(-1, mask, j);
        }
        return j;
    }

    Mat queueReconstruct(const Mat& marker, const Mat& mask)
    {
        CV_Assert(marker.type() == CV_8U && mask.type() == CV_8U);

        // Images must be of the same size
        CV_Assert(marker.size == mask.size);

        Mat k = marker.clone();
        std::queue<PixelPair> fifo = initPixelQueue(mask, k);

        // Iterate over fifo instead of image.
        const uchar step = k.step[0] / k.step[1];
        while (!fifo.empty()) {
            PixelPair t = fifo.front();
            fifo.pop();

            const int x_lower = std::max(t.coords.x - 1, 0);
            const int x_upper = std::min(t.coords.x + 1, mask.cols - 1);
            const int y_lower = std::max(t.coords.y - 1, 0);
            const int y_upper = std::min(t.coords.y + 1, mask.rows - 1);

            // For each neighbor...
            for (int y = y_lower; y <= y_upper; y++) {
                for (int x = x_lower; x <= x_upper; x++) {

                    const uchar* q_i = t.p_i - step + y * step - 1 + x;
                    uchar* q_j = t.p_j - step + y * step - 1 + x;

                    // Don't compare pixel with itself!
                    if (q_i == t.p_j && q_j == t.p_i) {
                        continue;
                    }

                    if (*q_j < *t.p_j && *q_i != *q_j) {
                        *q_j = std::min(*t.p_j, *q_i);
                        fifo.push(PixelPair(q_i, q_j,
                                t.coords.x - 1 + static_cast<int>(x),
                                t.coords.y - 1 + static_cast<int>(y)));
                    }
                }
            }
        }
        return k;
    }

    Mat hybridReconstruct(const Mat& marker, const Mat& mask)
    {
        Mat k = marker.clone();
        rasterReconstruct(1, mask, k);
        rasterReconstruct(-1, mask, k);
        return queueReconstruct(k, mask);
    }

    Mat computeHDomes(const Mat& src, uchar h)
    {
        CV_Assert(src.type() == CV_8U);
        return src - hybridReconstruct(src - h, src);
    }

    Mat computeHBasins(const Mat& src, uchar h)
    {
        CV_Assert(src.type() == CV_8U);
        return computeHDomes(negative(src), h);
    }
}
