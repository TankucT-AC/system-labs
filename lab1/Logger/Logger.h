// SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGGERS_H
#define LOGGERS_H

// STL
#include <chrono>
#include <fstream>
#include <iostream>
#include <string_view>

class Logger {
private:
    bool verbose;
    std::string logFileName;

public:
    Logger(bool initVerbose = false, std::string initLogFileName = "")
        : verbose(initVerbose)
        , logFileName(initLogFileName) {
    }

    void logging(std::string_view msg) {
        if (!verbose)
            return;

        auto nowUTC = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
        auto localUTC = std::chrono::zoned_time{std::chrono::current_zone(), nowUTC};
        std::string message = std::format("{:%Y-%m-%d %H:%M:%S} {}\n", localUTC, msg);

        std::ofstream logFileOfstream(logFileName, std::ios::app);
        if (logFileOfstream.is_open()) {
            logFileOfstream << message;
        } else {
            std::cerr << "ERROR: Failed to open the file!" << std::endl;
        }
    };

    void setLoggerState(bool newVerbose) {
        verbose = newVerbose;
    }
};

#endif // LOGGERS_H
