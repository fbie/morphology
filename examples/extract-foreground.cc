#include <iostream>
#include <vector>
#include <map>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <morphology/SegmentationTools.h>

using namespace cv;
using namespace morphology;
using namespace std;

Mat drawContours(const Mat& img, const Mat& mask, const Scalar& color)
{
    // Colored version of the image.
    Mat dst = img.clone();

    Mat bin;
    threshold(mask, bin, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

    // Remove grain from image.
    // This is somewhat cosmetic,
    // but should be mentioned somewhere.
    AttributeFilter<Area> attribute_filter;
    attribute_filter.open(bin, 150);

    // Find contours on the input image
    std::vector<std::vector<Point> > contours;
    findContours(bin, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    // Draw contours onto output image
    for (int i = 0; i < contours.size(); i++) {
        drawContours(dst, contours, i, color, 1);
    }

    return dst;
}

int main(int argc, char** argv)
{
    if (argc < 2 || argc > 5) {
        cout << "Usage: extract-foreground src channel* dst*" << endl;
        return 1;
    } else {
        const Mat color = imread(argv[1], CV_LOAD_IMAGE_COLOR);
        if (!color.data) {
            cerr << "Could not find file \"" << argv[1] << "\"\n";
            return EXIT_FAILURE;
        } else {

            Mat src;

            map<string, Scalar> colors;
            colors["blue"] = Scalar(255, 0, 0);
            colors["green"] = Scalar(0, 255, 0);
            colors["red"] = Scalar(0, 0, 255);
            colors["gray"] = colors["red"];

            string channel = argc >= 3 ? argv[2] : "gray";
            // Extract requested color channel
            // from image.
            {
                vector<Mat> channels(3);
                split(color, channels);
                if (channel == "blue") {
                    src = channels[0];
                } else if (channel ==  "green") {
                    src = channels[1];
                } else if (channel ==  "red") {
                    src = channels[2];
                } else if (channel ==  "gray") {
                    cvtColor(color, src, COLOR_BGR2GRAY);
                } else {
                    cerr << "Unknown color channel: " << channel << endl;
                    return EXIT_FAILURE;
                }
            }

            Mat uao;
            // Perform attribute filtering for
            // requested attribute.
            {
                string attribute = argc >= 4 ? argv[3] : "area";
                if (attribute == "area") {
                    uao = ultimateAttributeClosing<Area>(src, 0.25);
                } else if (attribute == "equal-sides") {
                    uao = ultimateAttributeClosing<EqualSideLength>(src);
                } else if (attribute == "fill-ratio") {
                    uao = ultimateAttributeClosing<FillRatio>(src);
                } else {
                    cerr << "# Unknown attribute: " << attribute << endl;
                    return EXIT_FAILURE;
                }
            }

            const Mat contours = drawContours(color, uao, colors[channel]);
            if (argc == 5) {
                imwrite(argv[4], contours);
            } else {
                imshow("Ultimate Area Opening", contours);
                waitKey();
            }
        }
        destroyAllWindows();
    }
    return EXIT_SUCCESS;
}
