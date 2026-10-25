// SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TERMINAL_H
#define TERMINAL_H

// STL
#include <array>
#include <mutex>
#include <string>
#include <vector>

// POSIX C
#include <termios.h>
#include <unistd.h>

class Logger;

class Terminal {
private:
    struct termios originalTerminalState;
    struct termios rawTerminalState;
    int terminalBackgroundColorCode;
    bool rawModeEnabled;
    Logger &termLogger;
    std::mutex termMutex;

public:
    explicit Terminal(int argc, char **argv, Logger &logger);
    explicit Terminal(const std::vector<std::string> &argv, Logger &logger);
    ~Terminal();

    void disableRawMode();
    void enableRawMode();
    void updateTerminalColor(char keyword = ' ');

    static int parseBackgroundColor(int argc, char **argv);
    static int parseBackgroundColor(const std::vector<std::string> &argv);

private:
    enum BackgroundColorCode {
        BLACK = 0x0,
        RED = 0xff0000,
        GREEN = 0x00ff00,
        YELLOW = 0xffff00,
        BLUE = 0x0000ff,
    };

    struct Command {
        std::string name;
        BackgroundColorCode value;
    };

    struct Keyword {
        char keyword;
        std::string color;
    };

    static constexpr auto color_list =
        std::to_array<Command>({{"black", BackgroundColorCode::BLACK},
            {"red", BackgroundColorCode::RED},
            {"green", BackgroundColorCode::GREEN},
            {"yellow", BackgroundColorCode::YELLOW},
            {"blue", BackgroundColorCode::BLUE}});

    static constexpr auto keyword_list = std::to_array<Keyword>({
        {'d', "black"},
        {'r', "red"},
        {'g', "green"},
        {'y', "yellow"},
        {'b', "blue"},
    });

private:
    static auto findCommandToColor(const std::string &color);
    static auto findColorToKeyword(char keyword);
};

#endif // TERMINAL_H
