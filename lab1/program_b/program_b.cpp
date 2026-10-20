// SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Logger/Logger.h>
#include <Terminal/Terminal.h>
#include <fcntl.h>
#include <filesystem>
#include <format>
#include <iostream>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

namespace utils {
std::filesystem::path exe_dir() {
    std::error_code ec;
    auto path = std::filesystem::read_symlink("/proc/self/exe", ec);
    if (ec)
        return std::filesystem::current_path();
    return path.parent_path();
}

int ipc_worker(char key) {
    std::string pipe_path = "/tmp/program_a_fifo";
    mkfifo(pipe_path.data(), 0666);

    int fd = open(pipe_path.data(), O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "ERROR: Failed to open FIFO channel!" << std::endl;
        return 1;
    }

    std::string output_msg = std::format("key={}\n", key);
    ssize_t bytesWritten = write(fd, output_msg.data(), output_msg.size());
    if (bytesWritten < 0) {
        if (errno == EPIPE) {
            std::cerr << "ERROR: all readers is dead!" << std::endl;
        } else {
            std::cerr << "ERROR: fail of write in FIFO channel" << std::endl;
        }
        return 1;
    }
    close(fd);
    return 0;
}
} // namespace utils


int main(int argc, char *argv[]) {
    Logger logger; // по умолчанию ничего не логирует.
    Terminal terminal(argc, argv, logger); // я ленивая жопа, поэтому делать
                                           // дополнительный конструктор не буду

    auto pathProgramA = utils::exe_dir() / "program_a";
    if (!std::filesystem::exists(pathProgramA)) {
        std::cerr << "ERROR: program_a is not found!" << std::endl;
        return 1;
    }

    int pid = -1;
    char *argvProgramA[] = {//
        const_cast<char *>("alacritty"),
        const_cast<char *>("-e"),
        const_cast<char *>(pathProgramA.c_str()),
        NULL};
    int err = posix_spawn(
        &pid, "/usr/bin/alacritty", nullptr, nullptr, argvProgramA, environ);
    if (err != 0) {
        std::cerr << std::format("ERROR {}: failed to call 'posix_spawn'", err)
                  << std::endl;
        return 1;
    }

    if (pid < 0) {
        std::cerr << std::format("ERROR {}: failed to create program_a", pid);
    }

    terminal.enableRawMode();
    while (true) {
        int status = 0;
        int return_pid = waitpid(pid, &status, WNOHANG);
        if (return_pid == pid) {
            std::cerr << "ERROR: program_a is dead" << std::endl;
            break;
        } else if (return_pid == -1) {
            std::cerr << "ERROR: unknown error" << std::endl;
            break;
        }

        char key;
        std::cin >> key;

        if (key == 'q' || key == 'Q')
            break;

        int err = utils::ipc_worker(key);
        if (err != 0)
            break;
    }
}
