#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <opencv4/opencv2/opencv.hpp>


using namespace std;

void termsize(int& tHeight, int& tWidth) {
    FILE* pipe = popen("stty size", "r");

    char buffer[128];
    string tsize;
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        tsize += buffer;
    }
    pclose(pipe);

    stringstream ss(tsize);

    ss >> tHeight >> tWidth;
}

void extract_frames(cv::VideoCapture capture, vector<cv::Mat>& frames)
{	
  	//cap.get(CV_CAP_PROP_FRAME_COUNT) contains the number of frames in the video;
  	for(int frameNum = 0; frameNum < capture.get(cv::CAP_PROP_FRAME_COUNT);frameNum++)
  	{
  		cv::Mat frame;
  		capture >> frame; // get the next frame from video
  		frames.push_back(frame);
  	}
}

void displayAsciiArt(const cv::Mat& image) {
    cout << "\033[H"; // ANSI escape code to move the cursor to the top-left corner

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            cv::Vec3b back = image.at<cv::Vec3b>(i, j);

            string foreVT = "";
            if (i < image.rows - 1) {
                cv::Vec3b fore = image.at<cv::Vec3b>(i + 1, j);
                foreVT = "\033[38;2;" + to_string(fore[2]) + ";" + to_string(fore[1]) + ";" + to_string(fore[0]) + "m";
            }

            string backVT = "\033[48;2;" + to_string(back[2]) + ";" + to_string(back[1]) + ";" + to_string(back[0]) + "m";

            string pixelOutput = backVT + foreVT + "\u2584\033[0m"; //aka "▄\033[0m"
            cout << pixelOutput;
        }

        cout << endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input_gif> <frame_delay>" << endl;
        cerr << "if frame_delay is -1 use the videos origional fps" << endl;
        return 1; 
    }

    cv::VideoCapture capture(argv[1]);
    int frameDelay = stoi(argv[2]); // Delay between frames in milliseconds

    if (!capture.isOpened()) {
        cerr << "Error opening video file." << endl;
        return 1;
    }

    vector<cv::Mat> vecoframes;
    extract_frames(capture, vecoframes);

    int frameCount = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT));
    double frameRate = capture.get(cv::CAP_PROP_FPS);  

    //std::cout << frameCount << "\n";
    //std::cout << frameRate << "\n";

    while (true) {//loop video
        for (const auto& frame : vecoframes) {
            int tHeight, tWidth;
            termsize(tHeight, tWidth);
            cv::Mat resizedFrame;
            cv::resize(frame, resizedFrame, cv::Size(tWidth, tHeight));

            displayAsciiArt(resizedFrame);

            //if framedelay is -1 use the videos origional fps
            //can cause video to be slow at higher term sizes
            if(argv[2] == string("-1")){
                std::cout << "argv2: " << argv[2] << endl;
                // delay to change "speed" of the video
                int delay = 1000 / frameRate;
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(frameDelay));
                std::cout << "argv2: " << argv[2] << endl;
            }
        }
    }

    capture.release();

    return 0;
}

//g++ -I/usr/include/opencv4 -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_videoio -std=c++17 videoterm.cpp -o vidterm