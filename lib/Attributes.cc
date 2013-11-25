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

#include <morphology/Attributes.h>

#include <morphology/ConnectedComponent.h>

using namespace cv;
using namespace std;

namespace morphology
{
    Area::Area(const ConnectedComponentP_t& pixel) :
        Attribute(pixel), m_area(1)
    {}

    int Area::compute()
    {
        return m_area;
    }

    void Area::merge(const Ptr<Attribute>& other)
    {
        const Ptr<Area> area_other = static_cast<Ptr<Area> >(other);
        m_area += area_other->m_area;
    }

    BoundingBoxAttribute::BoundingBoxAttribute(const ConnectedComponentP_t& pixel) :
        Attribute(pixel), x_min(pixel->m_x), x_max(pixel->m_x), y_min(pixel->m_y), y_max(pixel->m_y)
    {}

    void BoundingBoxAttribute::merge(const cv::Ptr<Attribute>& other)
    {
        Ptr<BoundingBoxAttribute> bb_other = static_cast<Ptr<BoundingBoxAttribute> >(other);

        x_min = min(x_min, bb_other->x_min);
        x_max = max(x_max, bb_other->x_max);
        y_min = min(y_min, bb_other->y_min);
        y_max = max(y_max, bb_other->y_max);
    }

    EqualSideLength::EqualSideLength(const ConnectedComponentP_t& pixel) :
        Attribute(pixel), BoundingBoxAttribute(pixel)
    {}

    int EqualSideLength::compute()
    {
        // Add one to avoid division by zero.
        const double width = x_max - x_min + 1;
        const double height = y_max - y_min + 1;

        // We do not want to know the actual ratio,
        // but a measurement of how circular the object is.
        const double equality = width > height ? height / width : width / height;

        CV_Assert(equality <= 1);

        // We must return an int as this is
        // value also used as index. We always
        // round down deliberately. Otherwise,
        // we'd get 101 for equal sides.
        return static_cast<int>(equality * 100);
    }

    FillRatio::FillRatio(const ConnectedComponentP_t& pixel) :
        Attribute(pixel), BoundingBoxAttribute(pixel), Area(pixel)
    {}

    int FillRatio::compute()
    {
        // Add one to avoid division by zero.
        const double width = x_max - x_min + 1;
        const double height = y_max - y_min + 1;

        double fill = m_area / (width * height);
        CV_Assert(fill <= 1);
        return static_cast<int>(fill * 100);
    }

    void FillRatio::merge(const Ptr<Attribute>& other)
    {
        BoundingBoxAttribute::merge(other);
        Area::merge(other);
    }

    inline int computeHash(const int x, const int y)
    {
        return ((x + y) * (x + y + 1)) / 2 + y;
    }

    ContourAttribute::HashedPoint* ContourAttribute::getPoint(const int x, const int y)
    {
        static map<int, HashedPoint> points;
        int hash = computeHash(x, y);

        HashedPoint& p = points[hash];
        if (!p.point) {
            p.hash = hash;
            p.point = new Point(x, y);
        }

        return &p;
    }

    ContourAttribute::HashedPoint::HashedPoint() :
        hash(-1), point(0)
    {}

    ContourAttribute::ContourAttribute(const ConnectedComponentP_t& pixel) :
        Attribute(pixel), m_start(0)
    {
        // A pixel's border consists of
        // the eight pixels around it.

        m_contour.push_back(getPoint(pixel->m_x - 1, pixel->m_y - 1));
        m_contour.push_back(getPoint(pixel->m_x - 1, pixel->m_y));
        m_contour.push_back(getPoint(pixel->m_x - 1, pixel->m_y + 1));

        m_contour.push_back(getPoint(pixel->m_x, pixel->m_y + 1));

        m_contour.push_back(getPoint(pixel->m_x + 1, pixel->m_y + 1));
        m_contour.push_back(getPoint(pixel->m_x + 1, pixel->m_y));
        m_contour.push_back(getPoint(pixel->m_x + 1, pixel->m_y - 1));

        m_contour.push_back(getPoint(pixel->m_x , pixel->m_y - 1));

        m_start = m_contour.front();

        updateMap();
    }

    void ContourAttribute::updateMap()
    {
        m_contour_map.clear();
        for (int i = 0; i < m_contour.size(); i++) {
            m_contour_map[m_contour[i]->hash] = i;
        }
    }

    void ContourAttribute::merge(const Ptr<Attribute>& other)
    {
        const Ptr<ContourAttribute> ca_other = static_cast<Ptr<ContourAttribute> >(other);

        // Determine active and inactive contours.
        vector<HashedPoint*> active = m_contour;
        map<int, int> active_map = m_contour_map;

        vector<HashedPoint*> inactive = ca_other->m_contour;
        map<int, int> inactive_map = ca_other->m_contour_map;

        // Swap, if other's start point is
        // closer to (0, 0). By doing so, we
        // avoid starting inside a contour.
        if (ca_other->m_start->point->ddot(*ca_other->m_start->point) < m_start->point->ddot(*m_start->point)) {
            active.swap(inactive);
            active_map.swap(inactive_map);
            m_start = ca_other->m_start;
        }

        // FIXME: Remove these.
        CV_Assert(!active.empty());
        CV_Assert(!inactive.empty());

        // Target contour list.
        vector<HashedPoint*> contour;

        int p = 0;
        do {
            HashedPoint* current = active[p];
            map<int, int>::iterator q = inactive_map.find(current->hash);
            if (q != inactive_map.end()) {
                // We found a neighbor, so we swap
                // to continue running on the
                // other contour.
                p = q->second;
                active.swap(inactive);
                active_map.swap(inactive_map);
            } else {
                contour.push_back(current);
            }
            p = (p + 1) % active.size();
        } while (active[p] != m_start);

        m_contour = contour;
        updateMap();
    }

}
