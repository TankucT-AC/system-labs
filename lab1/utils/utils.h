#ifndef UTILS_H
#define UTILS_H

// STL
#include <filesystem>
#include <optional>
#include <string_view>

namespace utils {
int ipc_worker(int fd, char key);
std::optional<char> keyMessageParser(std::string_view command_msg);
std::filesystem::path exe_dir();
} // namespace utils

#endif // UTILS_H