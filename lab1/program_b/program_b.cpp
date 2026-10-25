// SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
// SPDX-License-Identifier: GPL-3.0-or-later

// Local libraries
#include <Logger/Logger.h>
#include <Terminal/Terminal.h>
#include <utils/utils.h>

// STL
#include <format>
#include <iostream>
#include <filesystem>

// POSIX C
#include <csignal>
#include <fcntl.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // Блокируем SIGPIPE, чтобы он не убивал программу при попытки записи из пустого канала
    struct sigaction sa{};
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, nullptr);
    
    Logger logger; // по умолчанию ничего не логирует.
    Terminal terminal(argc, argv, logger); // мне лень писать пустой конструктор

    auto pathProgramA = utils::exe_dir() / "program_a";
    if (!std::filesystem::exists(pathProgramA)) {
        std::cerr << "ERROR: program_a is not found!" << std::endl;
        return 1;
    }

    pid_t pid = -1;
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

    // Создаем FIFO канал
    std::string pipe_path = "/tmp/program_a_fifo";
    mkfifo(pipe_path.data(), 0666);
    int fd = open(pipe_path.data(), O_WRONLY);
    if (fd < 0) {
        std::cerr << "ERROR: Failed to open FIFO channel!" << std::endl;
        return 1;
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

        int err = utils::ipc_worker(fd, key);
        if (err != 0)
            break;
    }
    close(fd); // Закрываем FIFO канал
}
