// SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Terminal.h"
#include <atomic>
#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <ranges>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

std::atomic<bool> isRunning{true};

int main(int argc, char *argv[]) {
    Logger logger(true);
    Terminal terminal(argc, argv, logger);

    std::thread ipc_reader(
        [&](const std::string &filename) {
            char buf[256];

            int fd = open(filename.data(), O_RDONLY | O_NONBLOCK);
            if (fd < 0) {
                if (errno != EINTR)
                    return;
            }
            struct pollfd pfd;
            pfd.fd = fd;
            pfd.events = POLLIN;

            while (isRunning) {
                int ret = poll(&pfd, 1, 100);
                if (ret > 0 && (pfd.revents & POLLIN)) {
                    ssize_t bytesRead = read(fd, buf, sizeof(buf) - 1);

                    if (bytesRead < 0) {
                        if (errno == EINTR)
                            continue;
                        break;
                    }

                    if (bytesRead == 0)
                        break;

                    buf[bytesRead] = '\0';
                    std::string_view command_msg(buf, bytesRead);

                    while (!command_msg.empty() && (command_msg.back() == '\n' || command_msg.back() == '\r')) {
                        command_msg.remove_suffix(1);
                    }

                    auto command_parse = command_msg | std::views::split('=');
                    auto it = command_parse.begin();
                    if (it == command_parse.end())
                        continue;

                    auto command_range = *it;
                    std::string_view command(command_range.begin(), command_range.end());
                    if (command != "key")
                        continue;

                    ++it;
                    if (it == command_parse.end())
                        continue;

                    auto key_range = *it;
                    std::string_view key(key_range.begin(), key_range.end());

                    if (key.size() != 1)
                        continue;

                    terminal.updateTerminalColor(key.front());
                } else if (ret == 0)
                    continue;
                else
                    break;
            }
            close(fd);
        },
        "/tmp/program_a_fifo");

    terminal.enableRawMode();
    while (true) {
        char key;
        std::cin >> key;

        if (key == 'q' || key == 'Q') {
            isRunning = false;
            break;
        }

        terminal.updateTerminalColor(key);
    }

    if (ipc_reader.joinable()) {
        ipc_reader.join();
    }

    return 0;
}
