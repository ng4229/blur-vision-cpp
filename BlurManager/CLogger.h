#pragma once
#include <fstream>
#include <iostream>
#include <mutex>
#include <ctime>
#include <chrono>

class CLogger {
private:
    static std::ofstream logFile;
    static std::mutex logMtx;

public:
    static void Init(const std::string& fileName = "image_blur.log") {
        std::lock_guard<std::mutex> lock(logMtx);
        logFile.open(fileName, std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "로그 파일 열기 실패 (파일명: " << fileName << ")" << std::endl;
        }
    }

    static void Log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMtx);
        if (logFile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto now_time_t = std::chrono::system_clock::to_time_t(now);
            auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            std::tm now_tm;
            localtime_s(&now_tm, &now_time_t);

            // 포맷 (YY-MM-DD HH:MM:SS)
            char timeBuffer[20];
            std::strftime(timeBuffer, sizeof(timeBuffer), "%y-%m-%d %H:%M:%S", &now_tm);

            // 로그 작성
            logFile << "[" << timeBuffer << "(" << std::setw(3) << std::setfill('0') << now_ms.count() << ")] " << message << std::endl;
        }
    }

    static void Close() {
        std::lock_guard<std::mutex> lock(logMtx);
        if (logFile.is_open()) {
            logFile.close();
        }
    }
};
