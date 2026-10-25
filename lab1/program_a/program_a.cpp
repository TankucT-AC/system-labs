// SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
// SPDX-License-Identifier: GPL-3.0-or-later

// Local libraries
#include <Logger/Logger.h>
#include <Terminal/Terminal.h>
#include <string_view>
#include <utils/utils.h>

// STL 
#include <atomic>
#include <cerrno>
#include <chrono>
#include <iostream>
#include <optional>
#include <thread>

// POSIX C
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void ipc_reader(const std::string &filename, Terminal &terminal,
    std::atomic<bool> &isRunning) {
    char buf[256];

    int fd = -1;
    do {
        fd = open(filename.data(), O_RDONLY | O_NONBLOCK);
    } while (fd < 0 && errno == EINTR);

    if (fd < 0) {
        return;
    }

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    while (isRunning) {
        int ret = poll(&pfd, 1, 100);
        if (ret < 0)
            break;
        else if (ret == 0)
            continue;
        else {
            if (pfd.revents & POLLHUP) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } else if (pfd.revents & POLLIN) {
                ssize_t bytesRead = read(fd, buf, sizeof(buf) - 1);
                if (bytesRead < 0) {
                    if (errno == EINTR)
                        continue;
                    break;
                }
                if (bytesRead == 0)
                    break;

                std::optional<char> optKey =
                    utils::keyMessageParser(std::string_view(buf, bytesRead));
                if (!optKey.has_value())
                    continue;
                terminal.updateTerminalColor(optKey.value());
            }
        }
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    Logger logger(true, "program_a.log");
    Terminal terminal(argc, argv, logger);

    std::atomic<bool> isRunning{true};
    std::thread ipc_reader_thread(ipc_reader,
        "/tmp/program_a_fifo",
        std::ref(terminal),
        std::ref(isRunning));

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

    if (ipc_reader_thread.joinable()) {
        ipc_reader_thread.join();
    }

    return 0;
}
