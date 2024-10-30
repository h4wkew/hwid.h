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

using HWIDError = std::string;

class HWID {
protected:
    HWID() = default;
    virtual ~HWID() = default;

    void set_username(const std::string &username) {
        m_username = username;
    }
    void set_os(const std::string &os) {
        m_os = os;
    }
    void set_architecture(const std::string &architecture) {
        m_architecture = architecture;
    }
    void set_motherboard_serial(const std::string &motherboard_serial) {
        m_motherboard_serial = motherboard_serial;
    }

    std::string to_string() const {
        return std::format("{},{},{}-{}", m_username, m_motherboard_serial, m_os, m_architecture);
    }

protected:
    std::string m_username;
    std::string m_os;
    std::string m_architecture;
    std::string m_motherboard_serial;

    friend struct std::hash<HWID>;
};

namespace std {
template <>
struct hash<HWID> {
    std::size_t operator()(const HWID &hwid) const {
        return std::hash<std::string>()(hwid.to_string());
    }
};
}  // namespace std

// Platform-specific implementations

class LinuxHWID : public HWID {
public:
    static std::expected<std::string, HWIDError> generate() {
        LinuxHWID hwid;

        if (const auto result = hwid.get_system_info(); !result) {
            return std::unexpected(result.error());
        }

        if (const auto result = hwid.get_motherboard_serial(); !result) {
            return std::unexpected(result.error());
        }

        std::hash<HWID> hasher;
        return std::to_string(hasher(hwid));
    }

private:
    LinuxHWID() : HWID() {}

    std::expected<void, HWIDError> get_system_info() {
        struct utsname utsame;
        if (-1 == uname(&utsame)) {
            return std::unexpected("Unable to access system architecture details.");
        }

        set_username(utsame.nodename);
        set_os(utsame.sysname);
        set_architecture(utsame.machine);

        return {};
    }

    std::expected<void, HWIDError> get_motherboard_serial() {
        std::ifstream file("/sys/devices/virtual/dmi/id/board_serial");
        if (!file.is_open()) {
            return std::unexpected(
                std::format("Failed to open the motherboard serial file ({}).", strerror(errno)));
        }

        std::string motherboard_serial;
        std::getline(file, motherboard_serial);
        set_motherboard_serial(motherboard_serial);

        return {};
    }
};

// Public interface

namespace hwid {

std::expected<std::string, HWIDError> generate() {
#ifdef __linux__
    return LinuxHWID::generate();
#else
    return std::unexpected("The current platform is unsupported, only Linux is supported for now.");
#endif
}

}  // namespace hwid

#endif  // HWID_H