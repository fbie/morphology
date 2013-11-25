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

#ifndef __MORPHOLOGY_ATTRIBUTE_FILTER_H
#define __MORPHOLOGY_ATTRIBUTE_FILTER_H

#include <algorithm>
#include <vector>

#include <opencv2/core/core.hpp>

#include "config.h"
#include "Attributes.h"
#include "ConnectedComponent.h"
#include "Utils.h"

namespace morphology
{
    /**
     * Algorithms performing attribute opening and closing
     * on a given image.
     *
     * Implemented after
     *
     * M. H. F. Wilkinson & J. B. T. M. Roerdink (2000):
     * "Fast Morphological Attribute Operations Using Tarjan's
     * Union-Find Algorithm". In Proceedings of the ISMM2000,
     * pp. 311-320.
     *
     * A. Meijster & M. H. F. Wilkinson (2002):
     * "A comparison of algorithms for connected set openings
     * and closings". In IEEE Transactions on Pattern Analysis
     * and Machine Intelligence, 24(4):484-494.
     */

    namespace
    {
        /**
         * Compute an index in the array of pixels
         */
        inline int computeIdx(const int x, const int y, const int width)
        {
            return x + y * width;
        }

        /**
         * Generate a list of pixels from given image.
         */
        template <typename A>
        std::vector<ConnectedComponentP_t> makePixelSets(cv::Mat& dst)
        {
            // Reserve space in the vector so that we can
            // access every index even if it is not initialized
            // yet. This enables parallelization.
            std::vector<ConnectedComponentP_t> sets(dst.rows * dst.cols);
            for (int y = 0; y < dst.rows; y++) {
                uchar* p = dst.ptr(y);
                const uchar* p_end = p + dst.cols;
                int x = 0;
                while (p != p_end) {
                    int index = computeIdx(x, y, dst.cols);
                    sets[index] = ConnectedComponent::create<A>(p, x, y, index);
                    p++;
                    x++;
                }
            }
            return sets;
        }
    }

    /**
     * An attribute filter for the attribute A.
     */
    template <typename A>
    class MORPHOLOGY_EXPORT AttributeFilter
    {
    public:
        virtual ~AttributeFilter() {}

        void open(cv::Mat& dst, int lambda, std::vector<cv::Ptr<A> >* attributes = 0);
        cv::Mat open(const cv::Mat& src, int lambda, std::vector<cv::Ptr<A> >* attributes = 0);

        void close(cv::Mat &dst, int lambda, std::vector<cv::Ptr<A> >* attributes = 0);
        cv::Mat close(const cv::Mat &src, int lambda, std::vector<cv::Ptr<A> >* attributes = 0);

    protected:
        int m_lambda;

        /**
         * Unites the pixel sets according to activity.
         */
        std::vector<ConnectedComponentP_t> buildSets(const std::vector<ConnectedComponentP_t>& pixels, const int rows, const int cols);

        /**
         * Unites two pixels and their corresponding
         * sets.
         */
        virtual void unite(ConnectedComponentP_t& neighbor, ConnectedComponentP_t& current);

    };

    template <typename A>
    void AttributeFilter<A>::open(cv::Mat& dst, int lambda, std::vector<cv::Ptr<A> >* attributes)
    {
        CV_Assert(dst.type() == CV_8U);

        m_lambda = lambda;

        const std::vector<ConnectedComponentP_t> sets = makePixelSets<A>(dst);
        std::vector<ConnectedComponentP_t> sorted = buildSets(sets, dst.rows, dst.cols);

        // Resolve pixel sets by assigning the grey value
        // of each root to the members of its set.
        for (std::vector<ConnectedComponentP_t>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); it++) {
            ConnectedComponentP_t current = *it;
            if (current->m_parent != current) {
                *current->m_pixel = *current->m_parent->m_pixel;
            } else if (attributes) {
                attributes->push_back(current->m_attribute);
            }
        }
    }

    template <typename A>
    cv::Mat AttributeFilter<A>::open(const cv::Mat& src, int lambda, std::vector<cv::Ptr<A> >* attributes)
    {
        cv::Mat dst = src.clone();
        open(dst, lambda, attributes);
        return dst;
    }

    template <typename A>
    void  AttributeFilter<A>::close(cv::Mat& dst, int lambda, std::vector<cv::Ptr<A> >* attributes)
    {
        CV_Assert(dst.type() == CV_8U);
        negative(dst);
        open(dst, lambda, attributes);
        negative(dst);
    }

    template <typename A>
    cv::Mat AttributeFilter<A>::close(const cv::Mat& src, int lambda, std::vector<cv::Ptr<A> >* attributes)
    {
        cv::Mat dst = src.clone();
        close(dst, lambda, attributes);
        return dst;
    }

    template <typename A>
    std::vector<ConnectedComponentP_t> AttributeFilter<A>::buildSets(const std::vector<ConnectedComponentP_t>& pixels, const int rows, const int cols)
    {
        std::vector<ConnectedComponentP_t> sorted = pixels;
        sort(sorted.begin(), sorted.end(), std::less<ConnectedComponentP_t>());

        // Build disjoint pixel sets
        for (std::vector<ConnectedComponentP_t>::iterator it = sorted.begin(); it != sorted.end(); it++) {
            ConnectedComponentP_t current = *it;

            // Compute pixel coordinate limits.
            const int x_lower = std::max(current->m_x - 1, 0);
            const int x_upper = std::min(current->m_x + 1, cols - 1);
            const int y_lower = std::max(current->m_y - 1, 0);
            const int y_upper = std::min(current->m_y + 1, rows - 1);

            // For each neighbor...
            for (int y = y_lower; y <= y_upper; y++) {
                for (int x = x_lower; x <= x_upper; x++) {
                    ConnectedComponentP_t neighbor = pixels[computeIdx(x, y, cols)];

                    // Unite if either neighbor has a higher grey value
                    // than current or if they are at level and neighbor
                    // comes before current in scan-line order.
                    if (*current->m_pixel < *neighbor->m_pixel || neighbor < current) {
                        unite(neighbor, current);
                    }
                }
            }
        }
        return sorted;
    }

    template <typename A>
    void AttributeFilter<A>::unite(ConnectedComponentP_t& neighbor, ConnectedComponentP_t& current)
    {
        ConnectedComponentP_t root = ConnectedComponent::findRoot(neighbor);

        // If root and current are the same,
        // neighbor and current are already
        // in the same set.
        if (root != current) {

            // Unite sets if root and current are level
            // pixels or if root's attribute is still
            // active for lambda.
            if (*root->m_pixel == *current->m_pixel || root->isActive(m_lambda)) {
                root->setParent(current);
            } else {
                current->m_active = false;
            }
        }
    }

    template <typename A>
    class MORPHOLOGY_EXPORT AttributePatternSpectrum : private AttributeFilter<A>
    {
    public:
        virtual ~AttributePatternSpectrum() {}

        /**
         * @brief open Computes a pattern spectrum via opening.
         * @param src The source image.
         * @param lambda Upper limit on attributes.
         * @param max_size The maximum size of the element to filter, defaults to 20% of the image area.
         * @return Differential pattern spectrum of the image for given attribute.
         */
        std::vector<int> open(const cv::Mat& src, int lambda, int max_size = -1);

        /**
         * @brief close Computes a pattern spectrum via closing.
         * @param src The source image.
         * @param lambda Upper limit on attributes.
         * @param max_size The maximum size of the element to filter, defaults to 20% of the image area.
         * @return Differential pattern spectrum of the image for given attribute.
         */
        std::vector<int> close(const cv::Mat& src, int lambda, int max_size = -1);

    private:
        std::vector<int> m_spectrum;
        int m_lambda;
        int m_max_size;

        virtual void unite(ConnectedComponentP_t& neighbor, ConnectedComponentP_t& current);
    };

    template <typename A>
    std::vector<int> AttributePatternSpectrum<A>::open(const cv::Mat& src, int lambda, int max_size)
    {
        CV_Assert(src.type() == CV_8U);

        if (max_size < 0) {
            m_max_size = (src.cols * src.rows) / 5;
        } else {
            m_max_size = max_size;
        }

        m_lambda = lambda;
        m_spectrum.clear();
        m_spectrum.resize(m_lambda);

        // For the sake of code re-use, we
        // perform an actual opening.
        cv::Mat aux = src.clone();
        const std::vector<ConnectedComponentP_t> sets = makePixelSets<A>(aux);
        AttributeFilter<A>::buildSets(sets, aux.rows, aux.cols);

        return m_spectrum;
    }

    template <typename A>
    std::vector<int> AttributePatternSpectrum<A>::close(const cv::Mat& src, int lambda, int max_size)
    {
        CV_Assert(src.type() == CV_8U);
        return open(negative(src), lambda, max_size);
    }

    template <typename A>
    void AttributePatternSpectrum<A>::unite(ConnectedComponentP_t& neighbor, ConnectedComponentP_t& current)
    {
        ConnectedComponentP_t root = ConnectedComponent::findRoot(neighbor);

        CV_Assert(current == current->m_parent);

        if (root != current && root->m_size <= m_max_size) {

            // Set spectrum grey value
            if (*root->m_pixel == *current->m_pixel || root->isActive(m_lambda)) {
                m_spectrum[root->m_attribute->compute()] += (*root->m_pixel - *current->m_pixel) * root->m_size;
            }
            root->setParent(current);
        }
    }
}

#endif // __MORPHOLOGY_ATTRIBUTE_FILTER_H
