#include <ctime>
#include <iostream>
#include <omp.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <morphology/AttributeFilter.h>
#include <morphology/SegmentationTools.h>
#include <morphology/Timer.h>

using namespace cv;
using namespace morphology;
using namespace std;

Mat drawContours(const Mat& img, const Mat& mask, const Scalar& color)
{
    // Colored version of the image.
    Mat dst = img.clone();

    Mat bin;
    threshold(mask, bin, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

    // Find contours on the input image
    std::vector<std::vector<Point> > contours;
    findContours(bin, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    // Draw contours onto output image
    for (int i = 0; i < contours.size(); i++) {
        drawContours(dst, contours, i, color, 1);
    }

    return dst;
}

Mat segmentNaively(const Mat& img)
{

    const int lambda = ultimateAttribute<Area>(img);
    AttributeFilter<Area> area_filter;
    const Mat closing = area_filter.close(img, lambda);
    return closing - img;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        cout << "Usage: segment img1 [img2 *]\n";
        return 1;
    } else {
        for (int i = 1; i < argc; i++) {
            const Mat color = imread(argv[i], CV_LOAD_IMAGE_COLOR);
            if (!color.data) {
                cout << "Could not find file \"" << argv[i] << "\"\n";
            } else {
                Mat gray;
                cvtColor(color, gray, COLOR_BGR2GRAY);
                const Mat fg = segmentNaively(gray);
                imshow("Naive foreground model", drawContours(color, fg, Scalar(0, 0, 255)));
                waitKey();
                destroyAllWindows();
            }
        }
    }
    return 0;
}
