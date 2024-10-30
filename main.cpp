#include <fmt/core.h>

#include "hwid.h"

int main() {
    if (const auto hwid = hwid::generate(); hwid) {
        fmt::print("hwid : {}\n", hwid.value());
    } else {
        fmt::print("(!) hwid failed : {}\n", hwid.error());
        return 1;
    }
    return 0;
}