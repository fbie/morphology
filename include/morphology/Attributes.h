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

#ifndef __MORPHOLOGY_ATTRIBUTE_H
#define __MORPHOLOGY_ATTRIBUTE_H

#include <map>
#include <vector>

#include <opencv2/core/core.hpp>

#include "config.h"
#include "forward.h"

namespace morphology
{
    /**
     * Represents an attribute on a pixel set. This
     * needs to be implemented by the specific algorithm.
     */
    class MORPHOLOGY_EXPORT Attribute
    {
    public:
        Attribute(const ConnectedComponentP_t& pixel) {}
        virtual ~Attribute() {}

        /**
         * Compute a discrete value for this attribute.
         */
        virtual int compute() = 0;

        /**
         * Unites another attribute with this one in-place.
         */
        virtual void merge(const cv::Ptr<Attribute>& other) = 0;
    };

    /**
     * Represents the area attribute.
     */
    class MORPHOLOGY_EXPORT Area : virtual public Attribute
    {
    public:
        Area(const ConnectedComponentP_t& pixel);
        virtual ~Area() {}

        /**
         * Returns the area of this set.
         */
        virtual int compute();
        virtual void merge(const cv::Ptr<Attribute>& other);

    protected:
        int m_area;
    };

    /**
     * Abstract base class for attributes using the
     * bounding box of a connected set.
     */
    class MORPHOLOGY_EXPORT BoundingBoxAttribute : virtual public Attribute
    {
    public:
        BoundingBoxAttribute(const ConnectedComponentP_t& pixel);
        virtual void merge(const cv::Ptr<Attribute>& other);

    protected:
        // These members describe the limits
        // of the bounding box enclosing
        // the connected set.
        double x_min;
        double x_max;
        double y_min;
        double y_max;
    };

    /**
     * Represents the equality of the sides
     * of the bounding box of a connected set.
     */
    class MORPHOLOGY_EXPORT EqualSideLength : virtual public BoundingBoxAttribute
    {
    public:
        EqualSideLength(const ConnectedComponentP_t& pixel);

        /**
         * Returns a circularity measure
         * between 0 an 100.
         */
        virtual int compute();
    };

    /**
     * Represents the fill ratio of the connected set
     * to its bounding box to its actual area
     */
    class MORPHOLOGY_EXPORT FillRatio : virtual public BoundingBoxAttribute, virtual public Area
    {
    public:
        FillRatio(const ConnectedComponentP_t& pixel);

        /**
         * Returns a fill ratio between the
         * bounding box the sets area in [0, 100].
         */
        virtual int compute();
        virtual void merge(const cv::Ptr<Attribute>& other);
    };

    class MORPHOLOGY_EXPORT ContourAttribute : virtual public Attribute
    {
    public:
        ContourAttribute(const ConnectedComponentP_t& pixel);
        virtual void merge(const cv::Ptr<Attribute>& other);

    protected:
        /**
        * Maps a point to an integer after
        * the Cantor Pairing Function.
        */
        struct HashedPoint
        {
            int hash;
            cv::Ptr<const cv::Point> point;

            HashedPoint();
        };

        HashedPoint* m_start;
        std::vector<HashedPoint*> m_contour;

    private:
        void updateMap();
        static HashedPoint* getPoint(const int x, const int y);
        std::map<int, int> m_contour_map;
    };
}

#endif // __MORPHOLOGY_ATTRIBUTE_H
