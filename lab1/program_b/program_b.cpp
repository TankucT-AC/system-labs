// SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <format>
#include <termios.h>

void ipc_worker(char key) {
    std::string pipe_path = "/tmp/program_a_fifo";
    mkfifo(pipe_path.data(), 0666);

    int fd = open(pipe_path.data(), O_WRONLY);
    if (fd != -1) {
        std::string output_msg = std::format("key={}\n", key);
        write(fd, output_msg.data(), output_msg.size());
        close(fd);
    }
}

int main(int argc, char *argv[]) {
    struct termios originalTermios;
    struct termios rawTermios;

    tcgetattr(STDIN_FILENO, &originalTermios);
    rawTermios = originalTermios;
    rawTermios.c_lflag &= ~(ICANON | ECHO);
    rawTermios.c_cc[VMIN] = 1;
    rawTermios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &rawTermios);
    
    while (true) {
        char key;
        std::cin >> key;

        if (key == 'q' || key == 'Q') break;

        ipc_worker(key);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios);
}