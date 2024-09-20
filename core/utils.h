#ifndef TOOCAL_CORE_UTILS_H
#define TOOCAL_CORE_UTILS_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace toocal::core::utils
{
  [[nodiscard]] auto safe_memcmp(std::string k1, std::string k2) noexcept -> int;
  [[nodiscard]] auto safe_bytescmp(
    std::vector<uint8_t> k1, std::vector<uint8_t> k2) noexcept -> int;
}; // namespace toocal::core::utils

#endif /* TOOCAL_CORE_UTILS_H */