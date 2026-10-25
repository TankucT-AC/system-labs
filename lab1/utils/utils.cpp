// Local libraries
#include <utils/utils.h>

// STL
#include <format>
#include <iostream>
#include <ranges>

// POSIX C
#include <fcntl.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int utils::ipc_worker(int fd, char key) {
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
    return 0;
}

std::optional<char> utils::keyMessageParser(std::string_view command_msg) {
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

std::filesystem::path utils::exe_dir() {
    std::error_code ec;
    auto path = std::filesystem::read_symlink("/proc/self/exe", ec);
    if (ec)
        return std::filesystem::current_path();
    return path.parent_path();
}
