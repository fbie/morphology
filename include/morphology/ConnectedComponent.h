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

#ifndef __MORPHOLOGY_PIXEL_SET_H
#define __MORPHOLOGY_PIXEL_SET_H

#include <opencv2/core/core.hpp>

#include "config.h"
#include "forward.h"

namespace morphology
{
    /**
     * Represents a pixel as part of a set,
     * modeled as a tree. If the pixel's parent
     * points to itself, the pixel is root.
     */
    class MORPHOLOGY_EXPORT ConnectedComponent
    {
    public:
        // The actual pixel reference.
        unsigned char* m_pixel;

        // Set to true if active.
        bool m_active;

        // Parent set
        ConnectedComponentP_t m_parent;

        // Location on the image and in
        // the sorted array.
        const int m_x;
        const int m_y;
        const int m_idx;

        int m_size;

        // The attribute of this set.
        cv::Ptr<Attribute> m_attribute;

        ConnectedComponent(unsigned char* pixel, const int x, const int y, const int idx)
            : m_pixel(pixel), m_parent(0), m_active(true), m_x(x), m_y(y), m_idx(idx), m_size(1), m_attribute(0)
        {}

        /**
         * Factory function to properly set internal pointers.
         */
        template <typename A>
        static ConnectedComponentP_t create(uchar* pixel, const int x, const int y, const int idx)
        {
            ConnectedComponentP_t p = new ConnectedComponent(pixel, x, y, idx);
            p->m_parent = p;
            p->m_attribute = new A(p);
            return p;
        }

        /**
         * Unite two pixel sets and merge their attributes.
         */
        void setParent(ConnectedComponentP_t& parent);

        /**
         * Find the root of this pixel set and
         * compresses the path to it.
         */
        static ConnectedComponentP_t findRoot(ConnectedComponentP_t& set);

        /**
         * Check if this pixel set is still active.
         */
        bool isActive(int lambda);
    };

    /**
     * Returns true if l if has a higher pixel value
     * or, if l and r have the same pixel value, returns
     * true if l comes before r in the scan-line order.
     * False otherwise.
     */
    static bool operator<(const ConnectedComponentP_t& l, const ConnectedComponentP_t& r)
    {
        return *l->m_pixel > *r->m_pixel || (*l->m_pixel == *r->m_pixel && l->m_idx < r->m_idx);
    }
}

#endif // __MORPHOLOGY_PIXEL_SET_H
