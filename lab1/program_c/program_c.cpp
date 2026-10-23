// SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fcntl.h>
#include <format>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <unistd.h>
#include <vector>
#include <Terminal/Terminal.h>
#include <Logger/Logger.h>

int findProgramPID(const std::filesystem::path &path, const std::string &programName) {
    for (const auto &dir : std::filesystem::directory_iterator(path)) {
        std::string filename = dir.path().filename();
        bool isNumber = true;
        for (const char ch : filename) {
            if (!std::isdigit(ch)) {
                isNumber = false;
                break;
            }
        }
        if (!isNumber) continue;

        std::filesystem::path common = dir.path() / "comm";
        if (!std::filesystem::exists(common)) {
            std::cerr << std::format("ERROR: failed to open common file in {}", filename) << std::endl;
            return -1;
        }

        std::ifstream common_in(common);
        std::string line;
        std::getline(common_in, line);
        if (line == programName) {
            return std::atoi(filename.c_str());
        }
    }

    return -1;
}

int main() {
    std::filesystem::path procPath = std::filesystem::path().root_path() / "/proc";
    if (!std::filesystem::exists(procPath)) {
        std::cerr << std::format("ERROR: file path '{}' is nox exist!", procPath.parent_path().string()) << std::endl;
        return 1;
    }

    std::string programName = "program_a";
    int pidProgramA = findProgramPID(procPath, programName);
    if (pidProgramA == -1) {
        std::cerr << std::format("ERROR: {} is not found", programName) << std::endl;
        return 1;
    }

    std::filesystem::path cmdline = procPath / std::to_string(pidProgramA) / "cmdline";
    if (!std::filesystem::exists(cmdline)) {
        std::cerr << std::format("ERROR: failed to open cmdline file in {}", pidProgramA) << std::endl;
        return 1;
    }

    std::ifstream cmdline_in(cmdline);
    std::vector<std::string> argvVec;
    std::string word;
    while (std::getline(cmdline_in, word, '\0')) {
        argvVec.push_back(std::move(word));
    }

    // Костыль на костыле
    try {
        int argc = argvVec.size();
        char* argv[argc];
        for (int i = 0; i < argc; ++i) {
            argv[i] = const_cast<char*>(argvVec[i].c_str());
        }
        Logger dummyLogger(false);
        Terminal dummyTerminal(argc, argv, dummyLogger);
    } catch (const std::invalid_argument &e) {
        std::cerr << std::format("ERROR: {} parameters is not correct!!!", programName) << std::endl;
        return 1;
    }
}