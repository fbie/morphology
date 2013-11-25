#include <iostream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <morphology/SegmentationTools.h>
#include <morphology/Utils.h>

using namespace cv;
using namespace morphology;
using namespace std;

int main(int argc, char** argv)
{
    if (argc < 3 || argc > 5) {
        cout << "# Usage: pattern-spectrum path/to/img.jpg lambda channel* attribute*" << endl;
        return EXIT_FAILURE;
    } else {
        Mat color = imread(argv[1], CV_LOAD_IMAGE_COLOR);

        if (!color.data) {
            cerr << "Could not find file \"" << argv[1] << "\"\n";
            return EXIT_FAILURE;
        }

        int lambda = atoi(argv[2]);

        Mat src;
        // Extract requested color channel
        // from image.
        {
            string channel = argc >= 4 ? argv[3] : "gray";
            vector<Mat> channels(3);
            split(color, channels);
            if (channel == "blue") {
                src = channels[0];
            } else if (channel ==  "red") {
                src = channels[1];
            } else if (channel ==  "green") {
                src = channels[2];
            } else if (channel ==  "gray") {
                cvtColor(color, src, COLOR_BGR2GRAY);
            } else {
                cerr << "Unknown color channel: " << channel << endl;
                return EXIT_FAILURE;
            }
        }

        string attribute = argc >= 5 ? argv[4] : "area";
        vector<int> spectrum;
        if (attribute == "area") {
            spectrum = computeGranulometry<Area>(src, lambda);
            // Convert
            vector<int> radius_spectrum;
            int last_radius = -1;
            for (int i = 0; i < spectrum.size(); i++) {

                // New radius? Push back a new bin!
                int radius = to_radius(i);
                if (last_radius != radius) {
                    last_radius = radius;
                    radius_spectrum.push_back(0);
                }
                radius_spectrum.back() += spectrum[i];
            }
            spectrum = radius_spectrum;

        } else if (attribute == "equal-sides") {
            spectrum = computeGranulometry<EqualSideLength>(src, lambda);

        } else if (attribute == "fill-ratio") {
            spectrum = computeGranulometry<FillRatio>(src, lambda);
        } else {
            cerr << "Unknown attribute: " << attribute << endl;
            return EXIT_FAILURE;
        }

        cout << "#" <<  argv[1] << ":" << attribute << ":" << argv[2] << endl;
        for (int i = 0; i < spectrum.size(); i++) {
            cout << i << ":" << spectrum[i] << endl;
        }
    }
    return EXIT_SUCCESS;
}
