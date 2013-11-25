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

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <morphology/Attributes.h>
#include <morphology/AttributeFilter.h>

using namespace cv;
using namespace morphology;
using namespace std;

int main(int argc, char** argv)
{
    if (argc < 3 || argc > 4) {
        cout << "Usage: area-closing src lambda dst*" << endl;
        return 1;
    }

    const Mat grayscale = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    if (!grayscale.data) {
        cout << "Could not find file \"" << argv[1] << "\"\n";
        return 1;
    }

    const int lambda = atoi(argv[2]);
    AttributeFilter<Area> filter;
    const Mat closing = filter.close(grayscale, lambda);

    if (argc == 4) {
        imwrite(argv[3], closing);
    } else {
        const string title("Area closing for lambda = ");
        imshow(title + argv[2], closing);
        waitKey();
        destroyAllWindows();
    }

    return 0;
}
