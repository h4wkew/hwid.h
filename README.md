# hwid.h

A lightweight, single-header C++23 library for generating unique, platform-specific hardware IDs.

## Example

```cpp
#include <print>

#include "hwid.h"

int main() {
    if (const auto hwid = hwid::generate(); hwid) {
        std::print("hwid : {}\n", hwid.value());
    } else {
        std::print("(!) hwid failed : {}\n", hwid.error());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
```