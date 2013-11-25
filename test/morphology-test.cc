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

#include <morphology/ConnectedComponent.h>
#include <morphology/Attributes.h>

#include <iostream>

#include <opencv2/core/core.hpp>

using namespace cv;
using namespace morphology;

void testSort()
{
    uchar a_p = 1;
    uchar b_p = 2;

    ConnectedComponentP_t a = ConnectedComponent::create<Area>(&a_p, 1, 1, 1);
    ConnectedComponentP_t b = ConnectedComponent::create<Area>(&b_p, 1, 1, 1);
    ConnectedComponentP_t c = ConnectedComponent::create<Area>(&a_p, 1, 1, 2);

    CV_Assert(a < c);
    CV_Assert(b < a);
    CV_Assert(b < c);

    CV_Assert(!(a < b));
    CV_Assert(!(c < a));
    CV_Assert(!(c < b));
}

void testSetParent()
{
    uchar a_p = 1;
    uchar b_p = 2;

    ConnectedComponentP_t a = ConnectedComponent::create<Area>(&a_p, 1, 1, 1);
    ConnectedComponentP_t b = ConnectedComponent::create<Area>(&b_p, 1, 1, 1);

    CV_Assert(a->m_parent == a);
    CV_Assert(b->m_parent == b);

    a->setParent(b);

    CV_Assert(a->m_parent == b);
    CV_Assert(b->m_parent == b);

    CV_Assert(a->m_attribute->compute() == 1);
    CV_Assert(b->m_attribute->compute() == 2);
}

void testFindRoot()
{
    uchar a_p = 1;

    ConnectedComponentP_t a = ConnectedComponent::create<Area>(&a_p, 1, 1, 1);
    ConnectedComponentP_t b = ConnectedComponent::create<Area>(&a_p, 1, 1, 1);
    ConnectedComponentP_t c = ConnectedComponent::create<Area>(&a_p, 1, 1, 1);

    CV_Assert(a->m_parent == a);
    CV_Assert(b->m_parent == b);
    CV_Assert(c->m_parent == c);

    a->setParent(b);
    b->setParent(c);

    CV_Assert(a->m_parent == b);
    CV_Assert(b->m_parent == c);
    CV_Assert(c->m_parent == c);

    CV_Assert(ConnectedComponent::findRoot(a) == c);
    CV_Assert(a->m_parent == c);
}

void testMerge()
{
    uchar p = 1;
    ConnectedComponentP_t a = ConnectedComponent::create<Area>(&p, 1, 1, 1);
    ConnectedComponentP_t b = ConnectedComponent::create<Area>(&p, 1, 1, 1);

    CV_Assert(a->m_attribute->compute() == 1);
    CV_Assert(b->m_attribute->compute() == 1);

    a->m_attribute->merge(b->m_attribute);

    CV_Assert(a->m_attribute->compute() == 2);
    CV_Assert(b->m_attribute->compute() == 1);
}

void testIsActive()
{
    uchar p = 1;
    ConnectedComponentP_t a = ConnectedComponent::create<Area>(&p, 1, 1, 1);

    // Instead of raising area, we decrease lambda.
    CV_Assert(a->m_active);
    CV_Assert(a->isActive(2));
    CV_Assert(a->m_active);
    CV_Assert(!a->isActive(1));
    CV_Assert(!a->m_active);
}

void testSetParentAndActive()
{
    uchar p = 1;
    ConnectedComponentP_t a = ConnectedComponent::create<Area>(&p, 1, 1, 1);
    ConnectedComponentP_t b = ConnectedComponent::create<Area>(&p, 1, 1, 1);
    ConnectedComponentP_t c = ConnectedComponent::create<Area>(&p, 1, 1, 1);

    CV_Assert(a->m_active);
    CV_Assert(b->m_active);
    CV_Assert(c->m_active);

    a->setParent(b);
    b->setParent(c);

    CV_Assert(a->m_active);
    CV_Assert(b->m_active);
    CV_Assert(c->m_active);

    CV_Assert(a->m_size == 1);
    CV_Assert(b->m_size == 2);
    CV_Assert(c->m_size == 3);

    CV_Assert(a->m_attribute->compute() == 1);
    CV_Assert(b->m_attribute->compute() == 2);
    CV_Assert(c->m_attribute->compute() == 3);

    CV_Assert(ConnectedComponent::findRoot(a) == c);

    CV_Assert(a->m_active);
    CV_Assert(b->m_active);
    CV_Assert(c->m_active);

    CV_Assert(a->m_size == 1);
    CV_Assert(b->m_size == 2);
    CV_Assert(c->m_size == 3);

    CV_Assert(a->m_attribute == c->m_attribute);
    CV_Assert(a->m_attribute == b->m_attribute);
    CV_Assert(c->m_attribute == b->m_attribute);

    CV_Assert(a->m_attribute->compute() == 3);
    CV_Assert(b->m_attribute->compute() == 3);
    CV_Assert(c->m_attribute->compute() == 3);
}

void testEqualSideLength()
{
    uchar p = 0;
    ConnectedComponentP_t a = ConnectedComponent::create<EqualSideLength>(&p, 2, 1, 1);
    ConnectedComponentP_t b = ConnectedComponent::create<EqualSideLength>(&p, 1, 1, 1);
    ConnectedComponentP_t c = ConnectedComponent::create<EqualSideLength>(&p, 1, 2, 1);

    CV_Assert(a->m_attribute->compute() == 100);
    CV_Assert(b->m_attribute->compute() == 100);
    CV_Assert(c->m_attribute->compute() == 100);

    a->setParent(b);
    CV_Assert(b->m_attribute->compute() == 50);

    c->setParent(b);
    CV_Assert(b->m_attribute->compute() == 100);
}

#define RUN_TEST(x) \
do {                \
    std::cout << "Running "#x"..." << std::endl; \
    (x)();          \
    std::cout << "Passed!" << std::endl; \
} while (0)         \

int main(int argc, char** argv)
{
    // Test Area
    RUN_TEST(testMerge);

    // Test PixelSet
    RUN_TEST(testSort);
    RUN_TEST(testSetParent);
    RUN_TEST(testFindRoot);
    RUN_TEST(testIsActive);
    RUN_TEST(testSetParentAndActive);

    // Test Circularity
    RUN_TEST(testEqualSideLength);

    std::cout << "All tests done!" << std::endl;
}
