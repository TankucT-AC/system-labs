// SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
// SPDX-License-Identifier: GPL-3.0-or-later

// Local libraries
#include <Terminal/Terminal.h>
#include <Logger/Logger.h>

// STL
#include <algorithm>
#include <format>
#include <iostream>
#include <mutex>

// POSIX C
#include <termios.h>

auto Terminal::findCommandToColor(const std::string &color) {
    auto it = std::find_if(color_list.begin(), color_list.end(), [&color](const auto &item) {
        return item.name == color;
    });

    return it;
};

auto Terminal::findColorToKeyword(char keyword) {
    auto it = std::find_if(keyword_list.begin(), keyword_list.end(), [keyword](const auto &item) {
        return item.keyword == keyword;
    });

    return it;
}

void Terminal::updateTerminalColor(char keyword) {
    std::lock_guard<std::mutex> lock(termMutex);
    if (keyword != ' ') {
        auto keyword_it = Terminal::findColorToKeyword(keyword);
        if (keyword_it == keyword_list.end())
            return;

        std::string color = keyword_it->color;
        auto command_it = Terminal::findCommandToColor(color);
        if (command_it == color_list.end())
            return;

        terminalBackgroundColorCode = command_it->value;
    }

    std::string output_msg = std::format("\033]11;#{:06X}\033[0m", terminalBackgroundColorCode);
    std::cout << output_msg << std::flush;
    termLogger.logging(std::format("key={}", keyword));
}

int Terminal::parseBackgroundColor(int argc, char **argv) {
    if (argc == 1)
        return BackgroundColorCode::BLACK;

    if (argc != 3 || std::string_view(argv[1]) != "--color")
        return -1;

    auto it = findCommandToColor(argv[2]);
    if (it == color_list.end())
        return -1;
    
    return it->value;
}

int Terminal::parseBackgroundColor(const std::vector<std::string> &argv) {
    int argc = argv.size();
    if (argc == 1)
        return BackgroundColorCode::BLACK;

    if (argc != 3 || argv[1] != "--color")
        return -1;

    auto it = findCommandToColor(argv[2]);
    if (it == color_list.end())
        return -1;

    return it->value;
}

Terminal::Terminal(int argc, char **argv, Logger &initLogger)
    : terminalBackgroundColorCode(parseBackgroundColor(argc, argv))
    , rawModeEnabled(false)
    , termLogger(initLogger) {
    if (terminalBackgroundColorCode == -1) 
        throw std::invalid_argument("Error: uncorrect arguments!!!");
    updateTerminalColor();
}

Terminal::Terminal(const std::vector<std::string> &argv, Logger &initLogger)
    : terminalBackgroundColorCode(parseBackgroundColor(argv))
    , rawModeEnabled(false)
    , termLogger(initLogger) {
    if (terminalBackgroundColorCode == -1) 
        throw std::invalid_argument("Error: uncorrect arguments!!!");
    updateTerminalColor();
}

Terminal::~Terminal() {
    disableRawMode();
    std::cout << "\033]111\033[0m" << std::flush; // возвращает цвет эмулятора терминала по умолчанию
    termLogger.logging("exit");
}

void Terminal::enableRawMode() {
    if (rawModeEnabled)
        return;
    tcgetattr(STDIN_FILENO, &originalTerminalState);
    rawTerminalState = originalTerminalState;
    rawTerminalState.c_lflag &= ~(ICANON | ECHO);
    rawTerminalState.c_cc[VMIN] = 1;
    rawTerminalState.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &rawTerminalState);
    tcflush(STDIN_FILENO, TCIFLUSH);
    rawModeEnabled = true;
}

void Terminal::disableRawMode() {
    if (!rawModeEnabled)
        return;
    tcsetattr(STDIN_FILENO, TCSANOW, &originalTerminalState);
    rawModeEnabled = false;
}
