#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

void setBinarizationStats(const int& binarizationType, int& lastBinarizationType);

int main(int argc, char* argv[]) {
    if (argc < 2){
        std::cerr << "Usage: " << argv[0] << " <video source no.>" << std::endl;
        return -1;
    }

    cv::VideoCapture webCam(std::atoi(argv[1]));       // VideoCapture object declaration. Usually 0 is the integrated, 2 is the first external USB one

    if (!webCam.isOpened()){   // Check if the VideoCapture object has been correctly associated to the webcam
        std::cerr << "error: Webcam could not be connected." << std::endl;
        return -1;
    }

    cv::Mat imgOriginal;            // input image
    cv::Mat imgGrayScale;           // grayscale image
    cv::Mat imgBlurred;             // blurred image
    cv::Mat imgCanny;               // edge image

    cv::Mat imgBinarized;           // binarized image
    cv::Mat imgAdaptiveBinarized;   // adaptive binarized image

    char charCheckForESCKey{0};     // escape key check

    // Binarization Parameters

    int threshold{127};             // binarized image threshold
    int maxValue{255};              // binarized image max value
    int type{0};                    // binarization type
    int lastType = 0;               // last used binarization type

    // Adaptive Binarization Parameters

    // pixel value for which the condition is satisfied
    int adaptiveThresholdMaxValue = 255;
    // 0 - ADAPTIVE_THRESH_MEAN_C   1 - ADAPTIVE_THRESH_GAUSSIAN_C
    int adaptiveMethod = 0;
    // 0 - THRESH_BINARY            1 - THRESH_BINARY_INV
    int thresholdType = 0;
    // size of a pixel neighborhood that is used to calculate a threshold value for the pixel
    std::vector<int> blockSizes({3,5,7,11});
    // index for selecting the block size
    int blockSizeIndex = 0;
    // Constant subtracted from the mean or weighted mean
    int C = 5;

    while (charCheckForESCKey != 27 && webCam.isOpened()){    // loop until ESC key is pressed or webcam is lost
        bool frameSuccess = webCam.read(imgOriginal);           // get next frame from input stream

        if (!frameSuccess || imgOriginal.empty()){              // if the frame was not read or read wrongly
            std::cerr << "error: Frame could not be read." << std::endl;
            break;
        }

        // Sets default options for binarization types when it detects a change.
        setBinarizationStats(type, lastType);

        cv::cvtColor(imgOriginal, imgGrayScale, cv::COLOR_BGR2GRAY);  // original video is converted to grayscale into imgGrayScale
        cv::GaussianBlur(imgGrayScale, imgBlurred, cv::Size(5, 5), 1.8);  // blurs the grayscale video. Check OpenCV docs for explanation of parameters
        cv::Canny(imgBlurred, imgCanny, threshold, maxValue);   // Canny edge detection. Check OpenCV docs for explanation of parameters

        // Grayscale image is converted to a parametrized binarized image into imgBinarized
        cv::threshold(imgGrayScale, imgBinarized, threshold, maxValue, type);
        // Grayscale image is converted to a parametrized adaptive binarized image into imgAdaptiveBinarized
        cv::adaptiveThreshold(imgGrayScale, imgAdaptiveBinarized, adaptiveThresholdMaxValue, adaptiveMethod, thresholdType, blockSizes[blockSizeIndex], C);

        // Declaration of windows for output video results. Check OpenCV docs for explanation of parameters
        cv::namedWindow("imgOriginal", cv::WINDOW_AUTOSIZE);
        cv::namedWindow("imgCanny", cv::WINDOW_AUTOSIZE);

        // New windows for binarization
        cv::namedWindow("imgBinarized", cv::WINDOW_AUTOSIZE);
        cv::namedWindow("imgAdaptiveBinarized", cv::WINDOW_AUTOSIZE);


        // Declaration of trackbars to change thresholds. Check OpenCV docs for explanation of parameters
        cv::createTrackbar("Threshold", "imgCanny", &threshold, 100);
        cv::createTrackbar("Max Value", "imgCanny", &maxValue, 100);

        // Binarization Parameters
        cv::createTrackbar("Binarization Type", "imgBinarized", &type, 4);
        cv::createTrackbar("Threshold", "imgBinarized", &threshold, 255);
        cv::createTrackbar("Max Value", "imgBinarized", &maxValue, 255);

        // Adaptive Binarization Parameters
        cv::createTrackbar("Adaptive Method", "imgAdaptiveBinarized", &adaptiveMethod, 1);
        cv::createTrackbar("Threshold Type", "imgAdaptiveBinarized", &thresholdType, 1);
        cv::createTrackbar("Block Size", "imgAdaptiveBinarized", &blockSizeIndex, (int)blockSizes.size() - 1);
        cv::createTrackbar("C", "imgAdaptiveBinarized", &C, 15);

        // Show output video results windows
        cv::imshow("imgOriginal", imgOriginal);
        cv::imshow("imgCanny", imgCanny);

        // New video outputs
        cv::imshow("imgBinarized", imgBinarized);
        cv::imshow("imgAdaptiveBinarized", imgAdaptiveBinarized);

        charCheckForESCKey = cv::waitKey(1);  // gets the key pressed
    }
    return 0;
}

void setBinarizationStats(const int& binarizationType, int& lastBinarizationType) {
    if (lastBinarizationType != binarizationType) {
        lastBinarizationType = binarizationType;
        /*
           0: Binary
           1: Binary Inverted
           2: Threshold Truncated
           3: Threshold to Zero
           4: Threshold to Zero Inverted
       */

        switch (binarizationType) {
            case 0: {
                cv::setTrackbarPos("Threshold", "imgBinarized", 127);
                cv::setTrackbarPos("Max Value", "imgBinarized", 255);
                break;
            }
            case 1: {
                cv::setTrackbarPos("Threshold", "imgBinarized", 127);
                cv::setTrackbarPos("Max Value", "imgBinarized", 255);
                break;
            }
            case 2: {
                cv::setTrackbarPos("Threshold", "imgBinarized", 127);
                cv::setTrackbarPos("Max Value", "imgBinarized", 0);
                break;
            }
            case 3: {
                cv::setTrackbarPos("Threshold", "imgBinarized", 127);
                cv::setTrackbarPos("Max Value", "imgBinarized", 0);
                break;
            }
            case 4: {
                cv::setTrackbarPos("Threshold", "imgBinarized", 127);
                cv::setTrackbarPos("Max Value", "imgBinarized", 0);
                break;
            }
        }
    }
}
