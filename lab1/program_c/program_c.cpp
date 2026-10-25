// SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
// SPDX-License-Identifier: GPL-3.0-or-later

// Local libraries
#include <Logger/Logger.h>
#include <Terminal/Terminal.h>
#include <utils/utils.h>

// STL
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

// POSIX C
#include <signal.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

pid_t findProgramPID(
    const std::filesystem::path &path, const std::string &programName) {
    for (const auto &dir : std::filesystem::directory_iterator(path)) {
        std::string filename = dir.path().filename();
        bool isNumber = true;
        for (const char ch : filename) {
            if (!std::isdigit(ch)) {
                isNumber = false;
                break;
            }
        }
        if (!isNumber)
            continue;

        std::filesystem::path common = dir.path() / "comm";
        if (!std::filesystem::exists(common)) {
            std::cerr << std::format(
                "ERROR: failed to open common file in {}", filename)
                      << std::endl;
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

bool isProcessAlive(pid_t pid) {
    if (kill(pid, 0) == 0) {
        return true;
    }
    return errno != ESRCH;
}

std::vector<std::string> parseProcCmdline(const std::filesystem::path &path) {
    std::ifstream cmdline_in(path);

    std::vector<std::string> argvVec;
    std::string word;
    while (std::getline(cmdline_in, word, '\0')) {
        argvVec.push_back(std::move(word));
    }
    return argvVec;
}

void injection(
    const std::string &path, std::atomic<bool> &isRunning, char trigger) {
    std::string pipe_path = "/tmp/program_a_fifo";
    int target_fd = open(pipe_path.data(), O_WRONLY);
    if (target_fd < 0) {
        std::cerr << "ERROR: Failed to open FIFO channel!" << std::endl;
        return;
    }

    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "ERROR [injection]: Log file NOT FOUND at path: "
                  << std::filesystem::absolute(path) << std::endl;
        close(target_fd);
        return;
    }

    in.seekg(0, std::ios::end);
    std::streampos last_pos = in.tellg();

    std::string line;
    while (isRunning) {
        in.seekg(0, std::ios::end);
        std::streampos current_len = in.tellg();

        if (current_len > last_pos) {
            in.seekg(last_pos);

            while (std::getline(in, line)) {
                if (line.empty())
                    continue;

                auto pos = line.find("key=");
                if (pos == std::string::npos) {
                    continue;
                }

                std::string_view payload = std::string_view(line).substr(pos);

                std::optional<char> keyOpt = utils::keyMessageParser(payload);
                if (keyOpt.has_value() && keyOpt.value() == trigger) {
                    utils::ipc_worker(target_fd, 'd');
                }
            }

            in.clear();
            last_pos = current_len;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    close(target_fd);
}

int loggingProgramToPID(pid_t pid, 
    std::filesystem::path &procPath, 
    Logger &logger) {
        
    std::filesystem::path programA = procPath / std::to_string(pid);
    if (!std::filesystem::exists(programA)) {
        std::cerr << std::format("ERROR: failed to open stat in {}", pid)
                  << std::endl;
        return 1;
    }

    auto loggingProgramAState = [&logger](auto &nowUTC, bool isRunningState) {
        auto localUTC = std::chrono::zoned_time{std::chrono::current_zone(), nowUTC};
        
        std::string state = (isRunningState) ? "running" : "ending";
        std::string startProgramAMessage = std::format(
            "Process `program_a` is {} in {:%Y-%m-%d %H:%M:%S}", state, localUTC);
        logger.logging(startProgramAMessage);
    };

    auto ftime = std::filesystem::last_write_time(programA);
    auto nowUTCStart = std::chrono::floor<std::chrono::seconds>(
        std::chrono::clock_cast<std::chrono::system_clock>(ftime));
    loggingProgramAState(nowUTCStart, true);
    
    bool isAlive;
    while (true) {
        isAlive = isProcessAlive(pid);
        if (!isAlive)
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    auto nowUTCExit = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now());
    loggingProgramAState(nowUTCExit, false);

    return 0;
}

int main() {
    // Блокируем SIGPIPE, чтобы он не убивал программу при попытки записи из
    // пустого канала
    struct sigaction sa{};
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, nullptr);

    std::atomic<bool> isRunning{true};

    std::filesystem::path procPath =
        std::filesystem::path().root_path() / "/proc";
    if (!std::filesystem::exists(procPath)) {
        std::cerr << std::format("ERROR: file path '{}' is nox exist!",
            procPath.parent_path().string())
                  << std::endl;
        return 1;
    }

    std::string programName = "program_a";
    pid_t pidProgramA = findProgramPID(procPath, programName);
    if (pidProgramA == -1) {
        std::cerr << std::format("ERROR: {} is not found", programName)
                  << std::endl;
        return 1;
    }

    std::filesystem::path cmdline =
        procPath / std::to_string(pidProgramA) / "cmdline";
    if (!std::filesystem::exists(cmdline)) {
        std::cerr << std::format(
            "ERROR: failed to open cmdline file in {}", pidProgramA)
                  << std::endl;
        return 1;
    }

    std::vector<std::string> argvVec = parseProcCmdline(cmdline);

    if (int err = Terminal::parseBackgroundColor(argvVec); err == -1) {
        std::cerr << std::format(
            "ERROR: {} parameters is not correct!!!", programName)
                  << std::endl;
        return 1;
    }

    Logger logger(true, "program_c.log");
    std::thread ipc_injection_thread(
        injection, "program_a.log", std::ref(isRunning), 'b');

    while (isRunning) {
        if (int err = loggingProgramToPID(pidProgramA, procPath, logger);
            err != 0)
            return 1;

        char key;
        std::cin >> key;
        if (key == 'q' || key == 'Q') {
            isRunning = false;
            break;
        }
    }

    if (ipc_injection_thread.joinable()) {
        ipc_injection_thread.join();
    }

    return 0;
}
