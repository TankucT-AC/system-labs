// SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TERMINAL_H
#define TERMINAL_H

#include <array>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <mutex>

class Logger;

class Terminal {
private:
    struct termios originalTerminalState;
    struct termios rawTerminalState;
    int terminalBackgroundColorCode;
    bool rawModeEnabled;
    Logger& termLogger;
    std::mutex termMutex;

public:
    explicit Terminal(int argc, char **argv, Logger& logger);
    ~Terminal();

    void disableRawMode();
    void enableRawMode();
    void updateTerminalColor(char keyword = ' ');

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

    static constexpr auto color_list = std::to_array<Command>({
        {"black", BackgroundColorCode::BLACK},
        {"red", BackgroundColorCode::RED},
        {"green", BackgroundColorCode::GREEN},
        {"yellow", BackgroundColorCode::YELLOW},
        {"blue", BackgroundColorCode::BLUE}}
    );

    static constexpr auto keyword_list = std::to_array<Keyword>({
        {'d', "black"},
        {'r', "red"},
        {'g', "green"},
        {'y', "yellow"},
        {'b', "blue"},
    });

private:
    static int parseBackgroundColor(int argc, char** argv);
    static auto findCommandToColor(const std::string &color);
    static auto findColorToKeyword(char keyword);
};

#endif // TERMINAL_H
