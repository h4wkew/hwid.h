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

using HwidError = std::string;

class HwidBase {
public:
    virtual ~HwidBase() = default;
    virtual std::expected<std::string, HwidError> generate() = 0;

protected:
    HwidBase() = default;

    std::string to_string() const {
        return std::format("{},{},{}-{}", m_username, m_motherboard_serial, m_os, m_architecture);
    }

    size_t to_hash() const {
        return std::hash<std::string>()(to_string());
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
    std::expected<std::string, HwidError> generate();

private:
    std::expected<void, HwidError> set_system_info();
    std::expected<void, HwidError> set_motherboard_serial();
};

inline std::expected<std::string, HwidError> LinuxHwid::generate() {
    if (auto sys_result = set_system_info(); !sys_result) {
        return std::unexpected(sys_result.error());
    }

    if (auto mb_result = set_motherboard_serial(); !mb_result) {
        return std::unexpected(mb_result.error());
    }
    return std::to_string(to_hash());
}

inline std::expected<void, HwidError> LinuxHwid::set_system_info() {
    struct utsname utsame;
    if (-1 == uname(&utsame)) {
        return std::unexpected("Failed to retrieve system information.");
    }

    m_username = utsame.nodename;
    m_os = utsame.sysname;
    m_architecture = utsame.machine;
    return {};
}

inline std::expected<void, HwidError> LinuxHwid::set_motherboard_serial() {
    std::ifstream file("/sys/devices/virtual/dmi/id/board_serial");
    if (!file) {
        return std::unexpected(
            std::format("Failed to open the motherboard serial file ({}).", strerror(errno)));
    }

    std::getline(file, m_motherboard_serial);
    return {};
}

#elif _WIN32
class WindowsHwid : public HwidBase {
public:
    std::expected<std::string, HwidError> generate();
};

inline std::expected<std::string, HwidError> WindowsHwid::generate() {
    m_username = "hawkew";
    m_os = "Windows";
    m_architecture = "x64";
    m_motherboard_serial = "1234567890";

    return std::to_string(to_hash());
}
#endif

// Public interface

namespace hwid {

std::expected<std::string, HwidError> generate() {
#ifdef __linux__
    LinuxHwid hwid;
    return hwid.generate();
#elif _WIN32
    WindowsHwid hwid;
    return hwid.generate();
#else
    return std::unexpected("The current platform is unsupported, only Linux is supported for now.");
#endif
}

}  // namespace hwid

#endif  // HWID_H