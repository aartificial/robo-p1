#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

void setDefaults(const int& type);
void drawHistogram(const cv::Mat& img);
void setupImgBinarizedProps(int &threshold, int &maxValue, int &type, const cv::Mat& img);
void setupImgAdaptiveBinarizedProps(const std::vector<int> &blockSizes, int &adaptiveMethod, int &thresholdType,int &blockSizeIndex, int &C, const cv::Mat& img);

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
    int lastType{0};                // last used binarization type

    // Adaptive Binarization Parameters

    // pixel value for which the condition is satisfied
    int adaptiveThresholdMaxValue{255};
    // 0 - ADAPTIVE_THRESH_MEAN_C   1 - ADAPTIVE_THRESH_GAUSSIAN_C
    int adaptiveMethod{1};
    // 0 - THRESH_BINARY            1 - THRESH_BINARY_INV
    int thresholdType{0};
    // size of a pixel neighborhood that is used to calculate a threshold value for the pixel
    std::vector<int> blockSizes({3,5,7,11});
    // index for selecting the block size
    int blockSizeIndex{0};
    // Constant subtracted from the mean or weighted mean
    int C{5};

    while (charCheckForESCKey != 27 && webCam.isOpened()) {    // loop until ESC key is pressed or webcam is lost
        bool frameSuccess = webCam.read(imgOriginal);           // get next frame from input stream

        if (!frameSuccess || imgOriginal.empty()){              // if the frame was not read or read wrongly
            std::cerr << "error: Frame could not be read." << std::endl;
            break;
        }

        // Sets default options for binarization types when it detects a change.
        if (type != lastType) {
            setDefaults(type);
            lastType = type;
        }

        // Draw histogram of the original image
        drawHistogram(imgOriginal);

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

        // Declaration of trackbars to change thresholds. Check OpenCV docs for explanation of parameters
        cv::createTrackbar("Threshold", "imgCanny", &threshold, 100);
        cv::createTrackbar("Max Value", "imgCanny", &maxValue, 100);

        // Binarization
        setupImgBinarizedProps(threshold, maxValue, type, imgBinarized);
        // Adaptive Binarization
        setupImgAdaptiveBinarizedProps(blockSizes, adaptiveMethod, thresholdType, blockSizeIndex, C, imgAdaptiveBinarized);

        // Show output video results windows
        cv::imshow("imgOriginal", imgOriginal);
        cv::imshow("imgCanny", imgCanny);

        // Check for ESC key press
        charCheckForESCKey = static_cast<char>(cv::waitKey(1));  // gets the key pressed
    }
    return 0;
}

void setupImgAdaptiveBinarizedProps(const std::vector<int> &blockSizes, int &adaptiveMethod, int &thresholdType, int &blockSizeIndex, int &C, const cv::Mat& img) {
    std::string name = "imgAdaptiveBinarized";
    cv::namedWindow(name, cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("Adaptive Method", name, &adaptiveMethod, 1);
    cv::createTrackbar("Threshold Type", name, &thresholdType, 1);
    cv::createTrackbar("Block Size", name, &blockSizeIndex, (int)blockSizes.size() - 1);
    cv::createTrackbar("C", name, &C, 15);
    cv::imshow(name, img);
}

void setupImgBinarizedProps(int &threshold, int &maxValue, int &type, const cv::Mat& img) {
    std::string name = "imgBinarized";
    cv::namedWindow(name, cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("Binarization Type", name, &type, 4);
    cv::createTrackbar("Threshold", name, &threshold, 255);
    cv::createTrackbar("Max Value", name, &maxValue, 255);
    cv::imshow(name, img);
}

void drawHistogram(const cv::Mat& img) {
    // Separate the source image in its three R, G and B planes.
    std::vector<cv::Mat> planes;
    cv::split(img, planes);

    // Establish the number of bins.
    int histSize = 256;
    float range[] = { 0, 256 };
    const float* histRange = { range };

    // Proceed to calculate the histograms.
    std::vector<cv::Mat> histograms(3);
    cv::calcHist(&planes[0], 1, nullptr, cv::Mat(), histograms[0], 1, &histSize, &histRange, true, false);
    cv::calcHist(&planes[1], 1, nullptr, cv::Mat(), histograms[1], 1, &histSize, &histRange, true, false);
    cv::calcHist(&planes[2], 1, nullptr, cv::Mat(), histograms[2], 1, &histSize, &histRange, true, false);

    // Normalize the histogram so its values fall in range
    cv::normalize(histograms[0], histograms[0], 0, 255, cv::NORM_MINMAX, -1, cv::Mat());
    cv::normalize(histograms[1], histograms[1], 0, 255, cv::NORM_MINMAX, -1, cv::Mat());
    cv::normalize(histograms[2], histograms[2], 0, 255, cv::NORM_MINMAX, -1, cv::Mat());

    // Create an image to display the histogram
    int width = 512, height = 400;
    int bin_w = cvRound((double) width / histSize);
    cv::Mat histogram(height, width, CV_8UC3, cv::Scalar(0, 0, 0));

    // Draw the histograms for each channel in the image
    for(int i = 1; i < histSize; i++) {
        line(histogram, cv::Point(bin_w * (i - 1), height - cvRound(histograms[0].at<float>(i - 1)) ), cv::Point(bin_w * (i), height - cvRound(histograms[0].at<float>(i)) ), cv::Scalar(255, 0, 0), 2, 8, 0  );
        line(histogram, cv::Point(bin_w * (i - 1), height - cvRound(histograms[1].at<float>(i - 1)) ), cv::Point(bin_w * (i), height - cvRound(histograms[1].at<float>(i)) ), cv::Scalar(0, 255, 0), 2, 8, 0  );
        line(histogram, cv::Point(bin_w * (i - 1), height - cvRound(histograms[2].at<float>(i - 1)) ), cv::Point(bin_w * (i), height - cvRound(histograms[2].at<float>(i)) ), cv::Scalar(0, 0, 255), 2, 8, 0  );
    }

    // Display the histogram
    cv::imshow("Histogram", histogram);
}

void setDefaults(const int& type) {
    /* 0: Binary
       1: Binary Inverted
       2: Threshold Truncated
       3: Threshold to Zero
       4: Threshold to Zero Inverted */
    int threshold;
    int maxValue;

    switch (type) {
        case 0 ... 1: {
            threshold = 127;
            maxValue = 255;
            break;
        }
        case 2 ... 4: {
            threshold = 127;
            maxValue = 0;
            break;
        }
        default: {
            threshold = 0;
            maxValue = 0;
        }
    }

    cv::setTrackbarPos("Threshold", "imgBinarized", threshold);
    cv::setTrackbarPos("Max Value", "imgBinarized", maxValue);
}
