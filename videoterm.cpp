#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <opencv2/opencv.hpp>

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

    ss >> tWidth >> tHeight;
}

void moveCursorToTopLeft() {
    cout << "\033[H"; // ANSI escape code to move the cursor to the top-left corner
}

void displayAsciiArt(const cv::Mat& image) {
    moveCursorToTopLeft(); // Move the cursor to the top-left corner before displaying each frame

    for (int i = 0; i < image.rows; i++) {
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
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input_gif> <frame_delay>" << endl;
        return 1; // Return an error code
    }

    cv::VideoCapture capture(argv[1]);

    if (!capture.isOpened()) {
        cerr << "Error opening video file." << endl;
        return 1;
    }

    int frameCount = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT));
    double frameRate = capture.get(cv::CAP_PROP_FPS);

    int frameDelay = stoi(argv[2]); // Delay between frames in milliseconds

    while (true) {
        cv::Mat frame;
        capture >> frame;

        if (frame.empty()) {
            capture.set(cv::CAP_PROP_POS_FRAMES, 0);
            continue;
        }

        // Resize the frame to match the terminal height
        int tHeight, tWidth;
        termsize(tHeight, tWidth);
        cv::resize(frame, frame, cv::Size(tWidth, tHeight / 4));

        // Display the frame as ASCII art
        displayAsciiArt(frame);

        // Introduce a delay based on the specified frame delay
        usleep(frameDelay * 1000); // usleep takes microseconds, so multiply by 1000 to convert milliseconds to microseconds
    }

    capture.release();

    return 0;
}
//g++ -I/usr/include/opencv4 -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_videoio -std=c++17 videoterm.cpp -o vidterm