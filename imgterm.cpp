#include <iostream>
#include <sstream>
#include <opencv4/opencv2/opencv.hpp>

void termsize(int& tHeight, int& tWidth) {
    FILE* pipe = popen("stty size", "r");

    char buffer[128];
    std::string tsize;
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        tsize += buffer;
    }
    pclose(pipe);

    std::stringstream ss(tsize);

    ss >> tHeight >> tWidth;
}

void toAsciiArt(const cv::Mat& image, std::ostringstream& imagebuffer) {
    //cout << "\033[H"; // ANSI escape code to move the cursor to the top-left corner

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            cv::Vec3b back = image.at<cv::Vec3b>(i, j);

            std::string foreVT = "";
            if (i < image.rows - 1) {
                cv::Vec3b fore = image.at<cv::Vec3b>(i + 1, j);
                foreVT = "\033[38;2;" + std::to_string(fore[2]) + ";" + std::to_string(fore[1]) + ";" + std::to_string(fore[0]) + "m";
            }

            std::string backVT = "\033[48;2;" + std::to_string(back[2]) + ";" + std::to_string(back[1]) + ";" + std::to_string(back[0]) + "m";

            std::string pixelOutput = backVT + foreVT + "\u2584\033[0m";
            imagebuffer << pixelOutput;
        }
    }
}

int main(int argc, char *argv[])
{
    cv::Mat image = cv::imread(argv[1], cv::IMREAD_ANYCOLOR);

    //resize image to terminal
    int tHeight, tWidth;
    termsize(tHeight, tWidth);
    cv::resize(image, image, cv::Size(tWidth, tHeight));

    // Create an output string stream
    std::ostringstream imagebuffer;

    //convert image to ascii art and put it in the buffer
    toAsciiArt(image, imagebuffer);

    std::cout << imagebuffer.str() << std::endl;

    return 0;
}
//g++ -I/usr/include/opencv4 -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_videoio -std=c++17 imgterm.cpp -o imgterm