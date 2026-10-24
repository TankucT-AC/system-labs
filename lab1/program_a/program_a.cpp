// SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Logger/Logger.h>
#include <Terminal/Terminal.h>
#include <atomic>
#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <optional>
#include <poll.h>
#include <ranges>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

std::optional<char> keyMessageParser(char buf[], ssize_t bytesRead) {
    std::string_view command_msg(buf, bytesRead);

    while (!command_msg.empty()
        && (command_msg.back() == '\n' || command_msg.back() == '\r')) {
        command_msg.remove_suffix(1);
    }

    auto command_parse = command_msg | std::views::split('=');
    auto it = command_parse.begin();
    if (it == command_parse.end())
        return std::nullopt;

    auto command_range = *it;
    std::string_view command(command_range.begin(), command_range.end());
    if (command != "key")
        return std::nullopt;

    ++it;
    if (it == command_parse.end())
        return std::nullopt;

    auto key_range = *it;
    std::string_view key(key_range.begin(), key_range.end());

    if (key.size() != 1)
        return std::nullopt;

    return key.front();
}

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
        if (ret > 0 && (pfd.revents & POLLIN)) {
            ssize_t bytesRead = read(fd, buf, sizeof(buf) - 1);

            if (bytesRead < 0) {
                if (errno == EINTR)
                    continue;
                break;
            }

            if (bytesRead == 0)
                break;

            std::optional<char> optKey = keyMessageParser(buf, bytesRead);
            if (!optKey.has_value())
                continue;

            terminal.updateTerminalColor(optKey.value());
        } else if (pfd.revents & POLLHUP) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        } else if (ret == 0)
            continue;
        else
            break;
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
