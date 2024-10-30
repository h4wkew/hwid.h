#ifndef HWID_H
#define HWID_H

#ifdef __linux__
#include <sys/utsname.h>
#endif

#include <expected>
#include <format>
#include <fstream>
#include <functional>
#include <iomanip>
#include <string>

struct HwidError {
    std::string message;
};

class HwidBase {
public:
    virtual ~HwidBase() = default;
    virtual std::expected<std::string, HwidError> generate() = 0;

protected:
    HwidBase() = default;

    std::string format() const {
        return std::format("{},{},{}-{}", m_username, m_motherboard_serial, m_os, m_architecture);
    }

protected:
    std::string m_username;
    std::string m_os;
    std::string m_architecture;
    std::string m_motherboard_serial;
};

// Platform-specific implementations

#ifdef __linux__
class LinuxHwid : public HwidBase {
public:
    std::expected<std::string, HwidError> generate() {
        set_system_info().or_else([](const HwidError &error) { return std::unexpected(error); });

        set_motherboard_serial().or_else(
            [](const HwidError &error) { return std::unexpected(error); });

        size_t hash = std::hash<std::string>()(format());
        return std::to_string(hash);
    }

private:
    std::expected<void, HwidError> set_system_info() {
        struct utsname utsame;
        if (-1 == uname(&utsame)) {
            return std::unexpected("Failed to retrieve system information.");
        }

        m_username = utsame.nodename;
        m_os = utsame.sysname;
        m_architecture = utsame.machine;
        return {};
    }
    std::expected<void, HwidError> set_motherboard_serial() {
        std::ifstream file("/sys/devices/virtual/dmi/id/board_serial");
        if (!file) {
            return std::unexpected(
                std::format("Failed to open the motherboard serial file ({}).", strerror(errno)));
        }

        std::getline(file, motherboard_serial);
        return {};
    }
};
#elif _WIN32
class WindowsHwid : public HwidBase {
public:
    std::expected<std::string, HwidError> generate() {
        m_username = "hawkew";
        m_os = "Windows";
        m_architecture = "x64";
        m_motherboard_serial = "1234567890";

        size_t hash = std::hash<std::string>()(format());
        return std::to_string(hash);
    }
};
#endif

// Public interface

namespace hwid {

std::expected<std::string, HwidError> generate() {
#ifdef __linux__
    LinuxHwid hwid;
#elif _WIN32
    WindowsHwid hwid;
#else
    return std::unexpected("The current platform is unsupported, only Linux is supported for now.");
#endif
    return hwid.generate();
}

}  // namespace hwid

#endif  // HWID_H
