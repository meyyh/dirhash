#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

void termsize(int& tHeight, int& tWidth) {
    FILE* pipe = popen("stty size", "r");

    char buffer[128];
    string tsize;
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        tsize += buffer;
    }
    pclose(pipe);

    stringstream ss(tsize);

    ss >> tWidth >> tHeight;
}

int main(int argc, char *argv[]){
        if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1; // Return an error code
    }

    int tHeight, tWidth;
    termsize(tHeight, tWidth);

    string imgpath = argv[1];

       // Check if the path is a full path
    if (!fs::path(imgpath).is_absolute()) {
        // If not a full path, convert it to a full path
        fs::path fullPath = fs::absolute(imgpath);

        // Assign the full path back to imagePath
        imgpath = fullPath.string();
    }
    cv::Mat image = cv::imread(imgpath);

    // ifstream inputFile(argv[1]);


    cout << imgpath << endl;
    cv::resize(image, image, cv::Size(tHeight, tWidth));

        for (int i = 0; i < image.rows; i += 2) {
        for (int j = 0; j < image.cols; j++) {
            cv::Vec3b back = image.at<cv::Vec3b>(i, j);

            string foreVT = "";
            if (i < image.rows - 1) {
                cv::Vec3b fore = image.at<cv::Vec3b>(i + 1, j);
                foreVT = "\033[38;2;" + to_string(fore[2]) + ";" + to_string(fore[1]) + ";" + to_string(fore[0]) + "m";
            }

            string backVT = "\033[48;2;" + to_string(back[2]) + ";" + to_string(back[1]) + ";" + to_string(back[0]) + "m";

            string pixelOutput = backVT + foreVT + "\u2584\033[0m";
            cout << pixelOutput;
        }
        cout << endl;
    }

    return 0;


    
}
//g++ -I/usr/include/opencv4 -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc test.cpp -o test
