#ifndef TOOCAL_CORE_UTILS_H
#define TOOCAL_CORE_UTILS_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace toocal::core::utils
{
  class Safecmp
  {
  public:
    [[nodiscard]] static auto
      memcmp(const std::string& k1, const std::string& k2) noexcept -> int;
    [[nodiscard]] static auto bytescmp(
      const std::vector<uint8_t>& k1,
      const std::vector<uint8_t>& k2) noexcept -> int;
  };
} // namespace toocal::core::utils

#endif /* TOOCAL_CORE_UTILS_H */
