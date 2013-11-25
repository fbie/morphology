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

namespace morphology
{
    void ConnectedComponent::setParent(ConnectedComponentP_t& parent)
    {
        parent->m_attribute->merge(m_attribute);

        // Size information is important for
        // calculating granulometries.
        parent->m_size += m_size;
        m_parent = parent;
    }

    ConnectedComponentP_t ConnectedComponent::findRoot(ConnectedComponentP_t& set)
    {
        ConnectedComponentP_t root = set;
        while (root !=root->m_parent) {
            root = root->m_parent;
        }

        ConnectedComponentP_t current = set;
        while(current != root) {
            ConnectedComponentP_t buffer = current->m_parent;
            current->m_parent = root;

            // Optimize memory usage by reducing attributes.
            current->m_attribute = root->m_attribute;
            current = buffer;
        }

        return root;
    }

    bool ConnectedComponent::isActive(int lambda)
    {
        if (m_active) {
            m_active = m_attribute->compute() < lambda;
        }
        return m_active;
    }
}
