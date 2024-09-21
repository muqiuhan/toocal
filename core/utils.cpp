#include "utils.h"

namespace toocal::core::utils
{
  [[nodiscard]] auto Safecmp::memcmp(
    const std::string & k1, const std::string & k2) noexcept -> int
  {
    return std::memcmp(
      k1.data(), k2.data(), (k1.size() > k2.size() ? k2.size() : k1.size()));
  }

  [[nodiscard]] auto Safecmp::bytescmp(
    const std::vector<uint8_t> & k1,
    const std::vector<uint8_t> & k2) noexcept -> int
  {
    return std::memcmp(
      k1.data(), k2.data(), (k1.size() > k2.size() ? k2.size() : k1.size()));
  }
} // namespace toocal::core::utils