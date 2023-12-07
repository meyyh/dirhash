#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <cstdlib>
#include <fstream>

using namespace std;
using namespace std::chrono;

// Global variables for shared data
cv::VideoCapture capture;
queue<cv::Mat> frameQueue;
mutex queueMutex;
condition_variable queueCV;

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

// Producer thread to load frames into the queue
void producerThread(const string& videoFile, duration<double>& producerTime) {
    cv::VideoCapture localCapture(videoFile);

    auto start = high_resolution_clock::now();

    while (true) {
        cv::Mat frame;
        localCapture >> frame;

        if (frame.empty()) {
            localCapture.set(cv::CAP_PROP_POS_FRAMES, 0);
            continue;
        }

        int tHeight, tWidth;
        termsize(tHeight, tWidth);
        cv::resize(frame, frame, cv::Size(tWidth, tHeight));

        lock_guard<mutex> lock(queueMutex);
        frameQueue.push(frame);
        queueCV.notify_one();
    }

    auto end = high_resolution_clock::now();
    producerTime = duration_cast<nanoseconds>(end - start);
}

// Consumer thread to display frames from the queue
void consumerThread(int frameDelay, duration<double>& consumerTime) {
    auto start = high_resolution_clock::now();

    while (true) {
        unique_lock<mutex> lock(queueMutex);

        queueCV.wait(lock, [] { return !frameQueue.empty(); });

        cv::Mat frame = frameQueue.front();
        frameQueue.pop();

        lock.unlock();

        displayAsciiArt(frame);

        usleep(frameDelay * 1000);
    }

    auto end = high_resolution_clock::now();
    consumerTime = duration_cast<nanoseconds>(end - start);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input_gif> <frame_delay>" << endl;
        return 1;
    }

    string videoFile = argv[1];
    int frameDelay = stoi(argv[2]);

    duration<double> producerTime, consumerTime;

    // Start producer and consumer threads
    thread producer(producerThread, videoFile, ref(producerTime));
    thread consumer(consumerThread, frameDelay, ref(consumerTime));


    // Wait for threads to finish
    producer.join();
    consumer.join();
    

    // Open the file for writing
    std::ofstream outputFile("time.txt");

    // Check if the file is opened successfully
    if (!outputFile.is_open()) {
        std::cerr << "Error opening the file time.txt" << std::endl;
        return 1; // Return an error code
    }

    outputFile << "Producer time: " << producerTime.count() << " nanoseconds" << endl;
    outputFile << "Consumer time: " << consumerTime.count() << " nanoseconds" << endl;



    return 0;
}
//g++ -I/usr/include/opencv4 -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_videoio -std=c++17 videoterm.cpp -o vidterm
